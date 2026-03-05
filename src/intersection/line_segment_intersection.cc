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
  
  // Segment info for sweep line ordering
  struct SegmentInfo {
    int index;
    const std::vector<Edge2D>* segments_ptr;
    double sweep_x;  // Current sweep line position
    
    SegmentInfo(int idx, const std::vector<Edge2D>* segs, double x)
        : index(idx), segments_ptr(segs), sweep_x(x) {}
    
    // Get y-coordinate at current sweep_x
    double getY() const {
      return GetYAtX((*segments_ptr)[index], sweep_x);
    }
    
    // Get slope
    double getSlope() const {
      const Edge2D& seg = (*segments_ptr)[index];
      return (seg.p2.y - seg.p1.y) / (seg.p2.x - seg.p1.x);
    }
  };
  
  // Comparator for sweep line (BBST)
  auto segment_comparator = [](const SegmentInfo& a, const SegmentInfo& b) {
    double y_a = a.getY();
    double y_b = b.getY();
    if (std::abs(y_a - y_b) > 1e-10) {
      return y_a < y_b;
    }
    // If y coordinates are equal, compare by slope
    return a.getSlope() < b.getSlope();
  };
  
  // Sweep line: BBST (std::set is a red-black tree)
  std::set<SegmentInfo, decltype(segment_comparator)> status(segment_comparator);
  
  // Initialize event queue with all endpoints
  std::priority_queue<EventPoint, std::vector<EventPoint>, std::greater<EventPoint>> event_queue;
  
  for (size_t i = 0; i < segments.size(); ++i) {
    const Edge2D& seg = segments[i];
    
    if (seg.p1.x < seg.p2.x) {
      event_queue.emplace(seg.p1, EventType::LeftEndpoint, static_cast<int>(i));
      event_queue.emplace(seg.p2, EventType::RightEndpoint, static_cast<int>(i));
    } else if (seg.p1.x > seg.p2.x) {
      event_queue.emplace(seg.p2, EventType::LeftEndpoint, static_cast<int>(i));
      event_queue.emplace(seg.p1, EventType::RightEndpoint, static_cast<int>(i));
    } else {
      // Vertical segment
      if (seg.p1.y < seg.p2.y) {
        event_queue.emplace(seg.p1, EventType::LeftEndpoint, static_cast<int>(i));
        event_queue.emplace(seg.p2, EventType::RightEndpoint, static_cast<int>(i));
      } else {
        event_queue.emplace(seg.p2, EventType::LeftEndpoint, static_cast<int>(i));
        event_queue.emplace(seg.p1, EventType::RightEndpoint, static_cast<int>(i));
      }
    }
  }
  
  // Track found intersections
  std::set<std::pair<int, int>> found_intersections;
  
  // Process events
  while (!event_queue.empty()) {
    EventPoint event = event_queue.top();
    event_queue.pop();
    
    double current_x = event.point.x;
    
    // Update sweep_x for all segments in status
    // Since SegmentInfo stores sweep_x, we need to rebuild
    std::vector<int> segments_in_status;
    segments_in_status.reserve(status.size());
    for (const auto& info : status) {
      segments_in_status.push_back(info.index);
    }
    status.clear();
    for (int seg_idx : segments_in_status) {
      status.emplace(seg_idx, &segments, current_x);
    }
    
    if (event.type == EventType::LeftEndpoint) {
      // Insert segment into status
      auto it = status.emplace(event.segment_index, &segments, current_x).first;
      
      // Get predecessor (using BBST iterator)
      if (it != status.begin()) {
        auto prev = it;
        --prev;
        Point2D intersection;
        if (SegmentsIntersect(segments[event.segment_index], segments[prev->index], intersection)) {
          auto pair = std::minmax(event.segment_index, prev->index);
          if (found_intersections.find(pair) == found_intersections.end()) {
            found_intersections.insert(pair);
            intersections.emplace_back(intersection, event.segment_index, prev->index);
            if (intersection.x > current_x + 1e-10) {
              event_queue.emplace(intersection, event.segment_index, prev->index);
            }
          }
        }
      }
      
      // Get successor (using BBST iterator)
      auto next = it;
      ++next;
      if (next != status.end()) {
        Point2D intersection;
        if (SegmentsIntersect(segments[event.segment_index], segments[next->index], intersection)) {
          auto pair = std::minmax(event.segment_index, next->index);
          if (found_intersections.find(pair) == found_intersections.end()) {
            found_intersections.insert(pair);
            intersections.emplace_back(intersection, event.segment_index, next->index);
            if (intersection.x > current_x + 1e-10) {
              event_queue.emplace(intersection, event.segment_index, next->index);
            }
          }
        }
      }
      
    } else if (event.type == EventType::RightEndpoint) {
      // Find segment to remove
      SegmentInfo target(event.segment_index, &segments, current_x);
      auto it = status.find(target);
      
      if (it != status.end()) {
        // Get predecessor and successor before removal (BBST operations)
        auto prev = it;
        auto next = it;
        ++next;
        
        if (it != status.begin()) {
          --prev;
        } else {
          prev = status.end();
        }
        
        // Remove segment from BBST
        status.erase(it);
        
        // Check if predecessor and successor now intersect
        if (prev != status.end() && next != status.end()) {
          Point2D intersection;
          if (SegmentsIntersect(segments[prev->index], segments[next->index], intersection)) {
            auto pair = std::minmax(prev->index, next->index);
            if (found_intersections.find(pair) == found_intersections.end()) {
              found_intersections.insert(pair);
              intersections.emplace_back(intersection, prev->index, next->index);
              if (intersection.x > current_x + 1e-10) {
                event_queue.emplace(intersection, prev->index, next->index);
              }
            }
          }
        }
      }
      
    } else {  // EventType::Intersection
      // Two segments intersect - swap their order in status
      int seg1 = event.segment_index;
      int seg2 = event.segment_index2;
      
      // Find both segments
      SegmentInfo info1(seg1, &segments, current_x);
      SegmentInfo info2(seg2, &segments, current_x);
      
      auto it1 = status.find(info1);
      auto it2 = status.find(info2);
      
      if (it1 != status.end() && it2 != status.end()) {
        // Get neighbors before removal (BBST operations)
        auto prev1 = it1, next1 = it1;
        auto prev2 = it2, next2 = it2;
        
        if (it1 != status.begin()) { --prev1; } else { prev1 = status.end(); }
        if (it2 != status.begin()) { --prev2; } else { prev2 = status.end(); }
        ++next1;
        ++next2;
        
        // Remove both segments from BBST
        status.erase(it1);
        status.erase(it2);
        
        // Re-insert (order will swap after intersection point)
        status.emplace(seg1, &segments, current_x);
        status.emplace(seg2, &segments, current_x);
        
        // Find new positions and check new neighbors
        auto new_it1 = status.find(SegmentInfo(seg1, &segments, current_x));
        auto new_it2 = status.find(SegmentInfo(seg2, &segments, current_x));
        
        // Check new neighbors of seg1
        if (new_it1 != status.begin()) {
          auto prev = new_it1;
          --prev;
          if (prev->index != seg2) {
            Point2D intersection;
            if (SegmentsIntersect(segments[seg1], segments[prev->index], intersection)) {
              auto pair = std::minmax(seg1, prev->index);
              if (found_intersections.find(pair) == found_intersections.end()) {
                found_intersections.insert(pair);
                intersections.emplace_back(intersection, seg1, prev->index);
                if (intersection.x > current_x + 1e-10) {
                  event_queue.emplace(intersection, seg1, prev->index);
                }
              }
            }
          }
        }
        
        auto next = new_it1;
        ++next;
        if (next != status.end() && next->index != seg2) {
          Point2D intersection;
          if (SegmentsIntersect(segments[seg1], segments[next->index], intersection)) {
            auto pair = std::minmax(seg1, next->index);
            if (found_intersections.find(pair) == found_intersections.end()) {
              found_intersections.insert(pair);
              intersections.emplace_back(intersection, seg1, next->index);
              if (intersection.x > current_x + 1e-10) {
                event_queue.emplace(intersection, seg1, next->index);
              }
            }
          }
        }
        
        // Check new neighbors of seg2
        if (new_it2 != status.begin()) {
          auto prev = new_it2;
          --prev;
          if (prev->index != seg1) {
            Point2D intersection;
            if (SegmentsIntersect(segments[seg2], segments[prev->index], intersection)) {
              auto pair = std::minmax(seg2, prev->index);
              if (found_intersections.find(pair) == found_intersections.end()) {
                found_intersections.insert(pair);
                intersections.emplace_back(intersection, seg2, prev->index);
                if (intersection.x > current_x + 1e-10) {
                  event_queue.emplace(intersection, seg2, prev->index);
                }
              }
            }
          }
        }
        
        next = new_it2;
        ++next;
        if (next != status.end() && next->index != seg1) {
          Point2D intersection;
          if (SegmentsIntersect(segments[seg2], segments[next->index], intersection)) {
            auto pair = std::minmax(seg2, next->index);
            if (found_intersections.find(pair) == found_intersections.end()) {
              found_intersections.insert(pair);
              intersections.emplace_back(intersection, seg2, next->index);
              if (intersection.x > current_x + 1e-10) {
                event_queue.emplace(intersection, seg2, next->index);
              }
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
  // Calculate intersection using line-line intersection
  Vector2D v1 = seg1.Direction();
  Vector2D v2 = seg2.Direction();
  Vector2D v3 = seg2.p1 - seg1.p1;
  
  double cross = v1.Cross(v2);
  if (std::abs(cross) < 1e-10) {
    return false;  // Parallel
  }
  
  double t = v3.Cross(v2) / cross;
  double s = v3.Cross(v1) / cross;
  
  // Check if intersection is within both segments
  const double eps = 1e-9;
  if (t >= -eps && t <= 1.0 + eps && s >= -eps && s <= 1.0 + eps) {
    Vector2D result = seg1.p1 - Point2D(0, 0) + v1 * t;
    intersection = Point2D(result.x, result.y);
    return true;
  }
  
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
