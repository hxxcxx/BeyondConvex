#ifndef GEOMETRY_CORE_EDGE2D_H_
#define GEOMETRY_CORE_EDGE2D_H_

#include "point2d.h"
#include "vector2d.h"
#include <cmath>
#include <iostream>

namespace geometry {

class Edge2D {
 public:
  Point2D p1;  // Start point
  Point2D p2;  // End point

  Edge2D() : p1(), p2() {}
  Edge2D(const Point2D& p1, const Point2D& p2) : p1(p1), p2(p2) {}

  // Get the direction vector of the edge (from p1 to p2)
  Vector2D Direction() const {
    return p2 - p1;
  }

  // Get the length of the edge
  double Length() const {
    return p1.DistanceTo(p2);
  }

  // Get the squared length of the edge (faster, no sqrt)
  double LengthSquared() const {
    return p1.DistanceSquaredTo(p2);
  }

  // Get the midpoint of the edge
  Point2D Midpoint() const {
    return Point2D((p1.x + p2.x) / 2.0, (p1.y + p2.y) / 2.0);
  }

  // Check if a point is on the edge (including endpoints)
  bool Contains(const Point2D& point) const {
    // Check if point is collinear with the edge
    Vector2D v1 = p2 - p1;
    Vector2D v2 = point - p1;
    double cross = v1.Cross(v2);
    
    if (std::abs(cross) > 1e-9) {
      return false;  // Not collinear
    }
    
    // Check if point is within the bounding box of the edge
    double min_x = std::min(p1.x, p2.x);
    double max_x = std::max(p1.x, p2.x);
    double min_y = std::min(p1.y, p2.y);
    double max_y = std::max(p1.y, p2.y);
    
    return point.x >= min_x - 1e-9 && point.x <= max_x + 1e-9 &&
           point.y >= min_y - 1e-9 && point.y <= max_y + 1e-9;
  }

  // Check if point is on the edge (excluding endpoints)
  bool ContainsInterior(const Point2D& point) const {
    return Contains(point) && point != p1 && point != p2;
  }

  // Get the perpendicular vector (rotated 90 degrees counter-clockwise)
  Vector2D Perpendicular() const {
    Vector2D dir = Direction();
    return Vector2D(-dir.y, dir.x);
  }

  // Comparison operators (for sorting, etc.)
  bool operator==(const Edge2D& other) const {
    return p1 == other.p1 && p2 == other.p2;
  }

  bool operator!=(const Edge2D& other) const {
    return !(*this == other);
  }

  // Output
  friend std::ostream& operator<<(std::ostream& os, const Edge2D& e) {
    os << "Edge(" << e.p1 << " -> " << e.p2 << ")";
    return os;
  }
};

}  // namespace geometry

#endif  // GEOMETRY_CORE_EDGE2D_H_
