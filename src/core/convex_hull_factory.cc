#include "convex_hull_factory.h"
#include "convex_hull_builder.h"

namespace geometry {

ConvexHull ConvexHullFactory::Create(ConvexHullAlgorithm algorithm, 
                                      const std::vector<Point2D>& points) {
  switch (algorithm) {
    case ConvexHullAlgorithm::JarvisMarch:
      return ConvexHullBuilder::BuildJarvisMarch(points);
    
    case ConvexHullAlgorithm::GrahamScan:
      // TODO: Implement GrahamScan
      throw std::runtime_error("GrahamScan not implemented yet");
    
    case ConvexHullAlgorithm::QuickHull:
      // TODO: Implement QuickHull
      throw std::runtime_error("QuickHull not implemented yet");
    
    case ConvexHullAlgorithm::MonotoneChain:
      // TODO: Implement MonotoneChain
      throw std::runtime_error("MonotoneChain not implemented yet");
    
    default:
      throw std::runtime_error("Unknown convex hull algorithm");
  }
}

bool ConvexHullFactory::IsAlgorithmSupported(ConvexHullAlgorithm algorithm) {
  switch (algorithm) {
    case ConvexHullAlgorithm::JarvisMarch:
      return true;
    case ConvexHullAlgorithm::GrahamScan:
    case ConvexHullAlgorithm::QuickHull:
    case ConvexHullAlgorithm::MonotoneChain:
      return false;  // Not implemented yet
    default:
      return false;
  }
}

std::vector<ConvexHullAlgorithm> ConvexHullFactory::GetSupportedAlgorithms() {
  return {
    ConvexHullAlgorithm::JarvisMarch
    // Add more algorithms as they are implemented
  };
}

}  // namespace geometry
