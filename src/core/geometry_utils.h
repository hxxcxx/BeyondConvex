#ifndef GEOMETRY_CORE_GEOMETRY_UTILS_H_
#define GEOMETRY_CORE_GEOMETRY_UTILS_H_

#include "point2d.h"
#include "vector2d.h"
#include <vector>

namespace geometry {

// Basic geometric utility functions
// This class contains fundamental geometric operations used by various algorithms
class GeometryUtils {
 public:
  // To-Left Test: Check if point r is to the left of the directed line pq
  // Returns: true if r is to the left (counter-clockwise), false otherwise
  static bool ToLeftTest(const Point2D& p, const Point2D& q, const Point2D& r);

  // To-Left Test with vectors: Check if vector v is to the left of vector u
  // Returns: true if v is to the left of u (counter-clockwise), false otherwise
  static bool ToLeftTest(const Vector2D& u, const Vector2D& v);
};

}  // namespace geometry

#endif  // GEOMETRY_CORE_GEOMETRY_UTILS_H_
