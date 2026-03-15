#ifndef GEOMETRY_TRIANGULATION_ALGORITHM_H_
#define GEOMETRY_TRIANGULATION_ALGORITHM_H_

#include "triangulation_types.h"
#include <vector>
#include <memory>

namespace geometry {

/**
 * @brief Interface for triangulation algorithms
 * 
 * This interface defines the contract that all triangulation algorithms
 * must implement. It follows the Strategy pattern, allowing different
 * triangulation algorithms to be used interchangeably.
 */
class ITriangulationAlgorithm {
 public:
  virtual ~ITriangulationAlgorithm() = default;
  
  /**
   * @brief Triangulate a polygon or point set
   * @param points Input polygon vertices (in order) or point set
   * @return Triangulation result
   */
  virtual TriangulationResult Triangulate(
      const std::vector<Point2D>& points) = 0;
  
  /**
   * @brief Get algorithm name
   * @return Human-readable algorithm name
   */
  virtual std::string Name() const = 0;
  
  /**
   * @brief Get algorithm time complexity
   * @return Complexity string (e.g., "O(n log n)")
   */
  virtual std::string Complexity() const = 0;
  
  /**
   * @brief Get algorithm description
   * @return Detailed description of the algorithm
   */
  virtual std::string Description() const = 0;
};

}  // namespace geometry

#endif  // GEOMETRY_TRIANGULATION_ALGORITHM_H_
