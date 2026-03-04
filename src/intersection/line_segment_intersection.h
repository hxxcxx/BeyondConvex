#ifndef GEOMETRY_INTERSECTION_LINE_SEGMENT_INTERSECTION_H_
#define GEOMETRY_INTERSECTION_LINE_SEGMENT_INTERSECTION_H_

#include "../core/point2d.h"
#include "../core/edge2d.h"
#include <vector>
#include <set>
#include <queue>

namespace geometry {

// Intersection point with additional information
struct IntersectionPoint {
  Point2D point;
  int segment1_index;  // Index of first segment
  int segment2_index;  // Index of second segment
  
  IntersectionPoint(const Point2D& p, int s1, int s2)
      : point(p), segment1_index(s1), segment2_index(s2) {}
  
  bool operator==(const IntersectionPoint& other) const {
    return point.x == other.point.x && point.y == other.point.y;
  }
};

// Event point for Bentley-Ottmann algorithm
struct EventPoint {
  Point2D point;
  bool is_left_endpoint;  // True if this is a left endpoint
  int segment_index;      // Index of the segment
  
  EventPoint(const Point2D& p, bool is_left, int seg_idx)
      : point(p), is_left_endpoint(is_left), segment_index(seg_idx) {}
  
  // Comparator for priority queue (min-heap by y, then x)
  bool operator>(const EventPoint& other) const {
    if (point.y != other.point.y) {
      return point.y > other.point.y;  // Higher y has lower priority
    }
    return point.x > other.point.x;
  }
};

// Line segment intersection using Bentley-Ottmann sweep line algorithm
class LineSegmentIntersection {
 public:
  // Find all intersections among line segments
  // Time complexity: O((n + k) log n) where n is number of segments, k is number of intersections
  static std::vector<IntersectionPoint> FindAllIntersections(
      const std::vector<Edge2D>& segments);
  
  // Simple O(n²) algorithm for verification
  static std::vector<IntersectionPoint> FindAllIntersectionsBruteForce(
      const std::vector<Edge2D>& segments);
  
  // Check if two segments intersect
  static bool SegmentsIntersect(const Edge2D& seg1, const Edge2D& seg2,
                                Point2D& intersection);

 private:
  // Get the y-coordinate of a segment at a given x (for sweep line ordering)
  static double GetYAtX(const Edge2D& segment, double x);
  
  // Compare two segments by their y-coordinate at the sweep line x position
  static bool CompareSegmentsAtX(const Edge2D& seg1, const Edge2D& seg2, double x);
};

}  // namespace geometry

#endif  // GEOMETRY_INTERSECTION_LINE_SEGMENT_INTERSECTION_H_
