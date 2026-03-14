#include "voronoi_algorithm.h"
#include "voronoi_incremental.h"
#include "voronoi_dcel.h"
#include <vector>

namespace geometry {

std::unique_ptr<IVoronoiAlgorithm> VoronoiAlgorithmFactory::Create(
    VoronoiAlgorithmType type) {
  
  switch (type) {
    case VoronoiAlgorithmType::kIncremental:
      return std::make_unique<IncrementalVoronoi>();
    
    case VoronoiAlgorithmType::kIncrementalDCEL:
      return std::make_unique<DCELVoronoi>();
    
    default:
      // Return default algorithm
      return std::make_unique<IncrementalVoronoi>();
  }
}

std::vector<VoronoiAlgorithmType> VoronoiAlgorithmFactory::GetAvailableTypes() {
  return {
    VoronoiAlgorithmType::kIncremental,
    VoronoiAlgorithmType::kIncrementalDCEL,
  };
}

}  // namespace geometry
