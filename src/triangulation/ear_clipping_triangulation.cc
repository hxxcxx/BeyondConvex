#include "ear_clipping_triangulation.h"
#include "triangulation_utils.h"
#include <iostream>
#include <algorithm>

namespace geometry {

TriangulationResult EarClippingTriangulation::Triangulate(
    const std::vector<Point2D>& polygon) {
  
  TriangulationResult result;
  
  if (polygon.size() < 3) {
    std::cerr << "[EarClipping] Invalid polygon: less than 3 vertices" << std::endl;
    return result;
  }
  
  std::cout << "[EarClipping] Triangulating polygon with " 
            << polygon.size() << " vertices" << std::endl;
  
  // Make a copy of the polygon (we'll modify it)
  std::vector<Point2D> poly = polygon;
  
  // Ensure CCW order
  if (!triangulation_utils::TriangulationUtils::IsCCW(poly)) {
    std::reverse(poly.begin(), poly.end());
  }
  
  // Remove collinear vertices
  triangulation_utils::TriangulationUtils::RemoveCollinear(poly);
  
  if (poly.size() < 3) {
    std::cerr << "[EarClipping] Polygon degenerate after removing collinear vertices" << std::endl;
    return result;
  }
  
  // Triangulate by ear clipping
  while (poly.size() > 3) {
    int ear_index = FindEar(poly);
    
    if (ear_index == -1) {
      std::cerr << "[EarClipping] No ear found, polygon may be self-intersecting" << std::endl;
      return result;
    }
    
    size_t n = poly.size();
    size_t prev = (ear_index + n - 1) % n;
    size_t next = (ear_index + 1) % n;
    
    // Add triangle
    result.triangles.emplace_back(poly[prev], poly[ear_index], poly[next]);
    
    // Remove ear vertex
    poly.erase(poly.begin() + ear_index);
  }
  
  // Add final triangle
  if (poly.size() == 3) {
    result.triangles.emplace_back(poly[0], poly[1], poly[2]);
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
  
  std::cout << "[EarClipping] Generated " << result.triangles.size() 
            << " triangles" << std::endl;
  
  return result;
}

bool EarClippingTriangulation::IsEar(const std::vector<Point2D>& polygon, size_t i) {
  size_t n = polygon.size();
  size_t prev = (i + n - 1) % n;
  size_t next = (i + 1) % n;
  
  // Check if vertex is convex
  if (!triangulation_utils::TriangulationUtils::IsConvex(
      polygon[prev], polygon[i], polygon[next])) {
    return false;
  }
  
  // Check if any other vertex is inside the ear triangle
  for (size_t j = 0; j < n; ++j) {
    if (j == prev || j == i || j == next) continue;
    
    if (triangulation_utils::TriangulationUtils::PointInTriangle(
        polygon[j], polygon[prev], polygon[i], polygon[next])) {
      return false;
    }
  }
  
  return true;
}

int EarClippingTriangulation::FindEar(const std::vector<Point2D>& polygon) {
  for (size_t i = 0; i < polygon.size(); ++i) {
    if (IsEar(polygon, i)) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

}  // namespace geometry
