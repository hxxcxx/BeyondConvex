
#pragma once
#include "voronoi_algorithm.h"
#include "half_plane_clipper.h"
#include <set>

namespace geometry {

/**
 * @brief Incremental Voronoi diagram algorithm using half-plane clipping
 * 
 * This algorithm computes the Voronoi diagram by iteratively clipping
 * each cell against half-planes defined by perpendicular bisectors.
 * 
 * Time Complexity: O(n²) ~ O(n³) depending on input distribution
 * Space Complexity: O(n²)
 * 
 * Advantages:
 * - Simple and intuitive
 * - Easy to implement
 * - Works well for small datasets
 * 
 * Disadvantages:
 * - No topological information
 * - Shared edges are duplicated
 * - Slower for large datasets
 */
class IncrementalVoronoi : public IVoronoiAlgorithm {
 public:
  /**
   * @brief Generate Voronoi diagram
   * @param sites Input site points
   * @param bounds Bounding box for the diagram
   * @return Voronoi diagram result
   */
  VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      const VoronoiBounds& bounds) override;
  
  /**
   * @brief Generate Voronoi diagram in DCEL format
   * @param sites Input site points
   * @param bounds Bounding box for the diagram
   * @return Pointer to DCEL structure
   */
  DCEL* GenerateDCEL(
      const std::vector<Point2D>& sites,
      const VoronoiBounds& bounds) override;
  
  /**
   * @brief Get algorithm name
   * @return Algorithm name
   */
  std::string Name() const override {
    return "Incremental Half-Plane Clipping";
  }
  
  /**
   * @brief Get time complexity
   * @return Complexity string
   */
  std::string Complexity() const override {
    return "O(n²) ~ O(n³)";
  }
  
  /**
   * @brief Get algorithm description
   * @return Description
   */
  std::string Description() const override {
    return "Incremental algorithm using half-plane clipping. "
           "Simple implementation but slower for large datasets. "
           "No topological information maintained.";
  }
  
 private:
  /**
   * @brief Create initial bounding box polygon
   * @param bounds Bounding box
   * @return Bounding box as polygon
   */
  std::vector<Point2D> CreateBoundingBox(const VoronoiBounds& bounds) const;
  
  /**
   * @brief Compute Voronoi cell for a single site
   * @param site The site point
   * @param all_sites All site points
   * @param site_index Index of the current site
   * @param bounds Bounding box
   * @return Voronoi cell polygon
   */
  std::vector<Point2D> ComputeCell(
      const Point2D& site,
      const std::vector<Point2D>& all_sites,
      size_t site_index,
      const VoronoiBounds& bounds) const;
};

}  // namespace geometry

