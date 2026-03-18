
#pragma once
#include "../core/point2d.h"
#include "../core/vector2d.h"
#include "../core/edge2d.h"
#include <vector>

namespace geometry {

/**
 * @brief Triangle representation with three vertices
 */
struct Triangle {
  Point2D v0, v1, v2;
  
  Triangle() = default;
  Triangle(const Point2D& a, const Point2D& b, const Point2D& c)
      : v0(a), v1(b), v2(c) {}
  
  /**
   * @brief Calculate triangle area using cross product
   * @return Area of the triangle
   */
  double Area() const {
    Vector2D v1_to_v2 = v1 - v0;
    Vector2D v1_to_v3 = v2 - v0;
    return std::abs(v1_to_v2.Cross(v1_to_v3)) / 2.0;
  }
  
  /**
   * @brief Check if point is inside triangle (including boundary)
   * @param p Point to check
   * @return True if point is inside or on boundary
   */
  bool Contains(const Point2D& p) const;
  
  /**
   * @brief Get circumcenter of the triangle
   * @return Circumcenter point
   */
  Point2D Circumcenter() const;
  
  /**
   * @brief Get circumradius of the triangle
   * @return Circumradius
   */
  double Circumradius() const;
  
  /**
   * @brief Check if triangle is degenerate (area near zero)
   * @return True if triangle is degenerate
   */
  bool IsDegenerate(double epsilon = 1e-9) const {
    return Area() < epsilon;
  }
};

/**
 * @brief Triangulation result containing triangles, vertices, and edges
 */
struct TriangulationResult {
  std::vector<Triangle> triangles;
  std::vector<Point2D> vertices;
  std::vector<Edge2D> edges;
  
  /**
   * @brief Check if result is valid (has triangles)
   * @return True if valid
   */
  bool IsValid() const {
    return !triangles.empty();
  }
  
  /**
   * @brief Get number of triangles
   * @return Triangle count
   */
  size_t TriangleCount() const {
    return triangles.size();
  }
  
  /**
   * @brief Calculate total area of all triangles
   * @return Total area
   */
  double TotalArea() const {
    double total = 0.0;
    for (const auto& tri : triangles) {
      total += tri.Area();
    }
    return total;
  }
  
  /**
   * @brief Clear all data
   */
  void Clear() {
    triangles.clear();
    vertices.clear();
    edges.clear();
  }
};

/**
 * @brief Triangulation algorithm types
 */
enum class TriangulationAlgorithmType {
  kSweepLine,        // Sweep line algorithm (top to bottom)
  kEarClipping,      // Ear clipping method
  kDelaunay,         // Delaunay triangulation
};

}  // namespace geometry

