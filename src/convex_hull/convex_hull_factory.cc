#include "convex_hull/convex_hull_factory.h"
#include "convex_hull/convex_hull_builder.h"

namespace geometry {

ConvexHull ConvexHullFactory::Create(ConvexHullAlgorithm algorithm, 
                                      const std::vector<Point2D>& points) {
  switch (algorithm) {
    case ConvexHullAlgorithm::JarvisMarch:
      return ConvexHullBuilder::BuildJarvisMarch(points);
    
    case ConvexHullAlgorithm::GrahamScan:
      return ConvexHullBuilder::BuildGrahamScan(points);
    
    case ConvexHullAlgorithm::MonotoneChain:
      return ConvexHullBuilder::BuildMonotoneChain(points);
    
    case ConvexHullAlgorithm::QuickHull:
      // TODO: Implement QuickHull
      throw std::runtime_error("QuickHull not implemented yet");
    
    default:
      throw std::runtime_error("Unknown convex hull algorithm");
  }
}

bool ConvexHullFactory::IsAlgorithmSupported(ConvexHullAlgorithm algorithm) {
  switch (algorithm) {
    case ConvexHullAlgorithm::JarvisMarch:
    case ConvexHullAlgorithm::GrahamScan:
    case ConvexHullAlgorithm::MonotoneChain:
      return true;
    case ConvexHullAlgorithm::QuickHull:
      return false;  // Not implemented yet
    default:
      return false;
  }
}

std::vector<ConvexHullAlgorithm> ConvexHullFactory::GetSupportedAlgorithms() {
  return {
    ConvexHullAlgorithm::JarvisMarch,
    ConvexHullAlgorithm::GrahamScan,
    ConvexHullAlgorithm::MonotoneChain
    // Add more algorithms as they are implemented
  };
}

}  // namespace geometry
