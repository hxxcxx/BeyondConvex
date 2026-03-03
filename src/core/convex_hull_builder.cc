#include "convex_hull_builder.h"
#include "geometry_utils.h"
#include <cmath>

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
    // Prefer smaller x, or smaller y if x values are equal
    if (points[i].x < points[leftmost_index].x ||
        (points[i].x == points[leftmost_index].x && points[i].y < points[leftmost_index].y)) {
      leftmost_index = static_cast<int>(i);
    }
  }
  
  return leftmost_index;
}

int ConvexHullBuilder::FindMostCounterClockwisePoint(const std::vector<Point2D>& points, int current_index) {
  int most_ccw_index = (current_index + 1) % static_cast<int>(points.size());
  const Point2D& current_point = points[current_index];
  
  // Check all other points to find the most counter-clockwise one
  for (size_t i = 0; i < points.size(); ++i) {
    if (static_cast<int>(i) == current_index) continue;
    
    const Point2D& candidate_point = points[i];
    const Point2D& current_best_point = points[most_ccw_index];
    
    // Check if candidate is more counter-clockwise than current best
    if (IsMoreCounterClockwise(current_point, candidate_point, current_best_point)) {
      most_ccw_index = static_cast<int>(i);
    }
  }
  
  return most_ccw_index;
}

bool ConvexHullBuilder::IsMoreCounterClockwise(const Point2D& origin, 
                                               const Point2D& candidate, 
                                               const Point2D& current_best) {
  // Check if candidate is to the left of origin->current_best
  if (GeometryUtils::ToLeftTest(origin, candidate, current_best)) {
    return true;
  }
  
  // Check if points are collinear (neither is to the left of the other)
  bool candidate_is_left = GeometryUtils::ToLeftTest(origin, candidate, current_best);
  bool current_best_is_left = GeometryUtils::ToLeftTest(origin, current_best, candidate);
  
  if (!candidate_is_left && !current_best_is_left) {
    // Collinear: choose the farther point
    double dist_to_candidate = (candidate - origin).LengthSquared();
    double dist_to_best = (current_best - origin).LengthSquared();
    return dist_to_candidate > dist_to_best;
  }
  
  return false;
}

}  // namespace geometry
