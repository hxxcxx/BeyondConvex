#include "convex_hull/convex_hull.h"
#include "../core/geometry_utils.h"
#include <cmath>

namespace geometry {

ConvexHull::ConvexHull(const std::vector<Point2D>& points) 
    : vertices_(points) {
}

const Point2D& ConvexHull::GetVertex(size_t index) const {
  return vertices_[index % vertices_.size()];
}

const Point2D& ConvexHull::GetPredecessor(size_t index) const {
  if (vertices_.empty()) return vertices_[0];
  return vertices_[(index + 1) % vertices_.size()];
}

const Point2D& ConvexHull::GetSuccessor(size_t index) const {
  if (vertices_.empty()) return vertices_[0];
  return vertices_[(index - 1 + vertices_.size()) % vertices_.size()];
}

bool ConvexHull::Contains(const Point2D& point) const {
  if (vertices_.size() < 3) return false;
  
  // Check if point is to the left of all hull edges (for CCW hull)
  for (size_t i = 0; i < vertices_.size(); ++i) {
    const Point2D& p1 = vertices_[i];
    const Point2D& p2 = vertices_[(i + 1) % vertices_.size()];
    
    if (!GeometryUtils::ToLeftTest(p1, p2, point)) {
      return false;
    }
  }
  
  return true;
}

bool ConvexHull::ContainsByTangent(const Point2D& point) const {
  if (vertices_.size() < 3) return false;
  
  size_t n = vertices_.size();
  
  // For each vertex v on the hull
  for (size_t i = 0; i < n; ++i) {
    const Point2D& v = vertices_[i];
    
    // Predecessor: next vertex in CCW direction
    const Point2D& pred = GetPredecessor(i);
    
    // Successor: next vertex in CW direction
    const Point2D& succ = GetSuccessor(i);
    
    // Check if predecessor and successor are on the same side of the ray
    bool pred_on_left = GeometryUtils::ToLeftTest(point, v, pred);
    bool succ_on_left = GeometryUtils::ToLeftTest(point, v, succ);
    
    // If both are on the same side, this is a tangent
    // This means the point is outside the hull
    if (pred_on_left == succ_on_left) {
      return false;
    }
  }
  
  // If for all vertices, predecessor and successor are on different sides,
  // then the point is inside the hull
  return true;
}

std::vector<Edge2D> ConvexHull::GetEdges() const {
  std::vector<Edge2D> edges;
  
  if (vertices_.size() < 2) return edges;
  
  for (size_t i = 0; i < vertices_.size(); ++i) {
    edges.push_back(Edge2D(vertices_[i], vertices_[(i + 1) % vertices_.size()]));
  }
  
  return edges;
}

double ConvexHull::Area() const {
  if (vertices_.size() < 3) return 0.0;
  
  double area = 0.0;
  size_t n = vertices_.size();
  
  for (size_t i = 0; i < n; ++i) {
    const Point2D& p1 = vertices_[i];
    const Point2D& p2 = vertices_[(i + 1) % n];
    area += (p1.x * p2.y - p2.x * p1.y);
  }
  
  return std::abs(area) / 2.0;
}

double ConvexHull::Perimeter() const {
  if (vertices_.size() < 2) return 0.0;
  
  double perimeter = 0.0;
  size_t n = vertices_.size();
  
  for (size_t i = 0; i < n; ++i) {
    perimeter += (vertices_[(i + 1) % n] - vertices_[i]).Length();
  }
  
  return perimeter;
}

}  // namespace geometry
