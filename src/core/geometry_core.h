
#pragma once
#include "../core/point2d.h"
#include "../core/vector2d.h"
#include "../convex_hull/convex_hull.h"
#include <vector>

namespace geometry {
namespace internal {

// Internal implementation class (not exported)
// This class contains the actual algorithm implementations
// and can be modified without breaking ABI compatibility
class GeometryCore {
 public:
  // To-Left Test implementation
  static bool ToLeftTest(const Point2D& p, const Point2D& q, const Point2D& r);
  
  static bool ToLeftTest(const Vector2D& u, const Vector2D& v);
};

}  // namespace internal
}  // namespace geometry

