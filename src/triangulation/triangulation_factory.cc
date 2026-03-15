#include "triangulation_factory.h"
#include "ear_clipping_triangulation.h"
#include "sweep_line_triangulation.h"
#include "delaunay_triangulation.h"
#include <stdexcept>

namespace geometry {

std::unique_ptr<ITriangulationAlgorithm> TriangulationFactory::Create(
    TriangulationAlgorithmType type) {
  switch (type) {
    case TriangulationAlgorithmType::kSweepLine:
      return std::make_unique<SweepLineTriangulation>();
    
    case TriangulationAlgorithmType::kEarClipping:
      return std::make_unique<EarClippingTriangulation>();
    
    case TriangulationAlgorithmType::kDelaunay:
      return std::make_unique<DelaunayTriangulation>();
    
    default:
      throw std::invalid_argument("Unknown triangulation algorithm type");
  }
}

std::vector<TriangulationAlgorithmType> TriangulationFactory::GetSupportedAlgorithms() {
  return {
    TriangulationAlgorithmType::kSweepLine,
    TriangulationAlgorithmType::kEarClipping,
    TriangulationAlgorithmType::kDelaunay,
  };
}

std::string TriangulationFactory::GetAlgorithmName(TriangulationAlgorithmType type) {
  auto algo = Create(type);
  return algo->Name();
}

std::string TriangulationFactory::GetAlgorithmComplexity(TriangulationAlgorithmType type) {
  auto algo = Create(type);
  return algo->Complexity();
}

std::string TriangulationFactory::GetAlgorithmDescription(TriangulationAlgorithmType type) {
  auto algo = Create(type);
  return algo->Description();
}

}  // namespace geometry
