#include "voronoi.h"

namespace geometry {

// ========== Voronoi Facade Class ==========

VoronoiDiagramResult Voronoi::Generate(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) {
  
  // Use default algorithm
  return Generate(sites, GetDefaultAlgorithm(), bounds);
}

VoronoiDiagramResult Voronoi::Generate(
    const std::vector<Point2D>& sites,
    VoronoiAlgorithmType algorithm,
    const VoronoiBounds& bounds) {
  
  auto algo = VoronoiAlgorithmFactory::Create(algorithm);
  return algo->Generate(sites, bounds);
}

DCEL* Voronoi::GenerateDCEL(
    const std::vector<Point2D>& sites,
    VoronoiAlgorithmType algorithm,
    const VoronoiBounds& bounds) {
  
  auto algo = VoronoiAlgorithmFactory::Create(algorithm);
  return algo->GenerateDCEL(sites, bounds);
}

std::vector<VoronoiAlgorithmType> Voronoi::GetAvailableAlgorithms() {
  return VoronoiAlgorithmFactory::GetAvailableTypes();
}

std::unique_ptr<IVoronoiAlgorithm> Voronoi::GetAlgorithmInfo(
    VoronoiAlgorithmType algorithm) {
  
  return VoronoiAlgorithmFactory::Create(algorithm);
}

// ========== VoronoiDiagram Legacy Class ==========

VoronoiDiagramResult VoronoiDiagram::Generate(
    const std::vector<Point2D>& sites,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  VoronoiBounds bounds(bounds_min_x, bounds_min_y, bounds_max_x, bounds_max_y);
  return Voronoi::Generate(sites, bounds);
}

DCEL* VoronoiDiagram::GenerateDCEL(
    const std::vector<Point2D>& sites,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  VoronoiBounds bounds(bounds_min_x, bounds_min_y, bounds_max_x, bounds_max_y);
  return Voronoi::GenerateDCEL(sites, VoronoiAlgorithmType::kIncrementalDCEL, bounds);
}

}  // namespace geometry
