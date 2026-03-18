
#pragma once
#include "core/point2d.h"
#include "core/vector2d.h"
#include "convex_hull/convex_hull.h"
#include <vector>

namespace geometry {

// Public facade class for geometry operations
// This class provides a stable API for library users
// All implementations are delegated to internal::GeometryCore
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

