#ifndef GEOMETRY_CORE_GEOMETRY_UTILS_H_
#define GEOMETRY_CORE_GEOMETRY_UTILS_H_

#include "point2d.h"
#include "vector2d.h"
#include "convex_hull.h"
#include <vector>

namespace geometry {

class GeometryUtils {
 public:
  // To-Left Test: Check if point r is to the left of the directed line pq
  // Returns: true if r is to the left (counter-clockwise), false otherwise
  static bool ToLeftTest(const Point2D& p, const Point2D& q, const Point2D& r);

  // To-Left Test with vectors: Check if vector v is to the left of vector u
  // Returns: true if v is to the left of u (counter-clockwise), false otherwise
  static bool ToLeftTest(const Vector2D& u, const Vector2D& v);

  // Jarvis March (Gift Wrapping) algorithm for convex hull
  // Time complexity: O(nh) where n is number of points, h is number of hull points
  // Returns: ConvexHull object with vertices in counter-clockwise order
  static ConvexHull JarvisMarch(const std::vector<Point2D>& points);
};

}  // namespace geometry

#endif  // GEOMETRY_CORE_GEOMETRY_UTILS_H_
