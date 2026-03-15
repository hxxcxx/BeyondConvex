#ifndef GEOMETRY_TRIANGULATION_DELAUNAY_H_
#define GEOMETRY_TRIANGULATION_DELAUNAY_H_

#include "triangulation_algorithm.h"
#include "triangulation_types.h"
#include <vector>

namespace geometry {

/**
 * @brief Delaunay triangulation algorithm
 * 
 * This algorithm computes the Delaunay triangulation of a point set,
 * which maximizes the minimum angle of all triangles and avoids
 * skinny triangles.
 * 
 * Time Complexity: O(n log n) average, O(n²) worst case
 * Space Complexity: O(n)
 * 
 * Advantages:
 * - Produces high-quality triangulation
 * - Maximizes minimum angle
 * - Unique for generic point sets
 * 
 * Disadvantages:
 * - More complex implementation
 * - Requires careful numerical handling
 */
class DelaunayTriangulation : public ITriangulationAlgorithm {
 public:
  /**
   * @brief Compute Delaunay triangulation of point set
   * @param points Input point set (not necessarily in order)
   * @return Triangulation result
   */
  TriangulationResult Triangulate(
      const std::vector<Point2D>& points) override;
  
  /**
   * @brief Get algorithm name
   * @return Algorithm name
   */
  std::string Name() const override {
    return "Delaunay";
  }
  
  /**
   * @brief Get time complexity
   * @return Complexity string
   */
  std::string Complexity() const override {
    return "O(n log n)";
  }
  
  /**
   * @brief Get algorithm description
   * @return Description
   */
  std::string Description() const override {
    return "Delaunay triangulation for point sets. "
           "Maximizes minimum angle and produces high-quality triangles. "
           "Optimal for many applications including mesh generation.";
  }
  
 private:
  /**
   * @brief Check if triangle is Delaunay (empty circumcircle)
   * @param tri Triangle to check
   * @param points All points in triangulation
   * @return True if Delaunay
   */
  bool IsDelaunay(
      const Triangle& tri,
      const std::vector<Point2D>& points);
  
  /**
   * @brief Flip edge to restore Delaunay property
   * @param edge Edge to flip
   * @param triangles Current triangulation
   */
  void FlipEdge(
      Edge2D edge,
      std::vector<Triangle>& triangles);
};

}  // namespace geometry

#endif  // GEOMETRY_TRIANGULATION_DELAUNAY_H_
