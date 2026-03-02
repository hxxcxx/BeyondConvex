#ifndef GEOMETRY_CORE_POINT_H_
#define GEOMETRY_CORE_POINT_H_

#include <cmath>
#include <iostream>

namespace geometry {

class Point {
 public:
  Point() : x_(0.0), y_(0.0) {}
  Point(double x, double y) : x_(x), y_(y) {}

  // Accessors
  double X() const { return x_; }
  double Y() const { return y_; }
  void SetX(double x) { x_ = x; }
  void SetY(double y) { y_ = y; }

  // Distance calculations
  double DistanceTo(const Point& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;
    return std::sqrt(dx * dx + dy * dy);
  }

  double DistanceSquaredTo(const Point& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;
    return dx * dx + dy * dy;
  }

  // Comparison operators
  bool operator==(const Point& other) const {
    return std::abs(x_ - other.x_) < 1e-9 && 
           std::abs(y_ - other.y_) < 1e-9;
  }

  bool operator!=(const Point& other) const {
    return !(*this == other);
  }

  bool operator<(const Point& other) const {
    if (std::abs(x_ - other.x_) > 1e-9) {
      return x_ < other.x_;
    }
    return y_ < other.y_;
  }

  // Arithmetic operators
  Point operator+(const Point& other) const {
    return Point(x_ + other.x_, y_ + other.y_);
  }

  Point operator-(const Point& other) const {
    return Point(x_ - other.x_, y_ - other.y_);
  }

  Point operator*(double scalar) const {
    return Point(x_ * scalar, y_ * scalar);
  }

  Point operator/(double scalar) const {
    return Point(x_ / scalar, y_ / scalar);
  }

  // Output
  friend std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << "(" << p.x_ << ", " << p.y_ << ")";
    return os;
  }

 private:
  double x_;
  double y_;
};

}  // namespace geometry

#endif  // GEOMETRY_CORE_POINT_H_
