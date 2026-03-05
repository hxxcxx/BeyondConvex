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
  
  // Helper structure for sweep line ordering
  struct SegmentInfo {
    int index;
    double y_at_current_x;
    const std::vector<Edge2D>* segments_ptr;
    
    SegmentInfo(int idx, double y, const std::vector<Edge2D>* segs)
        : index(idx), y_at_current_x(y), segments_ptr(segs) {}
    
    bool operator<(const SegmentInfo& other) const {
      if (std::abs(y_at_current_x - other.y_at_current_x) > 1e-10) {
        return y_at_current_x < other.y_at_current_x;
      }
      // If y coordinates are equal, compare by slope
      const Edge2D& seg1 = (*segments_ptr)[index];
      const Edge2D& seg2 = (*segments_ptr)[other.index];
      double slope1 = (seg1.p2.y - seg1.p1.y) / (seg1.p2.x - seg1.p1.x);
      double slope2 = (seg2.p2.y - seg2.p1.y) / (seg2.p2.x - seg2.p1.x);
      return slope1 < slope2;
    }
  };
  
  // Step 1: Initialize event queue with all segment endpoints
  std::priority_queue<EventPoint, std::vector<EventPoint>, std::greater<EventPoint>> event_queue;
  
  for (size_t i = 0; i < segments.size(); ++i) {
    const Edge2D& seg = segments[i];
    
    // Determine left and right endpoints (by x coordinate)
    if (seg.p1.x < seg.p2.x) {
      event_queue.emplace(seg.p1, EventType::LeftEndpoint, static_cast<int>(i));
      event_queue.emplace(seg.p2, EventType::RightEndpoint, static_cast<int>(i));
    } else if (seg.p1.x > seg.p2.x) {
      event_queue.emplace(seg.p2, EventType::LeftEndpoint, static_cast<int>(i));
      event_queue.emplace(seg.p1, EventType::RightEndpoint, static_cast<int>(i));
    } else {
      // Vertical segment, use y coordinate
      if (seg.p1.y < seg.p2.y) {
        event_queue.emplace(seg.p1, EventType::LeftEndpoint, static_cast<int>(i));
        event_queue.emplace(seg.p2, EventType::RightEndpoint, static_cast<int>(i));
      } else {
        event_queue.emplace(seg.p2, EventType::LeftEndpoint, static_cast<int>(i));
        event_queue.emplace(seg.p1, EventType::RightEndpoint, static_cast<int>(i));
      }
    }
  }
  
  // Step 2: Sweep line state (ordered by y-coordinate at current x)
  std::set<SegmentInfo> sweep_line;
  
  // Track which intersections have been found to avoid duplicates
  std::set<std::pair<int, int>> found_intersections;
  
  // Helper lambda to add intersection event
  auto add_intersection_event = [&](const Point2D& intersection, int seg1, int seg2) {
    // Only add if intersection is to the right of current sweep line
    // and we haven't found this intersection before
    auto pair = std::minmax(seg1, seg2);
    if (found_intersections.find(pair) == found_intersections.end()) {
      found_intersections.insert(pair);
      intersections.emplace_back(intersection, seg1, seg2);
      event_queue.emplace(intersection, seg1, seg2);
    }
  };
  
  // Step 3: Process events
  while (!event_queue.empty()) {
    EventPoint event = event_queue.top();
    event_queue.pop();
    
    double current_x = event.point.x;
    
    if (event.type == EventType::LeftEndpoint) {
      // Left endpoint: insert segment into sweep line
      const Edge2D& seg = segments[event.segment_index];
      double y = GetYAtX(seg, current_x);
      
      // Insert into sweep line
      SegmentInfo new_seg(event.segment_index, y, &segments);
      auto it = sweep_line.insert(new_seg).first;
      
      // Check for intersections with predecessor
      if (it != sweep_line.begin()) {
        auto prev = it;
        --prev;
        Point2D intersection;
        if (SegmentsIntersect(seg, segments[prev->index], intersection)) {
          if (intersection.x > current_x - 1e-10) {  // Intersection is to the right
            add_intersection_event(intersection, event.segment_index, prev->index);
          }
        }
      }
      
      // Check for intersections with successor
      auto next = it;
      ++next;
      if (next != sweep_line.end()) {
        Point2D intersection;
        if (SegmentsIntersect(seg, segments[next->index], intersection)) {
          if (intersection.x > current_x - 1e-10) {  // Intersection is to the right
            add_intersection_event(intersection, event.segment_index, next->index);
          }
        }
      }
      
    } else if (event.type == EventType::RightEndpoint) {
      // Right endpoint: remove segment from sweep line
      const Edge2D& seg = segments[event.segment_index];
      double y = GetYAtX(seg, current_x);
      
      SegmentInfo target(event.segment_index, y, &segments);
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
            if (intersection.x > current_x - 1e-10) {  // Intersection is to the right
              add_intersection_event(intersection, prev->index, next->index);
            }
          }
        }
      }
      
    } else {  // EventType::Intersection
      // Two segments intersect: swap their positions in sweep line
      int seg1_idx = event.segment_index;
      int seg2_idx = event.segment_index2;
      
      const Edge2D& seg1 = segments[seg1_idx];
      const Edge2D& seg2 = segments[seg2_idx];
      
      double y1 = GetYAtX(seg1, current_x);
      double y2 = GetYAtX(seg2, current_x);
      
      // Find and remove both segments
      SegmentInfo info1(seg1_idx, y1, &segments);
      SegmentInfo info2(seg2_idx, y2, &segments);
      
      auto it1 = sweep_line.find(info1);
      auto it2 = sweep_line.find(info2);
      
      if (it1 != sweep_line.end() && it2 != sweep_line.end()) {
        // Get neighbors before removing
        auto prev1 = it1, next1 = it1;
        auto prev2 = it2, next2 = it2;
        
        if (it1 != sweep_line.begin()) { --prev1; } else { prev1 = sweep_line.end(); }
        if (it2 != sweep_line.begin()) { --prev2; } else { prev2 = sweep_line.end(); }
        ++next1;
        ++next2;
        
        // Remove both segments
        sweep_line.erase(it1);
        sweep_line.erase(it2);
        
        // Re-insert with updated y-coordinates (swapped order)
        SegmentInfo new_info1(seg1_idx, y1, &segments);
        SegmentInfo new_info2(seg2_idx, y2, &segments);
        
        auto new_it1 = sweep_line.insert(new_info1).first;
        auto new_it2 = sweep_line.insert(new_info2).first;
        
        // Check for new intersections with neighbors of seg1
        // Check predecessor of new position
        if (new_it1 != sweep_line.begin()) {
          auto prev = new_it1;
          --prev;
          if (prev->index != seg2_idx) {  // Don't check with seg2
            Point2D intersection;
            if (SegmentsIntersect(seg1, segments[prev->index], intersection)) {
              if (intersection.x > current_x + 1e-10) {
                add_intersection_event(intersection, seg1_idx, prev->index);
              }
            }
          }
        }
        
        // Check successor of new position
        auto next = new_it1;
        ++next;
        if (next != sweep_line.end() && next->index != seg2_idx) {
          Point2D intersection;
          if (SegmentsIntersect(seg1, segments[next->index], intersection)) {
            if (intersection.x > current_x + 1e-10) {
              add_intersection_event(intersection, seg1_idx, next->index);
            }
          }
        }
        
        // Check for new intersections with neighbors of seg2
        // Check predecessor of new position
        if (new_it2 != sweep_line.begin()) {
          auto prev = new_it2;
          --prev;
          if (prev->index != seg1_idx) {  // Don't check with seg1
            Point2D intersection;
            if (SegmentsIntersect(seg2, segments[prev->index], intersection)) {
              if (intersection.x > current_x + 1e-10) {
                add_intersection_event(intersection, seg2_idx, prev->index);
              }
            }
          }
        }
        
        // Check successor of new position
        next = new_it2;
        ++next;
        if (next != sweep_line.end() && next->index != seg1_idx) {
          Point2D intersection;
          if (SegmentsIntersect(seg2, segments[next->index], intersection)) {
            if (intersection.x > current_x + 1e-10) {
              add_intersection_event(intersection, seg2_idx, next->index);
            }
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
