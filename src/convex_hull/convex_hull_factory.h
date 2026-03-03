#ifndef GEOMETRY_CORE_CONVEX_HULL_FACTORY_H_
#define GEOMETRY_CORE_CONVEX_HULL_FACTORY_H_

#include "convex_hull/convex_hull.h"
#include "../core/point2d.h"
#include <vector>
#include <memory>
#include <string>

namespace geometry {

// Supported convex hull algorithms
enum class ConvexHullAlgorithm {
  JarvisMarch,      // O(nh) - Gift Wrapping
  GrahamScan,       // O(n log n) - Sort by angle
  QuickHull,        // O(n log n) average - Divide and Conquer
  MonotoneChain     // O(n log n) - Sort by x then y
};

// Get algorithm name for display
inline std::string GetAlgorithmName(ConvexHullAlgorithm algorithm) {
  switch (algorithm) {
    case ConvexHullAlgorithm::JarvisMarch:
      return "Jarvis March (Gift Wrapping)";
    case ConvexHullAlgorithm::GrahamScan:
      return "Graham Scan";
    case ConvexHullAlgorithm::QuickHull:
      return "QuickHull";
    case ConvexHullAlgorithm::MonotoneChain:
      return "Monotone Chain";
    default:
      return "Unknown";
  }
}

// Get algorithm time complexity
inline std::string GetAlgorithmComplexity(ConvexHullAlgorithm algorithm) {
  switch (algorithm) {
    case ConvexHullAlgorithm::JarvisMarch:
      return "O(nh)";
    case ConvexHullAlgorithm::GrahamScan:
      return "O(n log n)";
    case ConvexHullAlgorithm::QuickHull:
      return "O(n log n) average";
    case ConvexHullAlgorithm::MonotoneChain:
      return "O(n log n)";
    default:
      return "Unknown";
  }
}

// Factory class for creating convex hulls using different algorithms
class ConvexHullFactory {
 public:
  // Create convex hull using specified algorithm
  static ConvexHull Create(ConvexHullAlgorithm algorithm, 
                           const std::vector<Point2D>& points);
  
  // Check if algorithm is supported
  static bool IsAlgorithmSupported(ConvexHullAlgorithm algorithm);
  
  // Get list of all supported algorithms
  static std::vector<ConvexHullAlgorithm> GetSupportedAlgorithms();
};

}  // namespace geometry

#endif  // GEOMETRY_CORE_CONVEX_HULL_FACTORY_H_
