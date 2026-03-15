/**
 * @file spatial_grid.h
 * @brief Spatial grid (bucket) data structure for accelerating Delaunay triangulation
 * 
 * This file implements a spatial grid (also known as bucket grid or spatial hash)
 * to accelerate point and triangle queries in Delaunay triangulation.
 * 
 * Motivation:
 * - In the naive implementation, finding bad triangles requires O(n) time per point
 * - With spatial grid, we only check triangles in nearby cells: O(1) average
 * - This reduces total complexity from O(n²) to O(n log n) or better
 * 
 * How it works:
 * - Divide the space into a grid of cells (buckets)
 * - Each cell stores triangles that overlap with it
 * - When querying a point, only check triangles in adjacent cells
 * 
 * Trade-offs:
 * - Memory: O(n) additional space for the grid
 * - Build time: O(n) to populate the grid
 * - Query time: O(1) average (constant number of cells to check)
 */

#ifndef GEOMETRY_TRIANGULATION_SPATIAL_GRID_H_
#define GEOMETRY_TRIANGULATION_SPATIAL_GRID_H_

#include "../core/point2d.h"
#include "../dcel/dcel.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace geometry {

/**
 * @brief Spatial grid for accelerating triangle queries
 * 
 * This class divides the 2D space into a uniform grid and stores
 * triangles in the cells they overlap with. This allows for efficient
 * spatial queries, especially for finding triangles near a given point.
 * 
 * Grid Parameters:
 * - cell_size: Size of each grid cell (should be ~average triangle size)
 * - bounds: Axis-aligned bounding box of the space
 * 
 * Usage:
 * 1. Create grid with appropriate cell size
 * 2. Insert triangles into the grid
 * 3. Query points to get nearby triangles
 * 
 * Performance:
 * - Insert: O(1) amortized
 * - Query: O(k) where k is the number of triangles in nearby cells (typically small)
 * - Memory: O(n) where n is the number of triangles
 */
class SpatialGrid {
 public:
  /**
   * @brief Construct a spatial grid with given cell size
   * @param cell_size Size of each grid cell (should be ~average triangle edge length)
   * @param min_x Minimum x coordinate of the grid (optional, auto-computed if not specified)
   * @param min_y Minimum y coordinate of the grid (optional, auto-computed if not specified)
   * @param max_x Maximum x coordinate of the grid (optional, auto-computed if not specified)
   * @param max_y Maximum y coordinate of the grid (optional, auto-computed if not specified)
   */
  explicit SpatialGrid(
      double cell_size = 1.0,
      double min_x = -1e10, double min_y = -1e10,
      double max_x = 1e10, double max_y = 1e10)
      : cell_size_(cell_size),
        min_x_(min_x), min_y_(min_y),
        max_x_(max_x), max_y_(max_y) {
    
    // Validate cell size
    if (cell_size_ <= 0.0) {
      cell_size_ = 1.0;  // Default fallback
    }
  }
  
  /**
   * @brief Insert a triangle into the spatial grid
   * 
   * The triangle is added to all cells it overlaps with.
   * This ensures that queries will find the triangle regardless of
   * which cell the query point falls in.
   * 
   * @param face Triangle face to insert
   * @param dcel DCEL structure (used to get triangle vertices)
   * 
   * @note A triangle may be inserted into multiple cells
   * @note Duplicate insertions are handled internally
   */
  void InsertTriangle(Face* face, const DCEL* dcel) {
    if (face == nullptr || face->IsUnbounded()) {
      return;
    }
    
    // Get triangle vertices
    HalfEdge* e0 = face->GetOuterComponent();
    HalfEdge* e1 = e0->GetNext();
    HalfEdge* e2 = e1->GetNext();
    
    Point2D v0 = e0->GetOrigin()->GetCoordinates();
    Point2D v1 = e1->GetOrigin()->GetCoordinates();
    Point2D v2 = e2->GetOrigin()->GetCoordinates();
    
    // Compute bounding box of triangle
    double tri_min_x = std::min({v0.x, v1.x, v2.x});
    double tri_max_x = std::max({v0.x, v1.x, v2.x});
    double tri_min_y = std::min({v0.y, v1.y, v2.y});
    double tri_max_y = std::max({v0.y, v1.y, v2.y});
    
    // Get range of cells this triangle overlaps
    int min_cell_x = static_cast<int>(std::floor((tri_min_x - min_x_) / cell_size_));
    int max_cell_x = static_cast<int>(std::floor((tri_max_x - min_x_) / cell_size_));
    int min_cell_y = static_cast<int>(std::floor((tri_min_y - min_y_) / cell_size_));
    int max_cell_y = static_cast<int>(std::floor((tri_max_y - min_y_) / cell_size_));
    
    // Insert triangle into all overlapping cells
    for (int cx = min_cell_x; cx <= max_cell_x; ++cx) {
      for (int cy = min_cell_y; cy <= max_cell_y; ++cy) {
        int64_t cell_key = GetCellKey(cx, cy);
        grid_[cell_key].insert(face);
      }
    }
  }
  
