#include "voronoi_algorithm.h"
#include "voronoi_incremental.h"
#include "voronoi_dcel.h"
#include "voronoi_divide_conquer.h"
#include <vector>

namespace geometry {

std::unique_ptr<IVoronoiAlgorithm> VoronoiAlgorithmFactory::Create(
    VoronoiAlgorithmType type) {
  
  switch (type) {
    case VoronoiAlgorithmType::kIncremental:
      return std::make_unique<IncrementalVoronoi>();
    
    case VoronoiAlgorithmType::kIncrementalDCEL:
      return std::make_unique<DCELVoronoi>();
    
    case VoronoiAlgorithmType::kDivideConquer:
      return std::make_unique<DivideConquerVoronoi>();
    
    default:
      // Return default algorithm
      return std::make_unique<IncrementalVoronoi>();
  }
}

std::vector<VoronoiAlgorithmType> VoronoiAlgorithmFactory::GetAvailableTypes() {
  return {
    VoronoiAlgorithmType::kIncremental,
    VoronoiAlgorithmType::kIncrementalDCEL,
    VoronoiAlgorithmType::kDivideConquer,
  };
}

}  // namespace geometry
