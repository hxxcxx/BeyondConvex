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
    const double eps = 1e-9;
    // Both points should be on the same side (including boundary)
    return (cross1 > -eps && cross2 > -eps) ||
           (cross1 < eps && cross2 < eps);
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
    const std::vector<Point2D>& polygon)
{
    TriangulationResult result;

    if (polygon.size() < 3)
        return result;

    std::vector<Point2D> vertices = polygon;

    if (!IsCCW(vertices))
        std::reverse(vertices.begin(), vertices.end());

    size_t n = vertices.size();

    // Step1: sort by y (top to bottom)
    std::vector<size_t> order(n);
    for (size_t i = 0; i < n; ++i)
        order[i] = i;

    std::sort(order.begin(), order.end(),
        [&](size_t a, size_t b)
        {
            if (fabs(vertices[a].y - vertices[b].y) > 1e-10)
                return vertices[a].y > vertices[b].y;
            return vertices[a].x < vertices[b].x;
        });

    size_t top = order.front();
    size_t bottom = order.back();

    // Step2: build chains - mark vertices on left chain
    std::vector<char> isLeftChain(n, 0);

    size_t v = top;
    while (v != bottom)
    {
        isLeftChain[v] = 1;
        v = (v + 1) % n;
    }

    // Step3: triangulate using stack
    std::vector<size_t> stack;
    stack.push_back(order[0]);
    stack.push_back(order[1]);

    for (size_t i = 2; i < n - 1; ++i)
    {
        size_t curr = order[i];

        // Check if current vertex and stack top are on different chains
        if (isLeftChain[curr] != isLeftChain[stack.back()])
        {
            // Different chains: pop all and form triangles
            size_t v_prev = stack.back();
            stack.pop_back();

            while (!stack.empty())
            {
                size_t v_top = stack.back();
                stack.pop_back();

                // Check if all edges are valid (polygon edges or valid diagonals)
                bool edge1_valid = AreAdjacentInPolygon(curr, v_prev, vertices) ||
                                  IsValidDiagonal(vertices, curr, v_prev);
                bool edge2_valid = AreAdjacentInPolygon(v_prev, v_top, vertices) ||
                                  IsValidDiagonal(vertices, v_prev, v_top);
                bool edge3_valid = AreAdjacentInPolygon(v_top, curr, vertices) ||
                                  IsValidDiagonal(vertices, v_top, curr);

                if (edge1_valid && edge2_valid && edge3_valid) {
                    result.triangles.emplace_back(
                        vertices[curr], vertices[v_prev], vertices[v_top]);
                }

                v_prev = v_top;
            }

            stack.push_back(order[i - 1]);
            stack.push_back(curr);
        }
        else
        {
            // Same chain: pop while diagonal is valid
            size_t v_last = stack.back();
            stack.pop_back();

            while (!stack.empty())
            {
                size_t v_top = stack.back();

                // Check if (v_last, curr, v_top) is a valid diagonal
                Vector2D v1 = vertices[curr] - vertices[v_last];
                Vector2D v2 = vertices[v_top] - vertices[v_last];
                double cross = v1.Cross(v2);

                bool is_convex = false;
                if (isLeftChain[curr])
                {
                    // On left chain, need cross < 0 (right turn)
                    is_convex = (cross < -1e-10);
                }
                else
                {
                    // On right chain, need cross > 0 (left turn)
                    is_convex = (cross > 1e-10);
                }

                if (is_convex)
                {
                    // Check if all edges are valid
                    bool edge1_valid = AreAdjacentInPolygon(curr, v_last, vertices) ||
                                      IsValidDiagonal(vertices, curr, v_last);
                    bool edge2_valid = AreAdjacentInPolygon(v_last, v_top, vertices) ||
                                      IsValidDiagonal(vertices, v_last, v_top);
                    bool edge3_valid = AreAdjacentInPolygon(v_top, curr, vertices) ||
                                      IsValidDiagonal(vertices, v_top, curr);

                    if (edge1_valid && edge2_valid && edge3_valid) {
                        result.triangles.emplace_back(
                            vertices[curr], vertices[v_last], vertices[v_top]);
                    }

                    v_last = v_top;
                    stack.pop_back();
                }
                else
                    break;
            }

            stack.push_back(v_last);
            stack.push_back(curr);
        }
    }

    // Step4: connect remaining vertices to bottom
    size_t bottom_idx = order.back();
    size_t v_prev = stack.back();
    stack.pop_back();

    while (!stack.empty())
    {
        size_t v_top = stack.back();
        stack.pop_back();

        // Check if all edges are valid
        bool edge1_valid = AreAdjacentInPolygon(bottom_idx, v_prev, vertices) ||
                          IsValidDiagonal(vertices, bottom_idx, v_prev);
        bool edge2_valid = AreAdjacentInPolygon(v_prev, v_top, vertices) ||
                          IsValidDiagonal(vertices, v_prev, v_top);
        bool edge3_valid = AreAdjacentInPolygon(v_top, bottom_idx, vertices) ||
                          IsValidDiagonal(vertices, v_top, bottom_idx);

        if (edge1_valid && edge2_valid && edge3_valid) {
            result.triangles.emplace_back(
                vertices[bottom_idx], vertices[v_prev], vertices[v_top]);
        }

        v_prev = v_top;
    }

    return result;
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
      // Use ToLeftTest for strict left turn check (cross > 0)
      Vector2D v1 = p_curr - p_prev;
      Vector2D v2 = p_next - p_curr;
      if (internal::GeometryCore::ToLeftTest(v1, v2)) {
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
  // Use ToLeftTest for strict left turn check (cross > 0)
  Vector2D v1 = polygon[i] - polygon[prev_i];
  Vector2D v2 = polygon[j] - polygon[prev_i];
  bool left1 = internal::GeometryCore::ToLeftTest(v1, v2);
  
  Vector2D v3 = polygon[next_i] - polygon[i];
  Vector2D v4 = polygon[j] - polygon[i];
  bool left2 = internal::GeometryCore::ToLeftTest(v3, v4);
  
  if (!left1 && !left2) {
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

bool Triangulation::AreAdjacentInPolygon(
    size_t i,
    size_t j,
    const std::vector<Point2D>& polygon) {

  size_t n = polygon.size();

  // Check if i and j are adjacent in the polygon
  size_t next_i = (i + 1) % n;
  size_t prev_i = (i + n - 1) % n;

  return (j == next_i || j == prev_i);
}

}  // namespace geometry
