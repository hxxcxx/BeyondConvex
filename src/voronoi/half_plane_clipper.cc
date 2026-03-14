#include "half_plane_clipper.h"
#include <cmath>

namespace geometry {

std::vector<Point2D> HalfPlaneClipper::ClipPolygon(
    const std::vector<Point2D>& polygon,
    const Point2D& point_on_line,
    const Vector2D& normal) {
  
  std::vector<Point2D> result;
  
  if (polygon.empty()) {
    return result;
  }
  
  // Process each edge of the polygon
  for (size_t i = 0; i < polygon.size(); ++i) {
    size_t next = (i + 1) % polygon.size();
    
    const Point2D& p1 = polygon[i];
    const Point2D& p2 = polygon[next];
    
    // Check which side each point is on
    bool inside1 = IsInsideHalfPlane(p1, point_on_line, normal);
    bool inside2 = IsInsideHalfPlane(p2, point_on_line, normal);
    
    // Add p1 if it's inside
    if (inside1) {
      result.push_back(p1);
    }
    
    // If the edge crosses the line, add intersection point
    if (inside1 != inside2) {
      Point2D intersection = IntersectSegmentWithLine(
          p1, p2, point_on_line, normal);
      result.push_back(intersection);
    }
  }
  
  return result;
}

Point2D HalfPlaneClipper::IntersectSegmentWithLine(
    const Point2D& seg_start,
    const Point2D& seg_end,
    const Point2D& point_on_line,
    const Vector2D& normal) {
  
  Vector2D seg_dir = seg_end - seg_start;
  Vector2D to_line = seg_start - point_on_line;
  
  // Line: (p - point_on_line) · normal = 0
  // Segment: p = seg_start + t * seg_dir
  // Solve: (seg_start + t * seg_dir - point_on_line) · normal = 0
  
  double denom = seg_dir.x * normal.x + seg_dir.y * normal.y;
  
  if (std::abs(denom) < EPSILON) {
    // Segment is parallel to the line
    return seg_start;
  }
  
  double t = -(to_line.x * normal.x + to_line.y * normal.y) / denom;
  
  return Point2D(seg_start.x + seg_dir.x * t, seg_start.y + seg_dir.y * t);
}

bool HalfPlaneClipper::IsInsideHalfPlane(
    const Point2D& point,
    const Point2D& point_on_line,
    const Vector2D& normal) {
  
  Vector2D vec = point - point_on_line;
  double dot = vec.x * normal.x + vec.y * normal.y;
  return dot >= -EPSILON;
}

void HalfPlaneClipper::ComputeBisector(
    const Point2D& p1,
    const Point2D& p2,
    Point2D& out_midpoint,
    Vector2D& out_normal) {
  
  // Compute midpoint
  out_midpoint = Point2D(
    (p1.x + p2.x) * 0.5,
    (p1.y + p2.y) * 0.5
  );
  
  // Compute normal pointing towards p1
  out_normal = p1 - p2;
  out_normal = out_normal.Normalize();
}

std::vector<Point2D> HalfPlaneClipper::ClipPolygonByMultipleHalfPlanes(
    const std::vector<Point2D>& polygon,
    const std::vector<Point2D>& point_on_line,
    const std::vector<Vector2D>& normals) {
  
  if (point_on_line.size() != normals.size()) {
    return polygon;  // Invalid input
  }
  
  std::vector<Point2D> result = polygon;
  
  // Clip against each half-plane in sequence
  for (size_t i = 0; i < point_on_line.size(); ++i) {
    result = ClipPolygon(result, point_on_line[i], normals[i]);
    
    // Early termination if polygon becomes empty
    if (result.empty()) {
      break;
    }
  }
  
  return result;
}

}  // namespace geometry
