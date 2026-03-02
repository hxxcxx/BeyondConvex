#include "geometry_utils.h"
#include <cmath>
#include <algorithm>

namespace geometry {

double DistanceToSegment(const Point& p, const Point& a, const Point& b) {
  // Vector AB
  double abx = b.X() - a.X();
  double aby = b.Y() - a.Y();
  
  // Vector AP
  double apx = p.X() - a.X();
  double apy = p.Y() - a.Y();
  
  // Vector BP
  double bpx = p.X() - b.X();
  double bpy = p.Y() - b.Y();
  
  // Dot products
  double ab_ap = abx * apx + aby * apy;
  double ab_bp = abx * bpx + aby * bpy;
  
  // Check if projection falls outside segment
  double ab_len_sq = abx * abx + aby * aby;
  
  if (ab_ap <= 0) {
    // Closest to A
    return std::sqrt(apx * apx + apy * apy);
  } else if (ab_bp >= 0) {
    // Closest to B
    return std::sqrt(bpx * bpx + bpy * bpy);
  } else {
    // Projection falls on segment
    double cross = std::abs(CrossProduct(a, b, p));
    return cross / std::sqrt(ab_len_sq);
  }
}

bool IsOnSegment(const Point& p, const Point& a, const Point& b) {
  // Check if p is collinear with a and b
  if (std::abs(CrossProduct(a, b, p)) > 1e-9) {
    return false;
  }
  
  // Check if p is within the bounding box of segment ab
  double min_x = std::min(a.X(), b.X());
  double max_x = std::max(a.X(), b.X());
  double min_y = std::min(a.Y(), b.Y());
  double max_y = std::max(a.Y(), b.Y());
  
  return p.X() >= min_x - 1e-9 && p.X() <= max_x + 1e-9 &&
         p.Y() >= min_y - 1e-9 && p.Y() <= max_y + 1e-9;
}

}  // namespace geometry
