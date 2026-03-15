#include "delaunay_triangulation.h"
#include "triangulation_utils.h"
#include <iostream>
#include <algorithm>
#include <set>

namespace geometry {

TriangulationResult DelaunayTriangulation::Triangulate(
    const std::vector<Point2D>& points) {
  
  TriangulationResult result;
  
  if (points.size() < 3) {
    std::cerr << "[Delaunay] Invalid input: less than 3 points" << std::endl;
    return result;
  }
  
  std::cout << "[Delaunay] Triangulating " << points.size() << " points" << std::endl;
  
  // For now, use a simple incremental approach
  // TODO: Implement proper Delaunay algorithm (e.g., Bowyer-Watson)
  std::cerr << "[Delaunay] Note: Using simple triangulation (not true Delaunay yet)" << std::endl;
  
  // Sort points by x-coordinate
  std::vector<Point2D> sorted_points = points;
  std::sort(sorted_points.begin(), sorted_points.end(),
      [](const Point2D& a, const Point2D& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
      });
  
  // Simple triangulation: connect consecutive points
  for (size_t i = 0; i + 2 < sorted_points.size(); ++i) {
    result.triangles.emplace_back(
        sorted_points[i], sorted_points[i + 1], sorted_points[i + 2]);
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
  
  std::cout << "[Delaunay] Generated " << result.triangles.size() 
            << " triangles" << std::endl;
  
  return result;
}

bool DelaunayTriangulation::IsDelaunay(
    const Triangle& tri,
    const std::vector<Point2D>& points) {
  
  Point2D center = tri.Circumcenter();
  double radius = tri.Circumradius();
  
  // Check if any other point is inside the circumcircle
  for (const auto& p : points) {
    if (p == tri.v0 || p == tri.v1 || p == tri.v2) continue;
    
    double dist = center.DistanceTo(p);
    if (dist < radius - 1e-9) {
      return false;  // Point inside circumcircle
    }
  }
  
  return true;
}

void DelaunayTriangulation::FlipEdge(
    Edge2D edge,
    std::vector<Triangle>& triangles) {
  // TODO: Implement edge flipping for Delaunay refinement
  // This is a placeholder for future implementation
}

}  // namespace geometry
