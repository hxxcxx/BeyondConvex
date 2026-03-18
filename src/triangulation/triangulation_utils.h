
#pragma once
#include "triangulation_types.h"
#include <vector>

namespace geometry {
namespace triangulation_utils {

/**
 * @brief Utility class for triangulation algorithms
 * 
 * This class provides helper functions that are shared across
 * different triangulation algorithms.
 */
class TriangulationUtils {
 public:
  /**
   * @brief Check if polygon is simple (no self-intersections)
   * @param polygon Polygon vertices in order
   * @return True if polygon is simple
   */
  static bool IsSimplePolygon(const std::vector<Point2D>& polygon);
  
  /**
   * @brief Check if polygon vertices are in CCW order
   * @param polygon Polygon vertices in order
   * @return True if CCW order
   */
  static bool IsCCW(const std::vector<Point2D>& polygon);
  
  /**
   * @brief Calculate signed area of polygon
   * @param polygon Polygon vertices in order
   * @return Signed area (positive for CCW, negative for CW)
   */
  static double SignedArea(const std::vector<Point2D>& polygon);
  
  /**
   * @brief Check if diagonal is valid (inside polygon and doesn't intersect edges)
   * @param polygon Polygon vertices in order
   * @param i Index of first vertex
   * @param j Index of second vertex
   * @return True if diagonal is valid
   */
  static bool IsValidDiagonal(
      const std::vector<Point2D>& polygon,
      size_t i, size_t j);
  
  /**
   * @brief Check if diagonal is inside polygon
   * @param polygon Polygon vertices in order
   * @param i Index of first vertex
   * @param j Index of second vertex
   * @return True if diagonal is inside
   */
  static bool IsDiagonalInside(
      const std::vector<Point2D>& polygon,
      size_t i, size_t j);
  
  /**
   * @brief Check if diagonal intersects any polygon edge
   * @param polygon Polygon vertices in order
   * @param i Index of first vertex
   * @param j Index of second vertex
   * @return True if diagonal intersects edges
   */
  static bool DiagonalIntersectsEdges(
      const std::vector<Point2D>& polygon,
      size_t i, size_t j);
  
  /**
   * @brief Check if two vertices are adjacent in polygon
   * @param i Index of first vertex
   * @param j Index of second vertex
   * @param polygon Polygon vertices
   * @return True if adjacent
   */
  static bool AreAdjacentInPolygon(
      size_t i,
      size_t j,
      const std::vector<Point2D>& polygon);
  
  /**
   * @brief Calculate cross product (b-a) x (c-a)
   * @param a First point
   * @param b Second point
   * @param c Third point
   * @return Cross product value
   */
  static double Cross(const Point2D& a, const Point2D& b, const Point2D& c);
  
  /**
   * @brief Check if point is inside triangle (including boundary)
   * @param p Point to check
   * @param a Triangle vertex 1
   * @param b Triangle vertex 2
   * @param c Triangle vertex 3
   * @return True if point is inside or on boundary
   */
  static bool PointInTriangle(
      const Point2D& p,
      const Point2D& a,
      const Point2D& b,
      const Point2D& c);
  
  /**
   * @brief Check if point is strictly inside triangle (excluding boundary)
   * @param p Point to check
   * @param a Triangle vertex 1
   * @param b Triangle vertex 2
   * @param c Triangle vertex 3
   * @return True if point is strictly inside
   */
  static bool PointStrictlyInTriangle(
      const Point2D& p,
      const Point2D& a,
      const Point2D& b,
      const Point2D& c);
  
  /**
   * @brief Check if three points form a convex corner (CCW turn)
   * @param prev Previous point
   * @param curr Current point
   * @param next Next point
   * @return True if convex (CCW turn)
   */
  static bool IsConvex(
      const Point2D& prev,
      const Point2D& curr,
      const Point2D& next);
  
  /**
   * @brief Remove collinear vertices from polygon
   * @param poly Polygon vertices (modified in place)
   */
  static void RemoveCollinear(std::vector<Point2D>& poly);
  
  /**
   * @brief Check if two line segments intersect
   * @param p1 Start of first segment
   * @param p2 End of first segment
   * @param p3 Start of second segment
   * @param p4 End of second segment
   * @return True if segments intersect
   */
  static bool SegmentsIntersect(
      const Point2D& p1, const Point2D& p2,
      const Point2D& p3, const Point2D& p4);
  
  /**
   * @brief Check if point is on line segment
   * @param p Point to check
   * @param p1 Segment start
   * @param p2 Segment end
   * @return True if point is on segment
   */
  static bool PointOnSegment(
      const Point2D& p,
      const Point2D& p1,
      const Point2D& p2);
};

}  // namespace triangulation_utils
}  // namespace geometry

