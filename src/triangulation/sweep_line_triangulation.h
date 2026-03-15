#ifndef GEOMETRY_TRIANGULATION_SWEEP_LINE_H_
#define GEOMETRY_TRIANGULATION_SWEEP_LINE_H_

#include "triangulation_algorithm.h"
#include "triangulation_types.h"
#include <vector>

namespace geometry {

/**
 * @brief Sweep line triangulation algorithm
 * 
 * This algorithm triangulates a monotone polygon by sweeping
 * from top to bottom and adding edges as needed.
 * 
 * Time Complexity: O(n log n) for monotone polygon
 * Space Complexity: O(n)
 * 
 * Advantages:
 * - Optimal O(n log n) time complexity
 * - Good for large datasets
 * 
 * Disadvantages:
 * - More complex implementation
 * - Requires monotone polygon decomposition first
 */
class SweepLineTriangulation : public ITriangulationAlgorithm {
 public:
  /**
   * @brief Triangulate polygon using sweep line
   * @param polygon Monotone polygon vertices in order
   * @return Triangulation result
   */
  TriangulationResult Triangulate(
      const std::vector<Point2D>& polygon) override;
  
  /**
   * @brief Get algorithm name
   * @return Algorithm name
   */
  std::string Name() const override {
    return "Sweep Line";
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
    return "Sweep line algorithm for monotone polygons. "
           "Sweeps from top to bottom, adding edges to maintain triangulation. "
           "Optimal for large monotone polygons.";
  }
  
 private:
  /**
   * @brief Triangulate a monotone polygon
   * @param polygon Monotone polygon vertices (sorted by y-coordinate)
   * @return Triangulation result
   */
  TriangulationResult TriangulateMonotonePolygon(
      const std::vector<Point2D>& polygon);
};

}  // namespace geometry

#endif  // GEOMETRY_TRIANGULATION_SWEEP_LINE_H_
