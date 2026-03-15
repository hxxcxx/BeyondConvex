#include "sweep_line_triangulation.h"
#include "triangulation_utils.h"
#include <iostream>
#include <algorithm>
#include <stack>

namespace geometry {

TriangulationResult SweepLineTriangulation::Triangulate(
    const std::vector<Point2D>& polygon) {
  
  TriangulationResult result;
  
  if (polygon.size() < 3) {
    std::cerr << "[SweepLine] Invalid polygon: less than 3 vertices" << std::endl;
    return result;
  }
  
  std::cout << "[SweepLine] Triangulating polygon with " 
            << polygon.size() << " vertices" << std::endl;
  
  // For now, delegate to ear clipping (sweep line requires monotone decomposition)
  // TODO: Implement proper sweep line algorithm with monotone decomposition
  std::cerr << "[SweepLine] Note: Currently delegating to ear clipping" << std::endl;
  
  EarClippingTriangulation ear_clipping;
  return ear_clipping.Triangulate(polygon);
}

TriangulationResult SweepLineTriangulation::TriangulateMonotonePolygon(
    const std::vector<Point2D>& polygon) {
  
  TriangulationResult result;
  
  if (polygon.size() < 3) {
    return result;
  }
  
  // Sort vertices by y-coordinate (top to bottom)
  std::vector<size_t> indices(polygon.size());
  for (size_t i = 0; i < polygon.size(); ++i) {
    indices[i] = i;
  }
  
  std::sort(indices.begin(), indices.end(),
      [&polygon](size_t i, size_t j) {
        return polygon[i].y > polygon[j].y;  // Higher y first
      });
  
  // Sweep from top to bottom
  std::stack<size_t> stack;
  
  for (size_t idx : indices) {
    if (stack.size() < 2) {
      stack.push(idx);
      continue;
    }
    
    size_t top = stack.top();
    stack.pop();
    size_t prev = stack.top();
    
    // Add triangle
    result.triangles.emplace_back(
        polygon[prev], polygon[top], polygon[idx]);
    
    stack.push(idx);
  }
  
  // Collect vertices and edges
  std::set<Point2D> vertex_set;
  std::set<Edge2D> edge_set;
  
  for (const auto& tri : result.triangles) {
    vertex_set.insert(tri.v0);
    vertex_set.insert(tri.v1);
    vertex_set.insert(tri.v2);
    
    // Normalize edge direction
    auto add_edge = [&edge_set](const Point2D& p1, const Point2D& p2) {
      Point2D a = p1, b = p2;
      if (b < a || (a == b && b < a)) std::swap(a, b);
      edge_set.emplace(a, b);
    };
    
    add_edge(tri.v0, tri.v1);
    add_edge(tri.v1, tri.v2);
    add_edge(tri.v2, tri.v0);
  }
  
  result.vertices.assign(vertex_set.begin(), vertex_set.end());
  result.edges.assign(edge_set.begin(), edge_set.end());
  
  return result;
}

}  // namespace geometry
