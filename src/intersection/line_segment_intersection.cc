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
  
  // Current sweep line x position (needed for ordering)
  double sweep_x = 0.0;
  
  // Comparator for sweep line (orders segments by y at current sweep_x)
  auto segment_comparator = [&sweep_x, &segments](int a, int b) {
    double y_a = GetYAtX(segments[a], sweep_x);
    double y_b = GetYAtX(segments[b], sweep_x);
    if (std::abs(y_a - y_b) > 1e-10) {
      return y_a < y_b;
    }
    // If y coordinates are equal, compare by slope
    const Edge2D& seg1 = segments[a];
    const Edge2D& seg2 = segments[b];
    double slope1 = (seg1.p2.y - seg1.p1.y) / (seg1.p2.x - seg1.p1.x);
    double slope2 = (seg2.p2.y - seg2.p1.y) / (seg2.p2.x - seg2.p1.x);
    return slope1 < slope2;
  };
  
  // Sweep line: set of segment indices ordered by y at current x
  std::set<int, decltype(segment_comparator)> sweep_line(segment_comparator);
  
  // Initialize event queue with all segment endpoints
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
  
  // Track found intersections to avoid duplicates
  std::set<std::pair<int, int>> found_intersections;
  
  // Process events
  while (!event_queue.empty()) {
    EventPoint event = event_queue.top();
    event_queue.pop();
    
    sweep_x = event.point.x;
    
    if (event.type == EventType::LeftEndpoint) {
      // Insert segment into sweep line
      int seg_idx = event.segment_index;
      auto it = sweep_line.insert(seg_idx).first;
      
      // Check predecessor
      if (it != sweep_line.begin()) {
        auto prev = it;
        --prev;
        Point2D intersection;
        if (SegmentsIntersect(segments[seg_idx], segments[*prev], intersection)) {
          auto pair = std::minmax(seg_idx, *prev);
          if (found_intersections.find(pair) == found_intersections.end()) {
            found_intersections.insert(pair);
            intersections.emplace_back(intersection, seg_idx, *prev);
            // Add intersection event if it's to the right
            if (intersection.x > sweep_x + 1e-10) {
              event_queue.emplace(intersection, seg_idx, *prev);
            }
          }
        }
      }
      
      // Check successor
      auto next = it;
      ++next;
      if (next != sweep_line.end()) {
        Point2D intersection;
        if (SegmentsIntersect(segments[seg_idx], segments[*next], intersection)) {
          auto pair = std::minmax(seg_idx, *next);
          if (found_intersections.find(pair) == found_intersections.end()) {
            found_intersections.insert(pair);
            intersections.emplace_back(intersection, seg_idx, *next);
            if (intersection.x > sweep_x + 1e-10) {
              event_queue.emplace(intersection, seg_idx, *next);
            }
          }
        }
      }
      
    } else if (event.type == EventType::RightEndpoint) {
      // Remove segment from sweep line
      int seg_idx = event.segment_index;
      auto it = sweep_line.find(seg_idx);
      
      if (it != sweep_line.end()) {
        auto prev = it;
        auto next = it;
        ++next;
        
        if (it != sweep_line.begin()) {
          --prev;
        } else {
          prev = sweep_line.end();
        }
        
        sweep_line.erase(it);
        
        // Check if neighbors now intersect
        if (prev != sweep_line.end() && next != sweep_line.end()) {
          Point2D intersection;
          if (SegmentsIntersect(segments[*prev], segments[*next], intersection)) {
            auto pair = std::minmax(*prev, *next);
            if (found_intersections.find(pair) == found_intersections.end()) {
              found_intersections.insert(pair);
              intersections.emplace_back(intersection, *prev, *next);
              if (intersection.x > sweep_x + 1e-10) {
                event_queue.emplace(intersection, *prev, *next);
              }
            }
          }
        }
      }
      
    } else {  // EventType::Intersection
      // Two segments intersect - they will swap order in sweep line
      int seg1_idx = event.segment_index;
      int seg2_idx = event.segment_index2;
      
      // Find and remove both segments
      auto it1 = sweep_line.find(seg1_idx);
      auto it2 = sweep_line.find(seg2_idx);
      
      if (it1 != sweep_line.end() && it2 != sweep_line.end()) {
        // Get neighbors before removing
        auto prev1 = it1, next1 = it1;
        auto prev2 = it2, next2 = it2;
        
        if (it1 != sweep_line.begin()) { --prev1; } else { prev1 = sweep_line.end(); }
        if (it2 != sweep_line.begin()) { --prev2; } else { prev2 = sweep_line.end(); }
        ++next1;
        ++next2;
        
        // Remove and re-insert (order will swap due to intersection)
        sweep_line.erase(it1);
        sweep_line.erase(it2);
        sweep_line.insert(seg1_idx);
        sweep_line.insert(seg2_idx);
        
        // Check new neighbors of seg1
        auto new_it1 = sweep_line.find(seg1_idx);
        if (new_it1 != sweep_line.begin()) {
          auto prev = new_it1;
          --prev;
          if (*prev != seg2_idx) {
            Point2D intersection;
            if (SegmentsIntersect(segments[seg1_idx], segments[*prev], intersection)) {
              auto pair = std::minmax(seg1_idx, *prev);
              if (found_intersections.find(pair) == found_intersections.end()) {
                found_intersections.insert(pair);
                intersections.emplace_back(intersection, seg1_idx, *prev);
                if (intersection.x > sweep_x + 1e-10) {
                  event_queue.emplace(intersection, seg1_idx, *prev);
                }
              }
            }
          }
        }
        
        auto next = new_it1;
        ++next;
        if (next != sweep_line.end() && *next != seg2_idx) {
          Point2D intersection;
          if (SegmentsIntersect(segments[seg1_idx], segments[*next], intersection)) {
            auto pair = std::minmax(seg1_idx, *next);
            if (found_intersections.find(pair) == found_intersections.end()) {
              found_intersections.insert(pair);
              intersections.emplace_back(intersection, seg1_idx, *next);
              if (intersection.x > sweep_x + 1e-10) {
                event_queue.emplace(intersection, seg1_idx, *next);
              }
            }
          }
        }
        
        // Check new neighbors of seg2
        auto new_it2 = sweep_line.find(seg2_idx);
        if (new_it2 != sweep_line.begin()) {
          auto prev = new_it2;
          --prev;
          if (*prev != seg1_idx) {
            Point2D intersection;
            if (SegmentsIntersect(segments[seg2_idx], segments[*prev], intersection)) {
              auto pair = std::minmax(seg2_idx, *prev);
              if (found_intersections.find(pair) == found_intersections.end()) {
                found_intersections.insert(pair);
                intersections.emplace_back(intersection, seg2_idx, *prev);
                if (intersection.x > sweep_x + 1e-10) {
                  event_queue.emplace(intersection, seg2_idx, *prev);
                }
              }
            }
          }
        }
        
        next = new_it2;
        ++next;
        if (next != sweep_line.end() && *next != seg1_idx) {
          Point2D intersection;
          if (SegmentsIntersect(segments[seg2_idx], segments[*next], intersection)) {
            auto pair = std::minmax(seg2_idx, *next);
            if (found_intersections.find(pair) == found_intersections.end()) {
              found_intersections.insert(pair);
              intersections.emplace_back(intersection, seg2_idx, *next);
              if (intersection.x > sweep_x + 1e-10) {
                event_queue.emplace(intersection, seg2_idx, *next);
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
