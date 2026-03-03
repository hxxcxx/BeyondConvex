#ifndef GEOMETRY_CORE_CONVEX_HULL_H_
#define GEOMETRY_CORE_CONVEX_HULL_H_

#include "point2d.h"
#include "edge2d.h"
#include <vector>

namespace geometry {

// ConvexHull class representing a convex polygon
// Stores vertices in counter-clockwise (CCW) order
class ConvexHull {
 public:
  // Default constructor
  ConvexHull() = default;
  
  // Construct from a vector of points (assumed to be in CCW order)
  explicit ConvexHull(const std::vector<Point2D>& points);
  
  // Get the vertices in CCW order
  const std::vector<Point2D>& GetVertices() const { return vertices_; }
  
  // Get number of vertices
  size_t Size() const { return vertices_.size(); }
  
  // Check if hull is empty
  bool IsEmpty() const { return vertices_.empty(); }
  
  // Check if a point is inside the convex hull
  // Returns: true if point is inside or on the hull
  bool Contains(const Point2D& point) const;
  
  // Check if a point is inside using tangent method
  // For a point inside, for any vertex v, its predecessor and successor
  // will always be on different sides of the ray from the point to v
  // Returns: true if point is inside or on the hull
  bool ContainsByTangent(const Point2D& point) const;
  
  // Get edges of the convex hull
  std::vector<Edge2D> GetEdges() const;
  
  // Calculate area of the convex hull
  double Area() const;
  
  // Calculate perimeter of the convex hull
  double Perimeter() const;
  
  // Get vertex at index (with wraparound)
  const Point2D& GetVertex(size_t index) const;
  
  // Get predecessor (next vertex in CCW direction)
  const Point2D& GetPredecessor(size_t index) const;
  
  // Get successor (next vertex in CW direction)
  const Point2D& GetSuccessor(size_t index) const;
  
 private:
  std::vector<Point2D> vertices_;  // Vertices in CCW order
};

}  // namespace geometry

#endif  // GEOMETRY_CORE_CONVEX_HULL_H_
