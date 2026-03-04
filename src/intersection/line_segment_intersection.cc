#include "line_segment_intersection.h"
#include "../core/geometry_core.h"
#include <cmath>
#include <algorithm>
#include <map>

namespace geometry {

std::vector<IntersectionPoint> LineSegmentIntersection::FindAllIntersections(
    const std::vector<Edge2D>& segments) {
  std::vector<IntersectionPoint> intersections;
  
  if (segments.size() < 2) {
    return intersections;
  }
  
  // Step 1: Initialize event queue with all segment endpoints
  std::priority_queue<EventPoint, std::vector<EventPoint>, std::greater<EventPoint>> event_queue;
  
  for (size_t i = 0; i < segments.size(); ++i) {
    const Edge2D& seg = segments[i];
    
    // Determine left and right endpoints (by x coordinate)
    if (seg.p1.x < seg.p2.x) {
      event_queue.emplace(seg.p1, true, static_cast<int>(i));   // Left endpoint
      event_queue.emplace(seg.p2, false, static_cast<int>(i));  // Right endpoint
    } else if (seg.p1.x > seg.p2.x) {
      event_queue.emplace(seg.p2, true, static_cast<int>(i));   // Left endpoint
      event_queue.emplace(seg.p1, false, static_cast<int>(i));  // Right endpoint
    } else {
      // Vertical segment, use y coordinate
      if (seg.p1.y < seg.p2.y) {
        event_queue.emplace(seg.p1, true, static_cast<int>(i));
        event_queue.emplace(seg.p2, false, static_cast<int>(i));
      } else {
        event_queue.emplace(seg.p2, true, static_cast<int>(i));
        event_queue.emplace(seg.p1, false, static_cast<int>(i));
      }
    }
  }
  
  // Step 2: Sweep line state (ordered by y-coordinate at current x)
  // Using a map to maintain order: key is the y-coordinate at current x
  struct SegmentInfo {
    int index;
    double y_at_current_x;
    
    SegmentInfo(int idx, double y) : index(idx), y_at_current_x(y) {}
    
    bool operator<(const SegmentInfo& other) const {
      return y_at_current_x < other.y_at_current_x;
    }
  };
  
  std::set<SegmentInfo> sweep_line;
  
  // Step 3: Process events
  while (!event_queue.empty()) {
    EventPoint event = event_queue.top();
    event_queue.pop();
    
    double current_x = event.point.x;
    
    if (event.is_left_endpoint) {
      // Left endpoint: insert segment into sweep line
      const Edge2D& seg = segments[event.segment_index];
      double y = GetYAtX(seg, current_x);
      
      // Find neighbors
      SegmentInfo new_seg(event.segment_index, y);
      auto it = sweep_line.insert(new_seg).first;
      
      // Check for intersections with neighbors
      if (it != sweep_line.begin()) {
        auto prev = it;
        --prev;
        Point2D intersection;
        if (SegmentsIntersect(seg, segments[prev->index], intersection)) {
          intersections.emplace_back(intersection, event.segment_index, prev->index);
        }
      }
      
      auto next = it;
      ++next;
      if (next != sweep_line.end()) {
        Point2D intersection;
        if (SegmentsIntersect(seg, segments[next->index], intersection)) {
          intersections.emplace_back(intersection, event.segment_index, next->index);
        }
      }
      
    } else {
      // Right endpoint: remove segment from sweep line
      const Edge2D& seg = segments[event.segment_index];
      double y = GetYAtX(seg, current_x);
      
      SegmentInfo target(event.segment_index, y);
      auto it = sweep_line.find(target);
      
      if (it != sweep_line.end()) {
        // Get neighbors before removing
        auto prev = it;
        auto next = it;
        ++next;
        
        if (it != sweep_line.begin()) {
          --prev;
        } else {
          prev = sweep_line.end();
        }
        
        // Remove segment
        sweep_line.erase(it);
        
        // Check if neighbors now intersect
        if (prev != sweep_line.end() && next != sweep_line.end()) {
          Point2D intersection;
          if (SegmentsIntersect(segments[prev->index], segments[next->index], intersection)) {
            intersections.emplace_back(intersection, prev->index, next->index);
          }
        }
      }
    }
  }
  
  return intersections;
}

std::vector<IntersectionPoint> LineSegmentIntersection::FindAllIntersectionsBruteForce(
    const std::vector<Edge2D>& segments) {
  std::vector<IntersectionPoint> intersections;
  
  for (size_t i = 0; i < segments.size(); ++i) {
    for (size_t j = i + 1; j < segments.size(); ++j) {
      Point2D intersection;
      if (SegmentsIntersect(segments[i], segments[j], intersection)) {
        intersections.emplace_back(intersection, static_cast<int>(i), static_cast<int>(j));
      }
    }
  }
  
  return intersections;
}

bool LineSegmentIntersection::SegmentsIntersect(const Edge2D& seg1, const Edge2D& seg2,
                                                  Point2D& intersection) {
  // Use orientation test (To-Left Test)
  bool o1 = internal::GeometryCore::ToLeftTest(seg1.p1, seg1.p2, seg2.p1);
  bool o2 = internal::GeometryCore::ToLeftTest(seg1.p1, seg1.p2, seg2.p2);
  bool o3 = internal::GeometryCore::ToLeftTest(seg2.p1, seg2.p2, seg1.p1);
  bool o4 = internal::GeometryCore::ToLeftTest(seg2.p1, seg2.p2, seg1.p2);
  
  // General case
  if (o1 != o2 && o3 != o4) {
    // Calculate intersection point using line-line intersection formula
    // Line 1: p1 + t * (p2 - p1)
    // Line 2: p3 + s * (p4 - p3)
    Vector2D v1 = seg1.Direction();
    Vector2D v2 = seg2.Direction();
    Vector2D v3 = seg2.p1 - seg1.p1;
    
    double cross = v1.Cross(v2);
    if (std::abs(cross) < 1e-10) {
      return false;  // Parallel or collinear
    }
    
    double t = v3.Cross(v2) / cross;
    Vector2D result = seg1.p1 - Point2D(0, 0) + v1 * t;
    intersection = Point2D(result.x, result.y);
    return true;
  }
  
  // Special cases: collinear or endpoint intersections
  // For simplicity, we'll skip these in this implementation
  return false;
}

double LineSegmentIntersection::GetYAtX(const Edge2D& segment, double x) {
  if (std::abs(segment.p2.x - segment.p1.x) < 1e-10) {
    // Vertical segment, return y at the point
    return segment.p1.y;
  }
  
  // Calculate y using line equation: y - y1 = m * (x - x1)
  double m = (segment.p2.y - segment.p1.y) / (segment.p2.x - segment.p1.x);
  return segment.p1.y + m * (x - segment.p1.x);
}

bool LineSegmentIntersection::CompareSegmentsAtX(const Edge2D& seg1, const Edge2D& seg2, double x) {
  double y1 = GetYAtX(seg1, x);
  double y2 = GetYAtX(seg2, x);
  
  if (std::abs(y1 - y2) > 1e-10) {
    return y1 < y2;
  }
  
  // If y coordinates are equal, compare by slope
  double slope1 = (seg1.p2.y - seg1.p1.y) / (seg1.p2.x - seg1.p1.x);
  double slope2 = (seg2.p2.y - seg2.p1.y) / (seg2.p2.x - seg2.p1.x);
  return slope1 < slope2;
}

}  // namespace geometry
