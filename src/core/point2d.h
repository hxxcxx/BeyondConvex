
#pragma once
#include "vector2d.h"
#include <cmath>
#include <iostream>

namespace geometry {

class Vector2D;  // Forward declaration

class Point2D {
 public:
  double x;
  double y;

  Point2D() : x(0.0), y(0.0) {}
  Point2D(double x, double y) : x(x), y(y) {}

  // Distance calculations
  double DistanceTo(const Point2D& other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    return std::sqrt(dx * dx + dy * dy);
  }

  double DistanceSquaredTo(const Point2D& other) const {
    double dx = x - other.x;
    double dy = y - other.y;
    return dx * dx + dy * dy;
  }

  // Point - Point = Vector (difference vector from other to this)
  Vector2D operator-(const Point2D& other) const {
    return Vector2D(x - other.x, y - other.y);
  }

  // Comparison operators
  bool operator==(const Point2D& other) const {
    return std::abs(x - other.x) < 1e-9 &&
           std::abs(y - other.y) < 1e-9;
  }

  bool operator!=(const Point2D& other) const {
    return !(*this == other);
  }

  bool operator<(const Point2D& other) const {
    if (std::abs(x - other.x) > 1e-9) {
      return x < other.x;
    }
    return y < other.y;
  }

  // Output
  friend std::ostream& operator<<(std::ostream& os, const Point2D& p) {
    os << "(" << p.x << ", " << p.y << ")";
    return os;
  }
};

}  // namespace geometry

