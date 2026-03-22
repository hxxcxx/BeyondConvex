#include "geometry_utils.h"
#include "../core/geometry_core.h"

namespace geometry {

bool GeometryUtils::ToLeftTest(const Point2D& p, const Point2D& q, const Point2D& r) {
  // Delegate to internal implementation
  return internal::GeometryCore::ToLeftTest(p, q, r);
}

bool GeometryUtils::ToLeftTest(const Vector2D& u, const Vector2D& v) {
  // Delegate to internal implementation
  return internal::GeometryCore::ToLeftTest(u, v);
}

} // namespace geometry
