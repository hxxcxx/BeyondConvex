#include "triangulation_utils.h"
#include "../core/geometry_core.h"
#include <algorithm>
#include <cmath>

namespace geometry {
namespace triangulation_utils {

bool TriangulationUtils::IsSimplePolygon(const std::vector<Point2D>& polygon) {
  // Check if any non-adjacent edges intersect
  size_t n = polygon.size();
  for (size_t i = 0; i < n; ++i) {
    for (size_t j = i + 2; j < n; ++j) {
      // Skip adjacent edges and first/last edge
      if (AreAdjacentInPolygon(i, j, polygon)) continue;
      
      size_t i_next = (i + 1) % n;
      size_t j_next = (j + 1) % n;
      
      if (SegmentsIntersect(polygon[i], polygon[i_next], polygon[j], polygon[j_next])) {
        return false;
      }
    }
  }
  return true;
}

bool TriangulationUtils::IsCCW(const std::vector<Point2D>& polygon) {
  return SignedArea(polygon) > 0;
}

double TriangulationUtils::SignedArea(const std::vector<Point2D>& polygon) {
  double area = 0.0;
  size_t n = polygon.size();
  for (size_t i = 0; i < n; ++i) {
    size_t j = (i + 1) % n;
    area += (polygon[j].x - polygon[i].x) * (polygon[j].y + polygon[i].y);
  }
  return area / 2.0;
}

bool TriangulationUtils::IsValidDiagonal(
    const std::vector<Point2D>& polygon,
    size_t i, size_t j) {
  size_t n = polygon.size();
  
  // Check if vertices are adjacent
  if (AreAdjacentInPolygon(i, j, polygon)) {
    return false;
  }
  
  // Check if diagonal is inside polygon
  if (!IsDiagonalInside(polygon, i, j)) {
    return false;
  }
  
  // Check if diagonal intersects any polygon edge
  if (DiagonalIntersectsEdges(polygon, i, j)) {
    return false;
  }
  
  return true;
}

bool TriangulationUtils::IsDiagonalInside(
    const std::vector<Point2D>& polygon,
    size_t i, size_t j) {
  size_t n = polygon.size();
  size_t i_prev = (i + n - 1) % n;
  size_t i_next = (i + 1) % n;
  
  // Diagonal is inside if it's convex at both endpoints
  bool convex_i = IsConvex(polygon[i_prev], polygon[i], polygon[i_next]);
  
  // Check if midpoint of diagonal is inside polygon
  Point2D midpoint = (polygon[i] + polygon[j]) / 2.0;
  
  // Simple check: midpoint should be on same side of all edges
  // For a convex polygon, this is sufficient
  // For concave polygons, need more sophisticated check
  return convex_i;
}

bool TriangulationUtils::DiagonalIntersectsEdges(
    const std::vector<Point2D>& polygon,
    size_t i, size_t j) {
  size_t n = polygon.size();
  
  for (size_t k = 0; k < n; ++k) {
    // Skip edges adjacent to i or j
    if (AreAdjacentInPolygon(i, k, polygon) || 
        AreAdjacentInPolygon(j, k, polygon)) {
      continue;
    }
    
    size_t k_next = (k + 1) % n;
    
    if (SegmentsIntersect(polygon[i], polygon[j], polygon[k], polygon[k_next])) {
      return true;
    }
  }
  
  return false;
}

bool TriangulationUtils::AreAdjacentInPolygon(
    size_t i,
    size_t j,
    const std::vector<Point2D>& polygon) {
  size_t n = polygon.size();
  return (i == j) || 
         ((i + 1) % n == j) || 
         ((j + 1) % n == i);
}

double TriangulationUtils::Cross(const Point2D& a, const Point2D& b, const Point2D& c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

bool TriangulationUtils::PointInTriangle(
    const Point2D& p,
    const Point2D& a,
    const Point2D& b,
    const Point2D& c) {
  // Use barycentric coordinates
  double cross1 = Cross(a, b, p);
  double cross2 = Cross(b, c, p);
  double cross3 = Cross(c, a, p);
  
  const double eps = 1e-9;
  
  // Point is inside if all cross products have the same sign (or zero)
  bool pos = (cross1 > -eps) && (cross2 > -eps) && (cross3 > -eps);
  bool neg = (cross1 < eps) && (cross2 < eps) && (cross3 < eps);
  
  return pos || neg;
}

bool TriangulationUtils::PointStrictlyInTriangle(
    const Point2D& p,
    const Point2D& a,
    const Point2D& b,
    const Point2D& c) {
  double cross1 = Cross(a, b, p);
  double cross2 = Cross(b, c, p);
  double cross3 = Cross(c, a, p);
  
  const double eps = 1e-9;
  
  // Point is strictly inside if all cross products have the same non-zero sign
  bool pos = (cross1 > eps) && (cross2 > eps) && (cross3 > eps);
  bool neg = (cross1 < -eps) && (cross2 < -eps) && (cross3 < -eps);
  
  return pos || neg;
}

bool TriangulationUtils::IsConvex(
    const Point2D& prev,
    const Point2D& curr,
    const Point2D& next) {
  return Cross(prev, curr, next) > 0;
}

void TriangulationUtils::RemoveCollinear(std::vector<Point2D>& poly) {
  if (poly.size() < 3) return;
  
  std::vector<size_t> to_remove;
  size_t n = poly.size();
  
  for (size_t i = 0; i < n; ++i) {
    size_t prev = (i + n - 1) % n;
    size_t next = (i + 1) % n;
    
    if (std::abs(Cross(poly[prev], poly[i], poly[next])) < 1e-9) {
      to_remove.push_back(i);
    }
  }
  
  // Remove in reverse order to maintain indices
  std::sort(to_remove.rbegin(), to_remove.rend());
  for (size_t idx : to_remove) {
    poly.erase(poly.begin() + idx);
  }
}

bool TriangulationUtils::SegmentsIntersect(
    const Point2D& p1, const Point2D& p2,
    const Point2D& p3, const Point2D& p4) {
  
  // Check if segments (p1,p2) and (p3,p4) intersect
  double d1 = Cross(p3, p4, p1);
  double d2 = Cross(p3, p4, p2);
  double d3 = Cross(p1, p2, p3);
  double d4 = Cross(p1, p2, p4);
  
  const double eps = 1e-9;
  
  // General case: segments straddle each other
  if (((d1 > eps && d2 < -eps) || (d1 < -eps && d2 > eps)) &&
      ((d3 > eps && d4 < -eps) || (d3 < -eps && d4 > eps))) {
    return true;
  }
  
  // Special cases: collinear or endpoint on segment
  if (std::abs(d1) < eps && PointOnSegment(p1, p3, p4)) return true;
  if (std::abs(d2) < eps && PointOnSegment(p2, p3, p4)) return true;
  if (std::abs(d3) < eps && PointOnSegment(p3, p1, p2)) return true;
  if (std::abs(d4) < eps && PointOnSegment(p4, p1, p2)) return true;
  
  return false;
}

bool TriangulationUtils::PointOnSegment(
    const Point2D& p,
    const Point2D& p1,
    const Point2D& p2) {
  
  // Check if point p is on segment (p1, p2)
  double cross = Cross(p1, p2, p);
  if (std::abs(cross) > 1e-9) return false;
  
  // Check if point is within bounding box of segment
  double min_x = std::min(p1.x, p2.x);
  double max_x = std::max(p1.x, p2.x);
  double min_y = std::min(p1.y, p2.y);
  double max_y = std::max(p1.y, p2.y);
  
  return p.x >= min_x - 1e-9 && p.x <= max_x + 1e-9 &&
         p.y >= min_y - 1e-9 && p.y <= max_y + 1e-9;
}

}  // namespace triangulation_utils
}  // namespace geometry
