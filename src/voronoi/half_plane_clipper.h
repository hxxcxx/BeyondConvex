
#pragma once
#include "../core/point2d.h"
#include "../core/vector2d.h"
#include <vector>

namespace geometry {

/**
 * @brief Utility class for half-plane clipping operations
 * 
 * This class provides methods to clip polygons against half-planes,
 * which is a fundamental operation in Voronoi diagram generation.
 */
class HalfPlaneClipper {
 public:
  /**
   * @brief Clip a polygon against a half-plane
   * 
   * A half-plane is defined by a line (point_on_line, normal).
   * The half-plane consists of all points p where (p - point_on_line) · normal >= 0.
   * 
   * @param polygon Input polygon (vertices in order)
   * @param point_on_line A point on the dividing line
   * @param normal Normal vector pointing towards the retained half-plane
   * @return Clipped polygon (vertices in order)
   */
  static std::vector<Point2D> ClipPolygon(
      const std::vector<Point2D>& polygon,
      const Point2D& point_on_line,
      const Vector2D& normal);
  
  /**
   * @brief Compute the intersection of a line segment with a half-plane boundary
   * 
   * @param seg_start Start point of the segment
   * @param seg_end End point of the segment
   * @param point_on_line A point on the line
   * @param normal Normal vector of the line
   * @return Intersection point
   */
  static Point2D IntersectSegmentWithLine(
      const Point2D& seg_start,
      const Point2D& seg_end,
      const Point2D& point_on_line,
      const Vector2D& normal);
  
  /**
   * @brief Check if a point is inside a half-plane
   * 
   * @param point Point to check
   * @param point_on_line A point on the line
   * @param normal Normal vector pointing towards the half-plane
   * @return true if the point is inside (or on the boundary)
   */
  static bool IsInsideHalfPlane(
      const Point2D& point,
      const Point2D& point_on_line,
      const Vector2D& normal);
  
  /**
   * @brief Compute the perpendicular bisector of two points
   * 
   * The bisector is the line that is equidistant from both points.
   * 
   * @param p1 First point
   * @param p2 Second point
   * @param out_midpoint Output: midpoint of p1 and p2
   * @param out_normal Output: normal vector pointing towards p1
   */
  static void ComputeBisector(
      const Point2D& p1,
      const Point2D& p2,
      Point2D& out_midpoint,
      Vector2D& out_normal);
  
  /**
   * @brief Clip a polygon against multiple half-planes
   * 
   * @param polygon Input polygon
   * @param point_on_line Points on each dividing line
   * @param normals Normal vectors for each line
   * @return Clipped polygon
   */
  static std::vector<Point2D> ClipPolygonByMultipleHalfPlanes(
      const std::vector<Point2D>& polygon,
      const std::vector<Point2D>& point_on_line,
      const std::vector<Vector2D>& normals);
  
 private:
  // Tolerance for floating point comparisons
  static constexpr double EPSILON = 1e-10;
};

}  // namespace geometry

