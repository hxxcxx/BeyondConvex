#include "convex_hull/convex_hull_builder.h"
#include "../core/geometry_core.h"
#include <cmath>
#include <algorithm>

namespace geometry {

ConvexHull ConvexHullBuilder::BuildJarvisMarch(const std::vector<Point2D>& points) {
  std::vector<Point2D> hull_vertices;
  
  if (points.size() < 3) {
    return ConvexHull(hull_vertices);
  }
  
  // Step 1: Find the starting point (leftmost point, lowest y if tie)
  int start_index = FindLeftmostPoint(points);
  
  // Step 2: Build the hull by wrapping around
  int current = start_index;
  
  do {
    // Add current vertex to hull
    hull_vertices.push_back(points[current]);
    
    // Find the next vertex (most counter-clockwise from current)
    int next = FindMostCounterClockwisePoint(points, current);
    
    // Move to next vertex
    current = next;
    
  } while (current != start_index);
  
  return ConvexHull(hull_vertices);
}

int ConvexHullBuilder::FindLeftmostPoint(const std::vector<Point2D>& points) {
  int leftmost_index = 0;
  
  for (size_t i = 1; i < points.size(); ++i) {
    if (points[i].x < points[leftmost_index].x ||
        (points[i].x == points[leftmost_index].x && 
         points[i].y < points[leftmost_index].y)) {
      leftmost_index = static_cast<int>(i);
    }
  }
  
  return leftmost_index;
}

int ConvexHullBuilder::FindMostCounterClockwisePoint(const std::vector<Point2D>& points, int current_index) {
  int most_ccw_index = (current_index + 1) % static_cast<int>(points.size());
  const Point2D& current_point = points[current_index];
  
  for (size_t i = 0; i < points.size(); ++i) {
    if (static_cast<int>(i) == current_index) continue;
    
    const Point2D& candidate_point = points[i];
    const Point2D& current_best_point = points[most_ccw_index];
    
    if (IsMoreCounterClockwise(current_point, candidate_point, current_best_point)) {
      most_ccw_index = static_cast<int>(i);
    }
  }
  
  return most_ccw_index;
}

bool ConvexHullBuilder::IsMoreCounterClockwise(const Point2D& origin, 
                                               const Point2D& candidate, 
                                               const Point2D& current_best) {
  if (internal::GeometryCore::ToLeftTest(origin, candidate, current_best)) {
    return true;
  }
  
  bool candidate_is_left = internal::GeometryCore::ToLeftTest(origin, candidate, current_best);
  bool current_best_is_left = internal::GeometryCore::ToLeftTest(origin, current_best, candidate);
  
  if (!candidate_is_left && !current_best_is_left) {
    double dist_to_candidate = (candidate - origin).LengthSquared();
    double dist_to_best = (current_best - origin).LengthSquared();
    return dist_to_candidate > dist_to_best;
  }
  
  return false;
}

ConvexHull ConvexHullBuilder::BuildGrahamScan(const std::vector<Point2D>& points) {
  std::vector<Point2D> hull_vertices;
  
  if (points.size() < 3) {
    return ConvexHull(hull_vertices);
  }
  
  // Step 1: Find the lowest point (lowest y, then lowest x if tie)
  int pivot_index = FindLowestPoint(points);
  Point2D pivot = points[pivot_index];
  
  // Step 2: Create a copy of points without the pivot
  std::vector<Point2D> sorted_points;
  for (size_t i = 0; i < points.size(); ++i) {
    if (static_cast<int>(i) != pivot_index) {
      sorted_points.push_back(points[i]);
    }
  }
  
  // Step 3: Sort points by polar angle with respect to pivot
  std::sort(sorted_points.begin(), sorted_points.end(),
    [&pivot](const Point2D& a, const Point2D& b) {
      return CompareByPolarAngle(pivot, a, b);
    });
  
  // Step 3.5: Remove collinear points with pivot (keep only the farthest)
  // After sorting, collinear points are grouped together with closer points first
  // We only need to keep the farthest point in each collinear group
  if (!sorted_points.empty()) {
    size_t last_unique = 0;  // Index of last point with unique angle
    for (size_t i = 1; i < sorted_points.size(); ++i) {
      Vector2D v1 = sorted_points[last_unique] - pivot;
      Vector2D v2 = sorted_points[i] - pivot;
      double cross = v1.Cross(v2);
      
      if (std::abs(cross) < 1e-10) {
        // Collinear with pivot, keep the farther one
        double dist1 = v1.LengthSquared();
        double dist2 = v2.LengthSquared();
        if (dist2 > dist1) {
          sorted_points[last_unique] = sorted_points[i];  // Replace with farther point
        }
        // If current point is closer, skip it (don't update last_unique)
      } else {
        // Different angle, keep this point
        ++last_unique;
        if (last_unique != i) {
          sorted_points[last_unique] = sorted_points[i];
        }
      }
    }
    sorted_points.resize(last_unique + 1);
  }
  
  // Step 4: Build the hull using a stack
  std::vector<Point2D> stack;
  stack.push_back(pivot);
  
  if (!sorted_points.empty()) {
    stack.push_back(sorted_points[0]);
  }
  
  for (size_t i = 1; i < sorted_points.size(); ++i) {
    while (stack.size() >= 2) {
      size_t n = stack.size();
      const Point2D& top = stack[n - 1];
      const Point2D& second_top = stack[n - 2];
      
      // Check if we need to remove top (not making a left turn)
      if (!internal::GeometryCore::ToLeftTest(second_top, top, sorted_points[i])) {
        stack.pop_back();
      } else {
        break;
      }
    }
    stack.push_back(sorted_points[i]);
  }
  
  return ConvexHull(stack);
}

int ConvexHullBuilder::FindLowestPoint(const std::vector<Point2D>& points) {
  int lowest_index = 0;
  
  for (size_t i = 1; i < points.size(); ++i) {
    if (points[i].y < points[lowest_index].y ||
        (points[i].y == points[lowest_index].y && 
         points[i].x < points[lowest_index].x)) {
      lowest_index = static_cast<int>(i);
    }
  }
  
  return lowest_index;
}

bool ConvexHullBuilder::CompareByPolarAngle(const Point2D& origin, const Point2D& a, const Point2D& b) {
  Vector2D oa = a - origin;
  Vector2D ob = b - origin;
  
  // Determine half-plane (upper: y > 0, lower: y <= 0)
  bool a_in_upper = (oa.y > 0);
  bool b_in_upper = (ob.y > 0);
  
  if (a_in_upper != b_in_upper) {
    // Points in different half-planes
    // Upper half-plane comes first (angles from 0 to π)
    return a_in_upper;
  }
  
  // Points in same half-plane, use cross product
  double cross = oa.Cross(ob);
  
  if (std::abs(cross) > 1e-10) {
    // Not collinear, use cross product
    // In upper half-plane: cross > 0 means a has smaller angle
    // In lower half-plane: cross > 0 means a has smaller angle (due to symmetry)
    return cross > 0;
  }
  
  // Collinear, closer point comes first
  double dist_a = oa.LengthSquared();
  double dist_b = ob.LengthSquared();
  return dist_a < dist_b;
}

}  // namespace geometry
