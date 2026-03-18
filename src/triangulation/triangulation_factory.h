
#pragma once
#include "triangulation_types.h"
#include "triangulation_algorithm.h"
#include <vector>
#include <memory>

namespace geometry {

/**
 * @brief Factory class for creating triangulation algorithms
 * 
 * This factory encapsulates the creation logic for different
 * triangulation algorithms, following the Factory pattern.
 */
class TriangulationFactory {
 public:
  /**
   * @brief Create triangulation algorithm instance
   * @param type Algorithm type
   * @return Unique pointer to algorithm instance
   */
  static std::unique_ptr<ITriangulationAlgorithm> Create(
      TriangulationAlgorithmType type);
  
  /**
   * @brief Get list of supported algorithms
   * @return Vector of supported algorithm types
   */
  static std::vector<TriangulationAlgorithmType> GetSupportedAlgorithms();
  
  /**
   * @brief Get algorithm name
   * @param type Algorithm type
   * @return Human-readable name
   */
  static std::string GetAlgorithmName(TriangulationAlgorithmType type);
  
  /**
   * @brief Get algorithm complexity
   * @param type Algorithm type
   * @return Complexity string
   */
  static std::string GetAlgorithmComplexity(TriangulationAlgorithmType type);
  
  /**
   * @brief Get algorithm description
   * @param type Algorithm type
   * @return Detailed description
   */
  static std::string GetAlgorithmDescription(TriangulationAlgorithmType type);
};

}  // namespace geometry

