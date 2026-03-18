
#pragma once
#include "voronoi_types.h"
#include "../dcel/dcel.h"
#include <memory>
#include <string>

namespace geometry {

/**
 * @brief Abstract base class for Voronoi diagram algorithms
 * 
 * This interface defines the contract that all Voronoi diagram
 * algorithms must implement. It supports both the legacy
 * VoronoiDiagramResult format and the modern DCEL format.
 */
class IVoronoiAlgorithm {
 public:
  virtual ~IVoronoiAlgorithm() = default;
  
  /**
   * @brief Generate Voronoi diagram in legacy format
   * @param sites Input site points
   * @param bounds Bounding box for the diagram
   * @return Voronoi diagram result
   */
  virtual VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      const VoronoiBounds& bounds) = 0;
  
  /**
   * @brief Generate Voronoi diagram in DCEL format
   * @param sites Input site points
   * @param bounds Bounding box for the diagram
   * @return Pointer to DCEL structure (caller owns the memory)
   */
  virtual DCEL* GenerateDCEL(
      const std::vector<Point2D>& sites,
      const VoronoiBounds& bounds) = 0;
  
  /**
   * @brief Get the algorithm name
   * @return Human-readable algorithm name
   */
  virtual std::string Name() const = 0;
  
  /**
   * @brief Get the time complexity
   * @return Time complexity string (e.g., "O(n log n)")
   */
  virtual std::string Complexity() const = 0;
  
  /**
   * @brief Get the algorithm description
   * @return Detailed description of the algorithm
   */
  virtual std::string Description() const = 0;
  
  /**
   * @brief Check if the algorithm supports DCEL output
   * @return true if GenerateDCEL() is implemented
   */
  virtual bool SupportsDCEL() const {
    return true;
  }
};

/**
 * @brief Enumeration of available Voronoi algorithms
 */
enum class VoronoiAlgorithmType {
  kIncremental,      // Incremental half-plane clipping
  kIncrementalDCEL,  // Incremental algorithm using DCEL
  kDivideConquer,    // Divide and conquer algorithm
  // Future algorithms:
  // kFortune,         // Fortune's sweep line algorithm
  // kDelaunayDual,    // Based on Delaunay triangulation
};

/**
 * @brief Factory for creating Voronoi algorithm instances
 */
class VoronoiAlgorithmFactory {
 public:
  /**
   * @brief Create an algorithm instance
   * @param type Algorithm type
   * @return Unique pointer to algorithm instance
   */
  static std::unique_ptr<IVoronoiAlgorithm> Create(
      VoronoiAlgorithmType type);
  
  /**
   * @brief Get all available algorithm types
   * @return Vector of available algorithm types
   */
  static std::vector<VoronoiAlgorithmType> GetAvailableTypes();
  
  /**
   * @brief Get the default algorithm type
   * @return Default algorithm type
   */
  static VoronoiAlgorithmType GetDefaultType() {
    return VoronoiAlgorithmType::kIncremental;
  }
};

}  // namespace geometry

