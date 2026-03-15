/**
 * @file delaunay_triangulation_optimized.h
 * @brief Optimized Delaunay triangulation using spatial grid (buckets)
 * 
 * This is an optimized version of the Delaunay triangulation that uses
 * a spatial grid to accelerate the search for bad triangles.
 * 
 * Performance Improvements:
 * - Naive: O(n²) total time (O(n) per point to find bad triangles)
 * - Optimized: O(n log n) total time (O(1) average per point)
 * 
 * When to use:
 * - Large point sets (> 1000 points)
 * - Need for fast performance
 * - Memory is not a constraint
 * 
 * Trade-offs:
 * - + Faster query time
 * - + Better scalability
 * - - Additional memory overhead
 * - - More complex implementation
 */

#ifndef GEOMETRY_TRIANGULATION_DELAUNAY_OPTIMIZED_H_
#define GEOMETRY_TRIANGULATION_DELAUNAY_OPTIMIZED_H_

#include "delaunay_triangulation.h"
#include "spatial_grid.h"
#include <memory>
#include <sstream>

namespace geometry {

/**
 * @brief Optimized Delaunay triangulation with spatial indexing
 * 
 * This class extends the basic Delaunay triangulation by adding
 * a spatial grid to accelerate the search for bad triangles.
 * 
 * Key Optimization:
 * - Instead of checking all triangles for each point
 * - Only check triangles in nearby grid cells
 * - Reduces per-point query from O(n) to O(1) average
 * 
 * Algorithm:
 * 1. Create spatial grid with appropriate cell size
 * 2. For each point insertion:
 *    a. Query grid for nearby triangles (O(1))
 *    b. Test only those triangles for circumcircle condition
 *    c. Update grid with new triangles
 * 
 * Performance:
 * - Grid construction: O(n)
 * - Per-point query: O(k) where k is small (typically < 20)
 * - Total time: O(n log n) average
 * - Memory: O(n) additional for grid
 */
class DelaunayTriangulationOptimized : public DelaunayTriangulation {
 public:
  /**
   * @brief Constructor with automatic cell size detection
   * @param auto_cell_size If true, automatically determine cell size from points
   */
  explicit DelaunayTriangulationOptimized(bool auto_cell_size = true)
      : auto_cell_size_(auto_cell_size),
        spatial_grid_(nullptr) {}
  
  /**
   * @brief Compute Delaunay triangulation with spatial optimization
   * @param points Input point set
   * @return Triangulation result
   * 
   * This method overrides the base implementation to use spatial grid
   * for faster bad triangle detection.
   */
  TriangulationResult Triangulate(
      const std::vector<Point2D>& points) override;
  
  /**
   * @brief Get algorithm name
   * @return Algorithm name with optimization note
   */
  std::string Name() const override {
    return "Delaunay (Bowyer-Watson + Spatial Grid)";
  }
  
  /**
   * @brief Get time complexity
   * @return Complexity string
   */
  std::string Complexity() const override {
    return "O(n log n) avg with spatial indexing";
  }
  
  /**
   * @brief Get algorithm description
   * @return Description with optimization details
   */
  std::string Description() const override {
    return DelaunayTriangulation::Description() + 
           " Optimized with spatial grid for O(1) average bad triangle search.";
  }
  
  /**
   * @brief Get statistics about the spatial grid
   * @return String with grid statistics
   */
  std::string GetGridStatistics() const {
    if (!spatial_grid_) {
      return "No spatial grid created";
    }
    
    std::ostringstream oss;
    oss << "Spatial Grid Statistics:\n";
    oss << "  Cell count: " << spatial_grid_->GetCellCount() << "\n";
    oss << "  Total triangle refs: " << spatial_grid_->GetTotalTriangleCount() << "\n";
    oss << "  Avg triangles per cell: " << spatial_grid_->GetAverageTrianglesPerCell();
    return oss.str();
  }

 private:
  /**
   * @brief Find bad triangles using spatial grid
   * @param point Point to test
   * @param dcel DCEL structure
   * @return Set of bad triangles
   * 
   * This is the optimized version that uses the spatial grid
   * to quickly find candidate triangles.
   */
  std::set<Face*> FindBadTrianglesOptimized(
      const Point2D& point,
      DCEL* dcel);
  
  /**
   * @brief Update spatial grid with new triangles
   * @param dcel DCEL structure
   * @param start_index Start adding from this face index
   * 
   * After re-triangulation, new triangles are added to the grid
   * to maintain spatial indexing for subsequent queries.
   */
  void UpdateSpatialGrid(DCEL* dcel, size_t start_index);
  
  bool auto_cell_size_;                    // Auto-detect cell size
  std::unique_ptr<SpatialGrid> spatial_grid_;  // Spatial indexing structure
};

}  // namespace geometry

#endif  // GEOMETRY_TRIANGULATION_DELAUNAY_OPTIMIZED_H_
