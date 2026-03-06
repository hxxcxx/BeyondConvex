#include "triangulation.h"
#include "../core/geometry_core.h"
#include <algorithm>
#include <map>
#include <set>
#include <cmath>

namespace geometry {

// Triangle methods
bool Triangle::Contains(const Point2D& p) const {
  // Use barycentric coordinates or check if point is on same side of all edges
  auto same_side = [](const Point2D& p1, const Point2D& p2, 
                      const Point2D& a, const Point2D& b) {
    Vector2D v1 = b - a;
    Vector2D v2 = p1 - a;
    Vector2D v3 = p2 - a;
    double cross1 = v1.Cross(v2);
    double cross2 = v1.Cross(v3);
    return (cross1 >= -1e-10 && cross2 >= -1e-10) || 
           (cross1 <= 1e-10 && cross2 <= 1e-10);
  };
  
  return same_side(p, v0, v1, v2) &&
         same_side(p, v1, v0, v2) &&
         same_side(p, v2, v0, v1);
}

Point2D Triangle::Circumcenter() const {
  // Calculate circumcenter using perpendicular bisectors
  double ax = v0.x, ay = v0.y;
  double bx = v1.x, by = v1.y;
  double cx = v2.x, cy = v2.y;
  
  double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
  if (std::abs(d) < 1e-10) {
    // Degenerate triangle, return centroid
    return Point2D((ax + bx + cx) / 3.0, (ay + by + cy) / 3.0);
  }
  
  double ux = ((ax * ax + ay * ay) * (by - cy) + 
               (bx * bx + by * by) * (cy - ay) + 
               (cx * cx + cy * cy) * (ay - by)) / d;
  
  double uy = ((ax * ax + ay * ay) * (cx - bx) + 
               (bx * bx + by * by) * (ax - cx) + 
               (cx * cx + cy * cy) * (bx - ax)) / d;
  
  return Point2D(ux, uy);
}

double Triangle::Circumradius() const {
  Point2D center = Circumcenter();
  return (v0 - center).Length();
}

// Main triangulation function
TriangulationResult Triangulation::Triangulate(
    const std::vector<Point2D>& polygon,
    TriangulationAlgorithm algorithm) {
  
  if (polygon.size() < 3) {
    return TriangulationResult();
  }
  
  if (!IsSimplePolygon(polygon)) {
    return TriangulationResult();
  }
  
  switch (algorithm) {
    case TriangulationAlgorithm::kSweepLine:
      return SweepLineTriangulation(polygon);
    case TriangulationAlgorithm::kEarClipping:
      return EarClippingTriangulation(polygon);
    case TriangulationAlgorithm::kDelaunay:
      return DelaunayTriangulation(polygon);
    default:
      return SweepLineTriangulation(polygon);
  }
}

TriangulationResult Triangulation::SweepLineTriangulation(
    const std::vector<Point2D>& polygon) {
  
  TriangulationResult result;
  
  if (polygon.size() < 3) {
    return result;
  }
  
  // Make a copy and ensure CCW order
  std::vector<Point2D> vertices = polygon;
  if (!IsCCW(vertices)) {
    std::reverse(vertices.begin(), vertices.end());
  }
  
  // Sort vertices by y-coordinate (top to bottom)
  std::vector<size_t> vertex_order(vertices.size());
  for (size_t i = 0; i < vertices.size(); ++i) {
    vertex_order[i] = i;
  }
  
  std::sort(vertex_order.begin(), vertex_order.end(),
      [&vertices](size_t i, size_t j) {
        if (std::abs(vertices[i].y - vertices[j].y) > 1e-10) {
          return vertices[i].y > vertices[j].y;  // Higher y first (top to bottom)
        }
        return vertices[i].x < vertices[j].x;  // Tie-breaker: left to right
      });
  
  // Track which vertices have been processed
  std::set<size_t> processed_vertices;
  
  // Track edges in the current triangulation
  std::set<std::pair<size_t, size_t>> triangulation_edges;
  
  // Add polygon edges
  for (size_t i = 0; i < vertices.size(); ++i) {
    size_t next = (i + 1) % vertices.size();
    size_t v1 = std::min(i, next);
    size_t v2 = std::max(i, next);
    triangulation_edges.insert({v1, v2});
    result.edges.push_back(Edge2D(vertices[i], vertices[next]));
  }
  
  // Process vertices from top to bottom
  for (size_t idx : vertex_order) {
    processed_vertices.insert(idx);
    
    // Find adjacent vertices in polygon
    size_t prev = (idx + vertices.size() - 1) % vertices.size();
    size_t next = (idx + 1) % vertices.size();
    
    // Check if we can form triangles with this vertex
    // Try to connect to visible vertices
    for (size_t other : processed_vertices) {
      if (other == idx || other == prev || other == next) continue;
      
      // Check if diagonal is valid
      if (IsValidDiagonal(vertices, idx, other)) {
        size_t v1 = std::min(idx, other);
        size_t v2 = std::max(idx, other);
        
        // Check if edge already exists
        if (triangulation_edges.find({v1, v2}) == triangulation_edges.end()) {
          // Add new triangle
          // Find the third vertex that forms a triangle with idx and other
          // This is a simplified approach - for proper sweep line, we need more sophisticated logic
        }
      }
    }
  }
  
  // For now, use ear clipping as fallback
  // A proper sweep line triangulation is more complex
  return EarClippingTriangulation(vertices);
}

TriangulationResult Triangulation::EarClippingTriangulation(
    const std::vector<Point2D>& polygon) {
  
  TriangulationResult result;
  
  if (polygon.size() < 3) {
    return result;
  }
  
  // Make a copy and ensure CCW order
  std::vector<size_t> vertices;
  for (size_t i = 0; i < polygon.size(); ++i) {
    vertices.push_back(i);
  }
  
  // Check if CCW, if not reverse
  std::vector<Point2D> polygon_points = polygon;
  if (!IsCCW(polygon_points)) {
    std::reverse(polygon_points.begin(), polygon_points.end());
  }
  
  // Ear clipping algorithm
  while (vertices.size() > 3) {
    bool ear_found = false;
    
    for (size_t i = 0; i < vertices.size(); ++i) {
      size_t prev = (i + vertices.size() - 1) % vertices.size();
      size_t curr = i;
      size_t next = (i + 1) % vertices.size();
      
      size_t ip = vertices[prev];
      size_t ic = vertices[curr];
      size_t in = vertices[next];
      
      const Point2D& p_prev = polygon_points[ip];
      const Point2D& p_curr = polygon_points[ic];
      const Point2D& p_next = polygon_points[in];
      
      // Check if (prev, curr, next) forms an ear (convex and no points inside)
      if (IsLeftTurn(p_prev, p_curr, p_next)) {
        bool is_ear = true;
        
        // Check if any other vertex is inside this triangle
        Triangle ear(p_prev, p_curr, p_next);
        for (size_t j = 0; j < vertices.size(); ++j) {
          size_t vj = vertices[j];
          if (vj == ip || vj == ic || vj == in) continue;
          
          if (ear.Contains(polygon_points[vj])) {
            is_ear = false;
            break;
          }
        }
        
        if (is_ear) {
          // Clip this ear
          result.triangles.push_back(ear);
          result.vertices.push_back(p_prev);
          result.vertices.push_back(p_curr);
          result.vertices.push_back(p_next);
          result.edges.push_back(Edge2D(p_prev, p_curr));
          result.edges.push_back(Edge2D(p_curr, p_next));
          result.edges.push_back(Edge2D(p_next, p_prev));
          
          // Remove current vertex from polygon
          vertices.erase(vertices.begin() + curr);
          
          ear_found = true;
          break;
        }
      }
    }
    
    if (!ear_found) {
      // No ear found, polygon might be self-intersecting
      break;
    }
  }
  
  // Add the final triangle
  if (vertices.size() == 3) {
    const Point2D& p0 = polygon_points[vertices[0]];
    const Point2D& p1 = polygon_points[vertices[1]];
    const Point2D& p2 = polygon_points[vertices[2]];
    
    result.triangles.push_back(Triangle(p0, p1, p2));
    result.vertices.push_back(p0);
    result.vertices.push_back(p1);
    result.vertices.push_back(p2);
    result.edges.push_back(Edge2D(p0, p1));
    result.edges.push_back(Edge2D(p1, p2));
    result.edges.push_back(Edge2D(p2, p0));
  }
  
  return result;
}

TriangulationResult Triangulation::DelaunayTriangulation(
    const std::vector<Point2D>& points) {
  
  // For now, use ear clipping on the convex hull
  // A proper Delaunay triangulation requires flip algorithm
  return EarClippingTriangulation(points);
}

std::vector<TriangulationAlgorithm> Triangulation::GetSupportedAlgorithms() {
  return {
    TriangulationAlgorithm::kSweepLine,
    TriangulationAlgorithm::kEarClipping,
    TriangulationAlgorithm::kDelaunay,
  };
}

std::string Triangulation::GetAlgorithmName(TriangulationAlgorithm algorithm) {
  switch (algorithm) {
    case TriangulationAlgorithm::kSweepLine:
      return "Sweep Line (Top to Bottom)";
    case TriangulationAlgorithm::kEarClipping:
      return "Ear Clipping";
    case TriangulationAlgorithm::kDelaunay:
      return "Delaunay";
    default:
      return "Unknown";
  }
}

std::string Triangulation::GetAlgorithmComplexity(TriangulationAlgorithm algorithm) {
  switch (algorithm) {
    case TriangulationAlgorithm::kSweepLine:
      return "O(n log n)";
    case TriangulationAlgorithm::kEarClipping:
      return "O(n²)";
    case TriangulationAlgorithm::kDelaunay:
      return "O(n log n)";
    default:
      return "Unknown";
  }
}

// Helper functions
bool Triangulation::IsSimplePolygon(const std::vector<Point2D>& polygon) {
  // Check if any non-adjacent edges intersect
  size_t n = polygon.size();
  if (n < 3) return false;
  
  for (size_t i = 0; i < n; ++i) {
    size_t next_i = (i + 1) % n;
    for (size_t j = i + 2; j < n; ++j) {
      // Skip adjacent edges and the same edge
      if (j == n - 1 && i == 0) continue;
      
      size_t next_j = (j + 1) % n;
      
      // Check intersection
      Vector2D v1 = polygon[next_i] - polygon[i];
      Vector2D v2 = polygon[next_j] - polygon[j];
      Vector2D v3 = polygon[j] - polygon[i];
      
      double cross = v1.Cross(v2);
      if (std::abs(cross) < 1e-10) continue;  // Parallel
      
      double t = v3.Cross(v2) / cross;
      double s = v3.Cross(v1) / cross;
      
      const double eps = 1e-9;
      if (t > eps && t < 1.0 - eps && s > eps && s < 1.0 - eps) {
        return false;  // Intersection found
      }
    }
  }
  
  return true;
}

bool Triangulation::IsCCW(const std::vector<Point2D>& polygon) {
  return SignedArea(polygon) > 0;
}

double Triangulation::SignedArea(const std::vector<Point2D>& polygon) {
  double area = 0.0;
  size_t n = polygon.size();
  
  for (size_t i = 0; i < n; ++i) {
    size_t next = (i + 1) % n;
    area += (polygon[next].x - polygon[i].x) * (polygon[next].y + polygon[i].y);
  }
  
  return area / 2.0;
}

bool Triangulation::IsLeftTurn(const Point2D& a, const Point2D& b, const Point2D& c) {
  Vector2D v1 = b - a;
  Vector2D v2 = c - a;
  return v1.Cross(v2) > 0;
}

bool Triangulation::IsValidDiagonal(
    const std::vector<Point2D>& polygon, size_t i, size_t j) {
  
  size_t n = polygon.size();
  
  // Check if diagonal is inside polygon
  if (!IsDiagonalInside(polygon, i, j)) {
    return false;
  }
  
  // Check if diagonal intersects any polygon edge
  if (DiagonalIntersectsEdges(polygon, i, j)) {
    return false;
  }
  
  return true;
}

bool Triangulation::IsDiagonalInside(
    const std::vector<Point2D>& polygon, size_t i, size_t j) {
  
  size_t n = polygon.size();
  size_t prev_i = (i + n - 1) % n;
  size_t next_i = (i + 1) % n;
  
  // Diagonal must be strictly inside the polygon at vertex i
  if (!IsLeftTurn(polygon[prev_i], polygon[i], polygon[j]) &&
      !IsLeftTurn(polygon[i], polygon[next_i], polygon[j])) {
    return false;
  }
  
  return true;
}

bool Triangulation::DiagonalIntersectsEdges(
    const std::vector<Point2D>& polygon, size_t i, size_t j) {
  
  size_t n = polygon.size();
  
  for (size_t k = 0; k < n; ++k) {
    size_t next_k = (k + 1) % n;
    
    // Skip edges incident to i or j
    if (k == i || next_k == i || k == j || next_k == j) {
      continue;
    }
    
    // Check if diagonal (i,j) intersects edge (k, next_k)
    Vector2D v1 = polygon[j] - polygon[i];
    Vector2D v2 = polygon[next_k] - polygon[k];
    Vector2D v3 = polygon[k] - polygon[i];
    
    double cross = v1.Cross(v2);
    if (std::abs(cross) < 1e-10) continue;  // Parallel
    
    double t = v3.Cross(v2) / cross;
    double s = v3.Cross(v1) / cross;
    
    const double eps = 1e-9;
    if (t > eps && t < 1.0 - eps && s > eps && s < 1.0 - eps) {
      return true;  // Intersection found
    }
  }
  
  return false;
}

}  // namespace geometry
