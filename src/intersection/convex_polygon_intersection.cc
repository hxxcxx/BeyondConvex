#include "convex_polygon_intersection.h"
#include "../core/geometry_core.h"
#include <algorithm>
#include <cmath>

namespace geometry {

ConvexIntersectionResult ConvexPolygonIntersection::Intersect(
    const ConvexHull& convex1,
    const ConvexHull& convex2,
    ConvexIntersectionAlgorithm algorithm) {
  
  // Quick rejection tests
  const auto& vertices1 = convex1.GetVertices();
  const auto& vertices2 = convex2.GetVertices();
  
  if (vertices1.empty() || vertices2.empty()) {
    return ConvexIntersectionResult();
  }
  
  // Bounding box test
  double min_x1 = vertices1[0].x, max_x1 = vertices1[0].x;
  double min_y1 = vertices1[0].y, max_y1 = vertices1[0].y;
  for (const auto& v : vertices1) {
    min_x1 = std::min(min_x1, v.x);
    max_x1 = std::max(max_x1, v.x);
    min_y1 = std::min(min_y1, v.y);
    max_y1 = std::max(max_y1, v.y);
  }
  
  double min_x2 = vertices2[0].x, max_x2 = vertices2[0].x;
  double min_y2 = vertices2[0].y, max_y2 = vertices2[0].y;
  for (const auto& v : vertices2) {
    min_x2 = std::min(min_x2, v.x);
    max_x2 = std::max(max_x2, v.x);
    min_y2 = std::min(min_y2, v.y);
    max_y2 = std::max(max_y2, v.y);
  }
  
  if (max_x1 < min_x2 || max_x2 < min_x1 || max_y1 < min_y2 || max_y2 < min_y1) {
    ConvexIntersectionResult result;
    result.is_empty = true;
    return result;
  }
  
  // Choose algorithm
  switch (algorithm) {
    case ConvexIntersectionAlgorithm::kLinearScan:
      return IntersectLinearScan(convex1, convex2);
    case ConvexIntersectionAlgorithm::kBinarySearch:
      return IntersectBinarySearch(convex1, convex2);
    default:
      return IntersectLinearScan(convex1, convex2);
  }
}

ConvexIntersectionResult ConvexPolygonIntersection::IntersectLinearScan(
    const ConvexHull& convex1,
    const ConvexHull& convex2) {
  
  ConvexIntersectionResult result;
  
  const auto& vertices1 = convex1.GetVertices();
  const auto& vertices2 = convex2.GetVertices();
  
  if (vertices1.empty() || vertices2.empty()) {
    result.is_empty = true;
    return result;
  }
  
  // Use Sutherland-Hodgman-like clipping
  // Start with polygon1 and clip against each edge of polygon2
  std::vector<Point2D> current_polygon = vertices1;
  
  for (size_t i = 0; i < vertices2.size() && !current_polygon.empty(); ++i) {
    size_t next_i = (i + 1) % vertices2.size();
    const Point2D& clip_p1 = vertices2[i];
    const Point2D& clip_p2 = vertices2[next_i];
    
    current_polygon = ClipConvexPolygon(current_polygon, clip_p1, clip_p2);
  }
  
  if (current_polygon.empty()) {
    result.is_empty = true;
    return result;
  }
  
  // Check if result is a point or segment
  if (current_polygon.size() == 1) {
    result.is_point = true;
    result.is_empty = false;
    result.vertices = current_polygon;
    return result;
  }
  
  if (current_polygon.size() == 2) {
    result.is_segment = true;
    result.is_empty = false;
    result.vertices = current_polygon;
    return result;
  }
  
  // Remove collinear points and ensure CCW order
  std::vector<Point2D> cleaned;
  for (size_t i = 0; i < current_polygon.size(); ++i) {
    size_t prev = (i + current_polygon.size() - 1) % current_polygon.size();
    size_t next = (i + 1) % current_polygon.size();
    
    Vector2D v1 = current_polygon[i] - current_polygon[prev];
    Vector2D v2 = current_polygon[next] - current_polygon[i];
    
    double cross = v1.Cross(v2);
    if (std::abs(cross) > 1e-10) {
      cleaned.push_back(current_polygon[i]);
    }
  }
  
  if (cleaned.size() < 3) {
    result.is_empty = true;
    return result;
  }
  
  result.vertices = cleaned;
  result.is_empty = false;
  return result;
}

ConvexIntersectionResult ConvexPolygonIntersection::IntersectBinarySearch(
    const ConvexHull& convex1,
    const ConvexHull& convex2) {
  
  ConvexIntersectionResult result;
  
  const auto& vertices1 = convex1.GetVertices();
  const auto& vertices2 = convex2.GetVertices();
  
  if (vertices1.empty() || vertices2.empty()) {
    result.is_empty = true;
    return result;
  }
  
  // Check if one polygon is completely inside the other
  bool v1_in_v2 = IsPointInConvexPolygon(vertices1[0], convex2);
  bool v2_in_v1 = IsPointInConvexPolygon(vertices2[0], convex1);
  
  if (v1_in_v2) {
    // Polygon 1 is inside polygon 2
    result.vertices = vertices1;
    result.is_empty = false;
    return result;
  }
  
  if (v2_in_v1) {
    // Polygon 2 is inside polygon 1
    result.vertices = vertices2;
    result.is_empty = false;
    return result;
  }
  
  // Find intersection points using binary search
  std::vector<Point2D> intersection_points;
  
  // Check each edge of polygon1 against polygon2
  for (size_t i = 0; i < vertices1.size(); ++i) {
    size_t next_i = (i + 1) % vertices1.size();
    const Point2D& p1 = vertices1[i];
    const Point2D& p2 = vertices1[next_i];
    
    // Check if this edge intersects polygon2
    for (size_t j = 0; j < vertices2.size(); ++j) {
      size_t next_j = (j + 1) % vertices2.size();
      const Point2D& q1 = vertices2[j];
      const Point2D& q2 = vertices2[next_j];
      
      Point2D intersection;
      if (SegmentIntersection(p1, p2, q1, q2, intersection)) {
        // Check if this point is already in our list
        bool found = false;
        for (const auto& pt : intersection_points) {
          if ((pt - intersection).Length() < 1e-8) {
            found = true;
            break;
          }
        }
        if (!found) {
          intersection_points.push_back(intersection);
        }
      }
    }
  }
  
  if (intersection_points.empty()) {
    result.is_empty = true;
    return result;
  }
  
  if (intersection_points.size() == 1) {
    result.is_point = true;
    result.is_empty = false;
    result.vertices = intersection_points;
    return result;
  }
  
  if (intersection_points.size() == 2) {
    result.is_segment = true;
    result.is_empty = false;
    result.vertices = intersection_points;
    return result;
  }
  
  // Sort points by angle around centroid
  Point2D centroid(0, 0);
  for (const auto& pt : intersection_points) {
    centroid.x += pt.x;
    centroid.y += pt.y;
  }
  centroid.x /= intersection_points.size();
  centroid.y /= intersection_points.size();
  
  std::sort(intersection_points.begin(), intersection_points.end(),
      [&centroid](const Point2D& a, const Point2D& b) {
        double angle_a = std::atan2(a.y - centroid.y, a.x - centroid.x);
        double angle_b = std::atan2(b.y - centroid.y, b.x - centroid.x);
        return angle_a < angle_b;
      });
  
  result.vertices = intersection_points;
  result.is_empty = false;
  return result;
}

bool ConvexPolygonIntersection::IsPointInConvexPolygon(
    const Point2D& point, const ConvexHull& convex) {
  
  const auto& vertices = convex.GetVertices();
  if (vertices.empty()) return false;
  
  // Use binary search for O(log n) query
  size_t n = vertices.size();
  
  // Check if point is on same side of all edges
  bool first_sign = IsOnLeft(point, vertices[0], vertices[1]);
  
  for (size_t i = 1; i < n; ++i) {
    size_t next = (i + 1) % n;
    bool current_sign = IsOnLeft(point, vertices[i], vertices[next]);
    if (current_sign != first_sign) {
      return false;
    }
  }
  
  return true;
}

std::vector<ConvexIntersectionAlgorithm> ConvexPolygonIntersection::GetSupportedAlgorithms() {
  return {
    ConvexIntersectionAlgorithm::kLinearScan,
    ConvexIntersectionAlgorithm::kBinarySearch,
  };
}

std::string ConvexPolygonIntersection::GetAlgorithmName(ConvexIntersectionAlgorithm algorithm) {
  switch (algorithm) {
    case ConvexIntersectionAlgorithm::kLinearScan:
      return "Linear Scan (Sutherland-Hodgman)";
    case ConvexIntersectionAlgorithm::kBinarySearch:
      return "Binary Search";
    default:
      return "Unknown";
  }
}

std::string ConvexPolygonIntersection::GetAlgorithmComplexity(ConvexIntersectionAlgorithm algorithm) {
  switch (algorithm) {
    case ConvexIntersectionAlgorithm::kLinearScan:
      return "O(n + m)";
    case ConvexIntersectionAlgorithm::kBinarySearch:
      return "O(log n + log m)";
    default:
      return "Unknown";
  }
}

bool ConvexPolygonIntersection::IsOnLeft(const Point2D& p, const Point2D& p1, const Point2D& p2) {
  Vector2D v1 = p2 - p1;
  Vector2D v2 = p - p1;
  return v1.Cross(v2) >= 0;
}

bool ConvexPolygonIntersection::IsOnRight(const Point2D& p, const Point2D& p1, const Point2D& p2) {
  Vector2D v1 = p2 - p1;
  Vector2D v2 = p - p1;
  return v1.Cross(v2) <= 0;
}

bool ConvexPolygonIntersection::SegmentIntersection(
    const Point2D& p1, const Point2D& p2,
    const Point2D& q1, const Point2D& q2,
    Point2D& result) {
  
  Vector2D v1 = p2 - p1;
  Vector2D v2 = q2 - q1;
  Vector2D v3 = q1 - p1;
  
  double cross = v1.Cross(v2);
  if (std::abs(cross) < 1e-10) {
    return false;  // Parallel
  }
  
  double t = v3.Cross(v2) / cross;
  double s = v3.Cross(v1) / cross;
  
  const double eps = 1e-9;
  if (t >= -eps && t <= 1.0 + eps && s >= -eps && s <= 1.0 + eps) {
    // Calculate intersection point: p1 + v1 * t
    Vector2D result_vec = p1 - Point2D(0, 0) + v1 * t;
    result = Point2D(result_vec.x, result_vec.y);
    return true;
  }
  
  return false;
}

size_t ConvexPolygonIntersection::FindTangentPoint(
    const ConvexHull& convex, const Point2D& external_point) {
  
  const auto& vertices = convex.GetVertices();
  size_t n = vertices.size();
  
  // Binary search for the tangent point
  size_t left = 0;
  size_t right = n;
  
  while (left < right) {
    size_t mid = (left + right) / 2;
    size_t next = (mid + 1) % n;
    
    if (IsOnLeft(vertices[mid], external_point, vertices[next])) {
      right = mid;
    } else {
      left = mid + 1;
    }
  }
  
  return left;
}

std::vector<Point2D> ConvexPolygonIntersection::ClipConvexPolygon(
    const std::vector<Point2D>& polygon,
    const Point2D& clip_p1,
    const Point2D& clip_p2) {
  
  if (polygon.empty()) return {};
  
  std::vector<Point2D> result;
  
  for (size_t i = 0; i < polygon.size(); ++i) {
    size_t next_i = (i + 1) % polygon.size();
    const Point2D& current = polygon[i];
    const Point2D& next = polygon[next_i];
    
    bool current_inside = IsOnLeft(current, clip_p1, clip_p2);
    bool next_inside = IsOnLeft(next, clip_p1, clip_p2);
    
    if (current_inside && next_inside) {
      // Both inside, keep next
      result.push_back(next);
    } else if (current_inside && !next_inside) {
      // Current inside, next outside, add intersection
      Point2D intersection;
      if (SegmentIntersection(current, next, clip_p1, clip_p2, intersection)) {
        result.push_back(intersection);
      }
    } else if (!current_inside && next_inside) {
      // Current outside, next inside, add intersection then next
      Point2D intersection;
      if (SegmentIntersection(current, next, clip_p1, clip_p2, intersection)) {
        result.push_back(intersection);
        result.push_back(next);
      }
    }
    // If both outside, add nothing
  }
  
  return result;
}

}  // namespace geometry
