#include "geometry_core.h"

namespace geometry {
namespace internal {

bool GeometryCore::ToLeftTest(const Point2D& p, const Point2D& q, const Point2D& r) {
  Vector2D pq = q - p;
  Vector2D pr = r - p;
  return pq.Cross(pr) > 0;
}

bool GeometryCore::ToLeftTest(const Vector2D& u, const Vector2D& v) {
  return u.Cross(v) > 0;
}

}  // namespace internal
}  // namespace geometry
