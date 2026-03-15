#ifndef GEOMETRY_VORONOI_DCEL_HELPER_H_
#define GEOMETRY_VORONOI_DCEL_HELPER_H_

#include "../dcel/dcel.h"
#include "voronoi_types.h"
#include "half_plane_clipper.h"
#include <map>
#include <set>
#include <vector>

namespace geometry {

/**
 * @brief Helper class for DCEL operations in Voronoi algorithms
 * 
 * This class provides common DCEL manipulation methods used by
 * multiple Voronoi algorithms, avoiding code duplication.
 */
class DCELHelper {
 public:
  /**
   * @brief Create a bounding box cell in DCEL
   * @param dcel DCEL structure
   * @param bounds Bounding box
   * @return Pointer to created face
   */
  static Face* CreateBoundingBoxCell(
      DCEL* dcel,
      const VoronoiBounds& bounds);
  
  /**
   * @brief Create a polygon cell in DCEL
   * @param dcel DCEL structure
   * @param vertices Vertices of the polygon (in CCW order)
   * @return Pointer to created face
   */
  static Face* CreatePolygonCell(
      DCEL* dcel,
      const std::vector<Point2D>& vertices);
  
  /**
   * @brief Clip a face by a half-plane and update DCEL
   * @param dcel DCEL structure
   * @param face Face to clip
   * @param point_on_line Point on the clipping line
   * @param normal Normal vector pointing towards retained half-plane
   * @return Pointer to updated face (may be new or original)
   */
  static Face* ClipFaceByHalfPlane(
      DCEL* dcel,
      Face* face,
      const Point2D& point_on_line,
      const Vector2D& normal);
  
  /**
   * @brief Merge shared edges in DCEL (set twin relationships)
   * @param dcel DCEL structure
   */
  static void MergeSharedEdges(DCEL* dcel);
  
  /**
   * @brief Convert DCEL to VoronoiDiagramResult
   * @param dcel DCEL structure
   * @param sites Original site points
   * @return Voronoi diagram result
   */
  static VoronoiDiagramResult ConvertDCELToResult(
      const DCEL* dcel,
      const std::vector<Point2D>& sites);
  
  /**
   * @brief Convert DCEL to VoronoiDiagramResult with explicit face-site mapping
   * @param dcel DCEL structure
   * @param sites Original site points
   * @param face_to_site Mapping from face index to site index
   * @return Voronoi diagram result
   */
  static VoronoiDiagramResult ConvertDCELToResultWithMapping(
      const DCEL* dcel,
      const std::vector<Point2D>& sites,
      const std::vector<size_t>& face_to_site);
  
  /**
   * @brief Copy all faces from one DCEL to another
   * @param src_dcel Source DCEL
   * @param dst_dcel Destination DCEL
   * @param vertex_map Map to track vertex pointers (optional)
   */
  static void CopyFaces(
      DCEL* src_dcel,
      DCEL* dst_dcel,
      std::map<Point2D, Vertex*>& vertex_map);
  
  /**
   * @brief Clip a face by multiple half-planes
   * @param dcel DCEL structure
   * @param face Face to clip
   * @param clip_points Points on clipping lines
   * @param clip_normals Normal vectors of clipping lines
   * @return Pointer to final face after all clippings
   */
  static Face* ClipFaceByMultipleHalfPlanes(
      DCEL* dcel,
      Face* face,
      const std::vector<Point2D>& clip_points,
      const std::vector<Vector2D>& clip_normals);
  
  /**
   * @brief Compute the center of a face
   * @param face Face to compute center for
   * @return Center point
   */
  static Point2D ComputeFaceCenter(Face* face);
  
  /**
   * @brief Find the closest site to a face center
   * @param face_center Center of the face
   * @param sites Candidate sites
   * @return Closest site
   */
  static Point2D FindClosestSite(
      const Point2D& face_center,
      const std::vector<Point2D>& sites);
};

}  // namespace geometry

#endif  // GEOMETRY_VORONOI_DCEL_HELPER_H_
