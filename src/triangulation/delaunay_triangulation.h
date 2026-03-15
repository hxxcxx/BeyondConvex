#ifndef GEOMETRY_TRIANGULATION_DELAUNAY_H_
#define GEOMETRY_TRIANGULATION_DELAUNAY_H_

#include "triangulation_algorithm.h"
#include "triangulation_types.h"
#include "../dcel/dcel.h"
#include <vector>
#include <set>
#include <map>

namespace geometry {

/**
 * @brief Delaunay triangulation algorithm using Bowyer-Watson
 * 
 * This algorithm computes the Delaunay triangulation of a point set
 * using the Bowyer-Watson incremental insertion algorithm with DCEL.
 * 
 * Time Complexity: O(n²) worst case, O(n log n) average with randomization
 * Space Complexity: O(n)
 * 
 * Advantages:
 * - Produces high-quality triangulation
 * - Maximizes minimum angle
 * - Unique for generic point sets
 * - Uses DCEL for efficient topology operations
 * 
 * Disadvantages:
 * - O(n²) worst case without spatial indexing
 * - Requires careful numerical handling
 */
class DelaunayTriangulation : public ITriangulationAlgorithm {
 public:
  /**
   * @brief Compute Delaunay triangulation of point set
   * @param points Input point set (not necessarily in order)
   * @return Triangulation result
   */
  TriangulationResult Triangulate(
      const std::vector<Point2D>& points) override;
  
  /**
   * @brief Get algorithm name
   * @return Algorithm name
   */
  std::string Name() const override {
    return "Delaunay (Bowyer-Watson)";
  }
  
  /**
   * @brief Get time complexity
   * @return Complexity string
   */
  std::string Complexity() const override {
    return "O(n²) worst, O(n log n) avg";
  }
  
  /**
   * @brief Get algorithm description
   * @return Description
   */
  std::string Description() const override {
    return "Delaunay triangulation using Bowyer-Watson algorithm with DCEL. "
           "Incrementally inserts points and maintains empty circumcircle property. "
           "Produces high-quality meshes optimal for many applications.";
  }
  
 private:
  /**
   * @brief Create super triangle that contains all points
   * @param points Input points
   * @param dcel DCEL structure
   * @return Face of super triangle
   */
  Face* CreateSuperTriangle(
      const std::vector<Point2D>& points,
      DCEL* dcel);
  
  /**
   * @brief Find triangles whose circumcircle contains a point
   * @param point Point to check
   * @param dcel DCEL structure
   * @return Set of bad triangles (faces)
   */
  std::set<Face*> FindBadTriangles(
      const Point2D& point,
      DCEL* dcel);
  
  /**
   * @brief Find boundary of bad triangles (polygon hole)
   * @param bad_triangles Set of bad triangles
   * @return Boundary edges (half-edges)
   */
  std::vector<HalfEdge*> FindBoundary(
      const std::set<Face*>& bad_triangles);
  
  /**
   * @brief Remove bad triangles from DCEL
   * @param bad_triangles Set of bad triangles
   * @param dcel DCEL structure
   */
  void RemoveBadTriangles(
      const std::set<Face*>& bad_triangles,
      DCEL* dcel);
  
  /**
   * @brief Re-triangulate hole with new point
   * @param boundary Boundary edges
   * @param point New point
   * @param dcel DCEL structure
   */
  void RetriangulateHole(
      const std::vector<HalfEdge*>& boundary,
      const Point2D& point,
      DCEL* dcel);
  
  /**
   * @brief Check if point is in circumcircle of triangle
   * @param point Point to check
   * @param face Triangle face
   * @return True if point is in circumcircle
   */
  bool IsInCircumcircle(
      const Point2D& point,
      Face* face);
  
  /**
   * @brief Get circumcenter of triangle face
   * @param face Triangle face
   * @return Circumcenter point
   */
  Point2D GetCircumcenter(Face* face);
  
  /**
   * @brief Get circumradius of triangle face
   * @param face Triangle face
   * @return Circumradius
   */
  double GetCircumradius(Face* face);
  
  /**
   * @brief Convert DCEL to TriangulationResult
   * @param dcel DCEL structure
   * @param super_triangle_face Super triangle face to exclude
   * @return Triangulation result
   */
  TriangulationResult ConvertDCELToResult(
      DCEL* dcel,
      Face* super_triangle_face);
};

}  // namespace geometry

#endif  // GEOMETRY_TRIANGULATION_DELAUNAY_H_
