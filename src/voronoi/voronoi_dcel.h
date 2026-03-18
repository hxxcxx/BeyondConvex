
#pragma once
#include "voronoi_algorithm.h"
#include "dcel_helper.h"
#include "../dcel/dcel.h"
#include <map>
#include <vector>

namespace geometry {

/**
 * @brief Incremental Voronoi diagram algorithm using DCEL structure
 * 
 * This algorithm computes the Voronoi diagram while maintaining
 * topological relationships through the DCEL data structure.
 * 
 * Time Complexity: O(n²)
 * Space Complexity: O(n²)
 * 
 * Advantages:
 * - Maintains topological information
 * - Shared edges are properly linked
 * - Supports efficient adjacency queries
 * - Can be extended for dynamic updates
 * 
 * Disadvantages:
 * - More complex implementation
 * - Still O(n²) time complexity
 */
class DCELVoronoi : public IVoronoiAlgorithm {
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
    return "Incremental DCEL";
  }
  
  /**
   * @brief Get time complexity
   * @return Complexity string
   */
  std::string Complexity() const override {
    return "O(n²)";
  }
  
  /**
   * @brief Get algorithm description
   * @return Description
   */
  std::string Description() const override {
    return "Incremental algorithm using DCEL data structure. "
           "Maintains topological relationships between cells. "
           "Supports efficient adjacency queries.";
  }
  
 private:
  /**
   * @brief Clip a cell by all other sites
   * @param dcel DCEL structure
   * @param cell Cell to clip
   * @param sites All sites
   * @param cell_index Index of the cell's site
   * @param bounds Bounding box
   */
  void ClipCellByAllSites(
      DCEL* dcel,
      Face* cell,
      const std::vector<Point2D>& sites,
      size_t cell_index,
      const VoronoiBounds& bounds) const;
};

}  // namespace geometry

