
#pragma once
#include "../core/point2d.h"
#include "../convex_hull/convex_hull.h"
#include <vector>
#include <memory>

namespace geometry {

// Algorithm types for convex polygon intersection
enum class ConvexIntersectionAlgorithm {
  kLinearScan,        // O(n + m) linear scan
  kBinarySearch,      // O(log n + log m) binary search
  // DobkinKirkpatrick,  // O(log n + log m) with DK hierarchy (future)
};

// Result of convex polygon intersection
struct ConvexIntersectionResult {
  std::vector<Point2D> vertices;  // Intersection polygon vertices (CCW)
  bool is_empty;                  // True if no intersection
  bool is_point;                  // True if intersection is a single point
  bool is_segment;                // True if intersection is a line segment
  
  ConvexIntersectionResult() : is_empty(true), is_point(false), is_segment(false) {}
  
  // Convert to ConvexHull for convenience
  ConvexHull ToConvexHull() const {
    if (is_empty || vertices.size() < 3) {
      return ConvexHull();
    }
    return ConvexHull(vertices);
  }
};

// Intersection algorithm for convex polygons
class ConvexPolygonIntersection {
 public:
  // Find intersection between two convex polygons
  static ConvexIntersectionResult Intersect(
      const ConvexHull& convex1,
      const ConvexHull& convex2,
      ConvexIntersectionAlgorithm algorithm = ConvexIntersectionAlgorithm::kLinearScan);
  
  // O(n + m) linear scan algorithm
  static ConvexIntersectionResult IntersectLinearScan(
      const ConvexHull& convex1,
      const ConvexHull& convex2);
  
  // O(log n + log m) binary search algorithm
  static ConvexIntersectionResult IntersectBinarySearch(
      const ConvexHull& convex1,
      const ConvexHull& convex2);
  
  // Check if a point is inside a convex polygon (O(log n))
  static bool IsPointInConvexPolygon(const Point2D& point, const ConvexHull& convex);
  
  // Get supported algorithms
  static std::vector<ConvexIntersectionAlgorithm> GetSupportedAlgorithms();
  
  // Get algorithm name
  static std::string GetAlgorithmName(ConvexIntersectionAlgorithm algorithm);
  
  // Get algorithm complexity
  static std::string GetAlgorithmComplexity(ConvexIntersectionAlgorithm algorithm);
  
 private:
  // Helper: Find intersection point of two line segments
  static bool SegmentIntersection(const Point2D& p1, const Point2D& p2,
                                   const Point2D& q1, const Point2D& q2,
                                   Point2D& result);
  
  // Helper: Binary search for tangency point
  static size_t FindTangentPoint(const ConvexHull& convex, const Point2D& external_point);
  
  // Helper: Clip convex polygon by half-plane
  static std::vector<Point2D> ClipConvexPolygon(
      const std::vector<Point2D>& polygon,
      const Point2D& clip_p1,
      const Point2D& clip_p2);
};

}  // namespace geometry

