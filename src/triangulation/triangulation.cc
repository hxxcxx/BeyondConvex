#include "triangulation.h"

namespace geometry {

TriangulationResult Triangulation::Triangulate(
    const std::vector<Point2D>& points,
    TriangulationAlgorithmType algorithm) {
  auto algo = TriangulationFactory::Create(algorithm);
  return algo->Triangulate(points);
}

std::vector<TriangulationAlgorithmType> Triangulation::GetSupportedAlgorithms() {
  return TriangulationFactory::GetSupportedAlgorithms();
}

std::string Triangulation::GetAlgorithmName(TriangulationAlgorithmType algorithm) {
  return TriangulationFactory::GetAlgorithmName(algorithm);
}

std::string Triangulation::GetAlgorithmComplexity(TriangulationAlgorithmType algorithm) {
  return TriangulationFactory::GetAlgorithmComplexity(algorithm);
}

}  // namespace geometry
