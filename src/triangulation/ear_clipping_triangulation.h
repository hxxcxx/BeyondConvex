
#pragma once
#include "triangulation_algorithm.h"
#include "triangulation_types.h"
#include <vector>

namespace geometry {

/**
 * @brief Ear clipping triangulation algorithm
 * 
 * This algorithm triangulates a simple polygon by repeatedly
 * removing "ears" - triangles formed by three consecutive vertices
 * that are entirely contained within the polygon.
 * 
 * Time Complexity: O(n^2) for simple polygon
 * Space Complexity: O(n)
 * 
 * Advantages:
 * - Simple to implement
 * - Works for any simple polygon
 * 
 * Disadvantages:
 * - O(n^2) time complexity
 * - Requires simple polygon (no self-intersections)
 */
class EarClippingTriangulation : public ITriangulationAlgorithm {
 public:
  /**
   * @brief Triangulate polygon using ear clipping
   * @param polygon Simple polygon vertices in order (CCW or CW)
   * @return Triangulation result
   */
  TriangulationResult Triangulate(
      const std::vector<Point2D>& polygon) override;
  
  /**
   * @brief Get algorithm name
   * @return Algorithm name
   */
  std::string Name() const override {
    return "Ear Clipping";
  }
  
  /**
   * @brief Get time complexity
   * @return Complexity string
   */
  std::string Complexity() const override {
    return "O(n²)";
  }
  
  /**
   * @brief Get algorithm description
   * @return Description
   */
  std::string Description() const override {
    return "Ear clipping algorithm for simple polygons. "
           "Repeatedly removes ears (convex vertices) until triangulation is complete. "
           "Simple but not optimal for large polygons.";
  }
  
 private:
  /**
   * @brief Check if vertex forms an ear
   * @param polygon Polygon vertices
   * @param i Vertex index
   * @return True if vertex is an ear
   */
  bool IsEar(const std::vector<Point2D>& polygon, size_t i);
  
  /**
   * @brief Find next ear in polygon
   * @param polygon Polygon vertices
   * @return Index of ear vertex, or -1 if none found
   */
  int FindEar(const std::vector<Point2D>& polygon);
};

}  // namespace geometry

