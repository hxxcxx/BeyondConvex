#ifndef GEOMETRY_CORE_GEOMETRY_UTILS_H_
#define GEOMETRY_CORE_GEOMETRY_UTILS_H_

#include "point.h"
#include <vector>

namespace geometry {

// Cross product of two vectors (OA x OB)
// Returns positive if O->A->B is counter-clockwise
// Returns negative if O->A->B is clockwise
// Returns zero if collinear
inline double CrossProduct(const Point& o, const Point& a, const Point& b) {
  return (a.X() - o.X()) * (b.Y() - o.Y()) - 
         (a.Y() - o.Y()) * (b.X() - o.X());
}

// Cross product of two vectors from origin
inline double CrossProduct(const Point& a, const Point& b) {
  return a.X() * b.Y() - a.Y() * b.X();
}

// Dot product
inline double DotProduct(const Point& a, const Point& b) {
  return a.X() * b.X() + a.Y() * b.Y();
}

// Distance from point p to line segment ab
double DistanceToSegment(const Point& p, const Point& a, const Point& b);

// Check if point is on segment
bool IsOnSegment(const Point& p, const Point& a, const Point& b);

// To-Left Test: Check if point r is to the left of the directed line pq
// Returns: true if r is to the left (counter-clockwise), false otherwise
inline bool ToLeftTest(const Point& p, const Point& q, const Point& r) {
  return CrossProduct(p, q, r) > 0;
}

}  // namespace geometry

#endif  // GEOMETRY_CORE_GEOMETRY_UTILS_H_
