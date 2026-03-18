
#pragma once
#include "triangulation_types.h"
#include "triangulation_factory.h"
#include <vector>
#include <memory>

namespace geometry {

/**
 * @brief Facade class for triangulation operations
 * 
 * This class provides a simple interface for triangulating polygons
 * and point sets. It delegates to specific algorithm implementations
 * through the factory pattern.
 * 
 * This is the main entry point for triangulation operations.
 */
class Triangulation {
 public:
  /**
   * @brief Triangulate a polygon or point set
   * @param points Input polygon vertices (in order) or point set
   * @param algorithm Algorithm type to use
   * @return Triangulation result
   */
  static TriangulationResult Triangulate(
      const std::vector<Point2D>& points,
      TriangulationAlgorithmType algorithm = TriangulationAlgorithmType::kSweepLine);
  
  /**
   * @brief Get list of supported algorithms
   * @return Vector of supported algorithm types
   */
  static std::vector<TriangulationAlgorithmType> GetSupportedAlgorithms();
  
  /**
   * @brief Get algorithm name
   * @param algorithm Algorithm type
   * @return Human-readable name
   */
  static std::string GetAlgorithmName(TriangulationAlgorithmType algorithm);
  
  /**
   * @brief Get algorithm complexity
   * @param algorithm Algorithm type
   * @return Complexity string
   */
  static std::string GetAlgorithmComplexity(TriangulationAlgorithmType algorithm);
};

}  // namespace geometry

