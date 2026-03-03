#ifndef GEOMETRY_CORE_CONVEX_HULL_BUILDER_H_
#define GEOMETRY_CORE_CONVEX_HULL_BUILDER_H_

#include "convex_hull/convex_hull.h"
#include "../core/point2d.h"
#include <vector>

namespace geometry {

// Builder class that contains implementations of various convex hull algorithms
// This class is responsible for the actual construction of convex hulls
class ConvexHullBuilder {
 public:
  // Jarvis March (Gift Wrapping) algorithm
  // Time complexity: O(nh) where n is number of points, h is number of hull points
  // Returns: ConvexHull object with vertices in counter-clockwise order
  static ConvexHull BuildJarvisMarch(const std::vector<Point2D>& points);

  // Graham Scan algorithm
  // Time complexity: O(n log n) due to sorting
  // Returns: ConvexHull object with vertices in counter-clockwise order
  static ConvexHull BuildGrahamScan(const std::vector<Point2D>& points);

  // Future algorithms can be added here
  // static ConvexHull BuildQuickHull(const std::vector<Point2D>& points);
  // static ConvexHull BuildMonotoneChain(const std::vector<Point2D>& points);

 private:
  // Helper: Find the leftmost point (lowest x, then lowest y if tie)
  static int FindLeftmostPoint(const std::vector<Point2D>& points);

  // Helper: Find the most counter-clockwise point from the given point
  static int FindMostCounterClockwisePoint(const std::vector<Point2D>& points, int current_index);

  // Helper: Check if candidate point is more counter-clockwise than current best
  static bool IsMoreCounterClockwise(const Point2D& origin, 
                                     const Point2D& candidate, 
                                     const Point2D& current_best);

  // Graham Scan helpers
  static int FindLowestPoint(const std::vector<Point2D>& points);
  
  // Compare two points by polar angle with respect to origin
  // Returns true if a comes before b in counter-clockwise order
  static bool CompareByPolarAngle(const Point2D& origin, const Point2D& a, const Point2D& b);
};

}  // namespace geometry

#endif  // GEOMETRY_CORE_CONVEX_HULL_BUILDER_H_
