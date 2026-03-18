
#pragma once
#include "../core/point2d.h"
#include "../core/edge2d.h"
#include <vector>

namespace geometry {

/**
 * @brief Voronoi cell representation
 * 
 * Represents a single Voronoi cell, which is the region of points
 * closest to a particular site point.
 */
struct VoronoiCell {
  size_t site_index;              // Index of the site point
  Point2D site;                   // The site point (generator)
  std::vector<Point2D> vertices;  // Vertices of the cell (polygon)
  std::vector<Edge2D> edges;      // Edges of the cell boundary
  
  /**
   * @brief Check if the cell is valid
   * @return true if the cell has vertices and edges
   */
  bool IsValid() const {
    return !vertices.empty() && !edges.empty();
  }
  
  /**
   * @brief Get the number of vertices in the cell
   * @return Number of vertices
   */
  size_t VertexCount() const {
    return vertices.size();
  }
  
  /**
   * @brief Get the number of edges in the cell
   * @return Number of edges
   */
  size_t EdgeCount() const {
    return edges.size();
  }
};

/**
 * @brief Voronoi diagram result
 * 
 * Contains the complete Voronoi diagram for a set of sites,
 * including all cells, edges, and vertices.
 */
struct VoronoiDiagramResult {
  std::vector<Point2D> sites;      // Input sites
  std::vector<VoronoiCell> cells;  // Voronoi cells (one per site)
  std::vector<Edge2D> edges;       // All Voronoi edges
  std::vector<Point2D> vertices;   // All Voronoi vertices
  
  /**
   * @brief Check if the diagram is valid
   * @return true if the diagram has at least one cell
   */
  bool IsValid() const {
    return !cells.empty();
  }
  
  /**
   * @brief Get the number of cells
   * @return Number of Voronoi cells
   */
  size_t CellCount() const {
    return cells.size();
  }
  
  /**
   * @brief Get the number of edges
   * @return Number of edges
   */
  size_t EdgeCount() const {
    return edges.size();
  }
  
  /**
   * @brief Get the number of vertices
   * @return Number of vertices
   */
  size_t VertexCount() const {
    return vertices.size();
  }
  
  /**
   * @brief Get the number of sites
   * @return Number of input sites
   */
  size_t SiteCount() const {
    return sites.size();
  }
};

/**
 * @brief Bounding box for Voronoi diagram computation
 */
struct VoronoiBounds {
  double min_x;
  double min_y;
  double max_x;
  double max_y;
  
  VoronoiBounds(double min_x = -100.0, double min_y = -100.0,
                double max_x = 100.0, double max_y = 100.0)
      : min_x(min_x), min_y(min_y), max_x(max_x), max_y(max_y) {}
  
  /**
   * @brief Get the width of the bounding box
   * @return Width
   */
  double Width() const {
    return max_x - min_x;
  }
  
  /**
   * @brief Get the height of the bounding box
   * @return Height
   */
  double Height() const {
    return max_y - min_y;
  }
  
  /**
   * @brief Check if a point is inside the bounds
   * @param point Point to check
   * @return true if the point is inside
   */
  bool Contains(const Point2D& point) const {
    return point.x >= min_x && point.x <= max_x &&
           point.y >= min_y && point.y <= max_y;
  }
};

}  // namespace geometry