  /**
   * @brief Find all triangles that might contain the given point in their circumcircle
   * 
   * Instead of checking all triangles, we only check triangles in the
   * cell containing the point and adjacent cells. This dramatically
   * reduces the number of triangles to test.
   * 
   * @param point Query point
   * @return Set of triangle faces that are near the point
   * 
   * @note This returns candidate triangles, not necessarily bad triangles
   * @note The caller should still test the circumcircle condition
   * @note Checks 3x3 = 9 cells to handle boundary cases
   */
  std::unordered_set<Face*> QueryNearbyTriangles(const Point2D& point) const {
    std::unordered_set<Face*> nearby_triangles;
    
    // Get cell containing the point
    int cell_x = static_cast<int>(std::floor((point.x - min_x_) / cell_size_));
    int cell_y = static_cast<int>(std::floor((point.y - min_y_) / cell_size_));
    
    // Check 3x3 neighborhood to handle boundary cases
    // This ensures we don't miss triangles that overlap the cell boundary
    for (int dx = -1; dx <= 1; ++dx) {
      for (int dy = -1; dy <= 1; ++dy) {
        int64_t cell_key = GetCellKey(cell_x + dx, cell_y + dy);
        
        auto it = grid_.find(cell_key);
        if (it != grid_.end()) {
          // Insert all triangles in this cell
          for (Face* face : it->second) {
            nearby_triangles.insert(face);
          }
        }
      }
    }
    
    return nearby_triangles;
  }
  
  /**
   * @brief Clear all triangles from the grid
   * 
   * This is useful when rebuilding the grid after significant
   * changes to the triangulation.
   */
  void Clear() {
    grid_.clear();
  }
  
  /**
   * @brief Get the number of cells in the grid
   * @return Number of non-empty cells
   */
  size_t GetCellCount() const {
    return grid_.size();
  }
  
  /**
   * @brief Get the total number of triangle references stored
   * @return Total number of triangle-cell associations
   * 
   * @note This may be larger than the number of unique triangles
   *       because a triangle can be in multiple cells
   */
  size_t GetTotalTriangleCount() const {
    size_t count = 0;
    for (const auto& entry : grid_) {
      count += entry.second.size();
    }
    return count;
  }
  
  /**
   * @brief Estimate the average number of triangles per cell
   * @return Average triangles per non-empty cell
   */
  double GetAverageTrianglesPerCell() const {
    size_t cell_count = GetCellCount();
    if (cell_count == 0) return 0.0;
    return static_cast<double>(GetTotalTriangleCount()) / cell_count;
  }
  
  /**
   * @brief Update grid bounds based on a set of points
   * 
   * This automatically computes the bounding box and sets appropriate
   * grid boundaries. Call this before inserting triangles if you didn't
   * specify bounds in the constructor.
   * 
   * @param points Input points to compute bounds from
   * @param expand_factor Factor to expand bounds (default: 2.0)
   * 
   * @note Expanding bounds ensures we don't miss triangles near the edge
   */
  void UpdateBoundsFromPoints(
      const std::vector<Point2D>& points,
      double expand_factor = 2.0) {
    
    if (points.empty()) return;
    
    // Compute bounding box
    double min_x = points[0].x, max_x = points[0].x;
    double min_y = points[0].y, max_y = points[0].y;
    
    for (const auto& p : points) {
      min_x = std::min(min_x, p.x);
      max_x = std::max(max_x, p.x);
      min_y = std::min(min_y, p.y);
      max_y = std::max(max_y, p.y);
    }
    
    // Expand bounds
    double dx = max_x - min_x;
    double dy = max_y - min_y;
    
    min_x_ = min_x - dx * (expand_factor - 1.0) * 0.5;
    max_x_ = max_x + dx * (expand_factor - 1.0) * 0.5;
    min_y_ = min_y - dy * (expand_factor - 1.0) * 0.5;
    max_y_ = max_y + dy * (expand_factor - 1.0) * 0.5;
  }
  
  /**
   * @brief Recommend a cell size based on point distribution
   * 
   * A good cell size is approximately the average edge length of
   * the expected triangles. This method estimates it from point density.
   * 
   * @param points Input points
   * @param target_triangles_per_cell Target number of triangles per cell (default: 10)
   * @return Recommended cell size
   * 
   * @note This is a heuristic; optimal size depends on the specific dataset
   */
  static double RecommendCellSize(
      const std::vector<Point2D>& points,
      double target_triangles_per_cell = 10.0) {
    
    if (points.size() < 3) return 1.0;
    
    // Compute bounding box
    double min_x = points[0].x, max_x = points[0].x;
    double min_y = points[0].y, max_y = points[0].y;
    
    for (const auto& p : points) {
      min_x = std::min(min_x, p.x);
      max_x = std::max(max_x, p.x);
      min_y = std::min(min_y, p.y);
      max_y = std::max(max_y, p.y);
    }
    
    double area = (max_x - min_x) * (max_y - min_y);
    double estimated_triangles = 2.0 * points.size();  // Euler's formula approximation
    
    double cell_area = area / (estimated_triangles / target_triangles_per_cell);
    return std::sqrt(cell_area);
  }

 private:
  /**
   * @brief Compute a unique key for a cell
   * @param cell_x Cell x index
   * @param cell_y Cell y index
   * @return Unique 64-bit key for the cell
   * 
   * @note Uses bit interleaving to create a unique key
   */
  int64_t GetCellKey(int cell_x, int cell_y) const {
    // Simple hash: combine x and y coordinates
    // This works well for most cases
    return (static_cast<int64_t>(cell_x) << 32) | 
           (static_cast<int64_t>(cell_y) & 0xFFFFFFFF);
  }
  
  double cell_size_;      // Size of each grid cell
  double min_x_, min_y_;  // Grid boundaries
  double max_x_, max_y_;
  
  // Map from cell key to set of triangles in that cell
  std::unordered_map<int64_t, std::unordered_set<Face*>> grid_;
};

}  // namespace geometry

#endif  // GEOMETRY_TRIANGULATION_SPATIAL_GRID_H_
