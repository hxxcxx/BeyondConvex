
#pragma once
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

// Event type for Bentley-Ottmann algorithm
enum class EventType {
  LeftEndpoint,   // Segment enters sweep line
  RightEndpoint,  // Segment leaves sweep line
  Intersection    // Two segments intersect
};

// Event point for Bentley-Ottmann algorithm
struct EventPoint {
  Point2D point;
  EventType type;
  int segment_index;      // Index of the segment (for endpoint events)
  int segment_index2;     // Index of second segment (for intersection events)
  
  // Constructor for endpoint events
  EventPoint(const Point2D& p, EventType t, int seg_idx)
      : point(p), type(t), segment_index(seg_idx), segment_index2(-1) {}
  
  // Constructor for intersection events
  EventPoint(const Point2D& p, int seg_idx1, int seg_idx2)
      : point(p), type(EventType::Intersection), segment_index(seg_idx1), segment_index2(seg_idx2) {}
  
  // Comparator for priority queue (min-heap by x, then y)
  // Process events from left to right
  bool operator>(const EventPoint& other) const {
    if (point.x != other.point.x) {
      return point.x > other.point.x;  // Larger x has lower priority (process left to right)
    }
    if (point.y != other.point.y) {
      return point.y > other.point.y;
    }
    // If points are equal, process in order: LeftEndpoint, Intersection, RightEndpoint
    return static_cast<int>(type) > static_cast<int>(other.type);
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

  // Get the y-coordinate of a segment at a given x (for visualization)
  static double GetYAtX(const Edge2D& segment, double x);

 private:
  // Compare two segments by their y-coordinate at the sweep line x position
  static bool CompareSegmentsAtX(const Edge2D& seg1, const Edge2D& seg2, double x);
};

}  // namespace geometry

