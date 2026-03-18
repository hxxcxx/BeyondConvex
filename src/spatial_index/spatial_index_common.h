#ifndef GEOMETRY_SPATIAL_INDEX_SPATIAL_INDEX_COMMON_H_
#define GEOMETRY_SPATIAL_INDEX_SPATIAL_INDEX_COMMON_H_

#include "../core/point2d.h"

namespace geometry {

// Axis-aligned bounding box for spatial indexing
struct BoundingBox {
  double min_x, min_y;
  double max_x, max_y;
  
  BoundingBox() : min_x(0), min_y(0), max_x(0), max_y(0) {}
  BoundingBox(double min_x, double min_y, double max_x, double max_y)
      : min_x(min_x), min_y(min_y), max_x(max_x), max_y(max_y) {}
  
  double Width() const { return max_x - min_x; }
  double Height() const { return max_y - min_y; }
  double Area() const { return Width() * Height(); }
  
  Point2D Center() const {
    return Point2D((min_x + max_x) / 2.0, (min_y + max_y) / 2.0);
  }
  
  bool Contains(const Point2D& point) const {
    return point.x >= min_x && point.x <= max_x &&
           point.y >= min_y && point.y <= max_y;
  }
  
  bool Intersects(const BoundingBox& other) const {
    return !(other.max_x < min_x || other.min_x > max_x ||
             other.max_y < min_y || other.min_y > max_y);
  }
  
  // Get the four quadrants (for Quadtree)
  BoundingBox GetNW() const {
    return BoundingBox(min_x, Center().y, Center().x, max_y);
  }
  BoundingBox GetNE() const {
    return BoundingBox(Center().x, Center().y, max_x, max_y);
  }
  BoundingBox GetSW() const {
    return BoundingBox(min_x, min_y, Center().x, Center().y);
  }
  BoundingBox GetSE() const {
    return BoundingBox(Center().x, min_y, max_x, Center().y);
  }
};

}  // namespace geometry

#endif  // GEOMETRY_SPATIAL_INDEX_SPATIAL_INDEX_COMMON_H_
