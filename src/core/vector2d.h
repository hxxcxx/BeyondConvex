#ifndef GEOMETRY_CORE_VECTOR2D_H_
#define GEOMETRY_CORE_VECTOR2D_H_

#include <cmath>
#include <iostream>

namespace geometry {

class Vector2D {
 public:
  double x;
  double y;

  Vector2D() : x(0.0), y(0.0) {}
  Vector2D(double x, double y) : x(x), y(y) {}

  // Vector operations
  double Length() const {
    return std::sqrt(x * x + y * y);
  }

  double LengthSquared() const {
    return x * x + y * y;
  }

  Vector2D Normalize() const {
    double len = Length();
    if (len > 1e-9) {
      return Vector2D(x / len, y / len);
    }
    return Vector2D(0.0, 0.0);
  }

  // Dot product
  double Dot(const Vector2D& other) const {
    return x * other.x + y * other.y;
  }

  // Cross product (2D, returns scalar)
  double Cross(const Vector2D& other) const {
    return x * other.y - y * other.x;
  }

  // Arithmetic operators
  Vector2D operator+(const Vector2D& other) const {
    return Vector2D(x + other.x, y + other.y);
  }

  Vector2D operator-(const Vector2D& other) const {
    return Vector2D(x - other.x, y - other.y);
  }

  Vector2D operator*(double scalar) const {
    return Vector2D(x * scalar, y * scalar);
  }

  Vector2D operator/(double scalar) const {
    return Vector2D(x / scalar, y / scalar);
  }

  Vector2D operator-() const {
    return Vector2D(-x, -y);
  }

  // Comparison operators
  bool operator==(const Vector2D& other) const {
    return std::abs(x - other.x) < 1e-9 &&
           std::abs(y - other.y) < 1e-9;
  }

  bool operator!=(const Vector2D& other) const {
    return !(*this == other);
  }

  bool operator<(const Vector2D& other) const {
    if (std::abs(x - other.x) > 1e-9) {
      return x < other.x;
    }
    return y < other.y;
  }

  // Output
  friend std::ostream& operator<<(std::ostream& os, const Vector2D& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
  }
};

}  // namespace geometry

#endif  // GEOMETRY_CORE_VECTOR2D_H_
