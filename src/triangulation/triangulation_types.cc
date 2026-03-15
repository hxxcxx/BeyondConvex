#include "triangulation_types.h"
#include "../core/geometry_core.h"
#include <cmath>
#include <algorithm>

namespace geometry {

bool Triangle::Contains(const Point2D& p) const {
  // Use barycentric coordinates or check if point is on same side of all edges
  auto same_side = [](const Point2D& p1, const Point2D& p2,
                      const Point2D& a, const Point2D& b) {
    Vector2D v1 = b - a;
    Vector2D v2 = p1 - a;
    Vector2D v3 = p2 - a;
    double cross1 = v1.Cross(v2);
    double cross2 = v1.Cross(v3);
    const double eps = 1e-9;
    // Both points should be on the same side (including boundary)
    return (cross1 > -eps && cross2 > -eps) ||
           (cross1 < eps && cross2 < eps);
  };

  return same_side(p, v0, v1, v2) &&
         same_side(p, v1, v0, v2) &&
         same_side(p, v2, v0, v1);
}

Point2D Triangle::Circumcenter() const {
  // Calculate circumcenter using perpendicular bisectors
  double ax = v0.x, ay = v0.y;
  double bx = v1.x, by = v1.y;
  double cx = v2.x, cy = v2.y;
  
  double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
  if (std::abs(d) < 1e-10) {
    // Degenerate triangle, return centroid
    return Point2D((ax + bx + cx) / 3.0, (ay + by + cy) / 3.0);
  }
  
  double ux = ((ax * ax + ay * ay) * (by - cy) + 
               (bx * bx + by * by) * (cy - ay) + 
               (cx * cx + cy * cy) * (ay - by)) / d;
  
  double uy = ((ax * ax + ay * ay) * (cx - bx) + 
               (bx * bx + by * by) * (ax - cx) + 
               (cx * cx + cy * cy) * (bx - ax)) / d;
  
  return Point2D(ux, uy);
}

double Triangle::Circumradius() const {
  Point2D center = Circumcenter();
  return v0.DistanceTo(center);
}

}  // namespace geometry
