#include "geometry_utils.h"
#include <cmath>

namespace geometry {

bool GeometryUtils::ToLeftTest(const Point2D& p, const Point2D& q, const Point2D& r) {
    Vector2D pq = q - p;
    Vector2D pr = r - p;
    return pq.Cross(pr) > 0;
}

bool GeometryUtils::ToLeftTest(const Vector2D& v1, const Vector2D& v2) {
    return v1.Cross(v2) > 0;
}

} // namespace geometry
