#ifndef GEOMETRY_VORONOI_DCEL_H_
#define GEOMETRY_VORONOI_DCEL_H_

#include "voronoi_algorithm.h"
#include "half_plane_clipper.h"
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
   * @brief Create initial bounding box cell in DCEL
   * @param dcel DCEL structure
   * @param site Site point
   * @param bounds Bounding box
   * @return Pointer to created face
   */
  Face* CreateBoundingBoxCell(
      DCEL* dcel,
      const Point2D& site,
      const VoronoiBounds& bounds) const;
  
  /**
   * @brief Clip a DCEL face by a half-plane
   * @param dcel DCEL structure
   * @param face Face to clip
   * @param point_on_line Point on the clipping line
   * @param normal Normal vector pointing towards retained half-plane
   */
  void ClipFaceByHalfPlane(
      DCEL* dcel,
      Face* face,
      const Point2D& point_on_line,
      const Vector2D& normal) const;
  
  /**
   * @brief Find the face that contains a given point
   * @param dcel DCEL structure
   * @param point Point to locate
   * @param face_sites Map of faces to their sites
   * @return Pointer to containing face, or nullptr if not found
   */
  Face* FindFaceContainingPoint(
      DCEL* dcel,
      const Point2D& point,
      const std::map<Face*, Point2D>& face_sites) const;
  
  /**
   * @brief Split a face by a line (bisector)
   * @param dcel DCEL structure
   * @param face Face to split
   * @param point_on_line Point on the splitting line
   * @param normal Normal vector of the line
   * @return Pointer to new face (the part on the positive side)
   */
  Face* SplitFaceByLine(
      DCEL* dcel,
      Face* face,
      const Point2D& point_on_line,
      const Vector2D& normal) const;
  
  /**
   * @brief Merge shared edges in DCEL
   * @param dcel DCEL structure
   * 
   * This method identifies edges that are shared between multiple
   * faces and sets up proper twin relationships.
   */
  void MergeSharedEdges(DCEL* dcel) const;
  
  /**
   * @brief Convert DCEL to VoronoiDiagramResult
   * @param dcel DCEL structure
   * @param sites Original site points
   * @return Voronoi diagram result
   */
  VoronoiDiagramResult ConvertDCELToResult(
      const DCEL* dcel,
      const std::vector<Point2D>& sites) const;
};

}  // namespace geometry

#endif  // GEOMETRY_VORONOI_DCEL_H_
