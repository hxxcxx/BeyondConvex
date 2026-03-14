#ifndef GEOMETRY_VORONOI_H_
#define GEOMETRY_VORONOI_H_

#include "voronoi_types.h"
#include "voronoi_algorithm.h"
#include "../dcel/dcel.h"
#include <memory>
#include <vector>

namespace geometry {

/**
 * @brief Facade class for Voronoi diagram generation
 * 
 * This class provides a simple, unified interface for generating
 * Voronoi diagrams using different algorithms. It supports both
 * the legacy VoronoiDiagramResult format and the modern DCEL format.
 * 
 * Usage:
 * @code
 *   // Simple usage (default algorithm)
 *   auto result = Voronoi::Generate(sites);
 * 
 *   // Specify algorithm
 *   auto result = Voronoi::Generate(sites, VoronoiAlgorithmType::kIncrementalDCEL);
 * 
 *   // Generate DCEL
 *   DCEL* dcel = Voronoi::GenerateDCEL(sites);
 * @endcode
 */
class Voronoi {
 public:
  /**
   * @brief Generate Voronoi diagram using default algorithm
   * @param sites Input site points
   * @param bounds Bounding box for the diagram
   * @return Voronoi diagram result
   */
  static VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      const VoronoiBounds& bounds = VoronoiBounds());
  
  /**
   * @brief Generate Voronoi diagram using specified algorithm
   * @param sites Input site points
   * @param algorithm Algorithm type to use
   * @param bounds Bounding box for the diagram
   * @return Voronoi diagram result
   */
  static VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      VoronoiAlgorithmType algorithm,
      const VoronoiBounds& bounds = VoronoiBounds());
  
  /**
   * @brief Generate Voronoi diagram in DCEL format
   * @param sites Input site points
   * @param algorithm Algorithm type to use
   * @param bounds Bounding box for the diagram
   * @return Pointer to DCEL structure (caller owns the memory)
   */
  static DCEL* GenerateDCEL(
      const std::vector<Point2D>& sites,
      VoronoiAlgorithmType algorithm = VoronoiAlgorithmType::kIncrementalDCEL,
      const VoronoiBounds& bounds = VoronoiBounds());
  
  /**
   * @brief Get all available algorithm types
   * @return Vector of available algorithm types
   */
  static std::vector<VoronoiAlgorithmType> GetAvailableAlgorithms();
  
  /**
   * @brief Get algorithm information
   * @param algorithm Algorithm type
   * @return Algorithm instance (for querying information)
   */
  static std::unique_ptr<IVoronoiAlgorithm> GetAlgorithmInfo(
      VoronoiAlgorithmType algorithm);
  
  /**
   * @brief Get the default algorithm type
   * @return Default algorithm type
   */
  static VoronoiAlgorithmType GetDefaultAlgorithm() {
    return VoronoiAlgorithmFactory::GetDefaultType();
  }
};

/**
 * @brief Legacy compatibility class (deprecated)
 * 
 * This class maintains backward compatibility with the old
 * VoronoiDiagram interface. New code should use Voronoi instead.
 * 
 * @deprecated Use Voronoi class instead
 */
class [[deprecated("Use Voronoi class instead")]] VoronoiDiagram {
 public:
  /**
   * @brief Generate Voronoi diagram (legacy interface)
   * @param sites Input site points
   * @param bounds_min_x Minimum x coordinate
   * @param bounds_min_y Minimum y coordinate
   * @param bounds_max_x Maximum x coordinate
   * @param bounds_max_y Maximum y coordinate
   * @return Voronoi diagram result
   */
  static VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      double bounds_min_x = -100.0,
      double bounds_min_y = -100.0,
      double bounds_max_x = 100.0,
      double bounds_max_y = 100.0);
  
  /**
   * @brief Generate Voronoi diagram in DCEL format
   * @param sites Input site points
   * @param bounds_min_x Minimum x coordinate
   * @param bounds_min_y Minimum y coordinate
   * @param bounds_max_x Maximum x coordinate
   * @param bounds_max_y Maximum y coordinate
   * @return Pointer to DCEL structure
   */
  static DCEL* GenerateDCEL(
      const std::vector<Point2D>& sites,
      double bounds_min_x = -100.0,
      double bounds_min_y = -100.0,
      double bounds_max_x = 100.0,
      double bounds_max_y = 100.0);
};

}  // namespace geometry

#endif  // GEOMETRY_VORONOI_H_
