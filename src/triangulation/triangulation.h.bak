#ifndef GEOMETRY_TRIANGULATION_H_
#define GEOMETRY_TRIANGULATION_H_

#include "../core/point2d.h"
#include "../core/vector2d.h"
#include "../core/edge2d.h"
#include <vector>
#include <memory>

namespace geometry {

// Triangle representation
struct Triangle {
  Point2D v0, v1, v2;
  
  Triangle() = default;
  Triangle(const Point2D& a, const Point2D& b, const Point2D& c)
      : v0(a), v1(b), v2(c) {}
  
  // Calculate area using cross product
  double Area() const {
    Vector2D v1_to_v2 = v1 - v0;
    Vector2D v1_to_v3 = v2 - v0;
    return std::abs(v1_to_v2.Cross(v1_to_v3)) / 2.0;
  }
  
  // Check if point is inside triangle
  bool Contains(const Point2D& p) const;
  
  // Get circumcenter
  Point2D Circumcenter() const;
  
  // Get circumradius
  double Circumradius() const;
};

// Triangulation result
struct TriangulationResult {
  std::vector<Triangle> triangles;
  std::vector<Point2D> vertices;
  std::vector<Edge2D> edges;
  
  bool IsValid() const {
    return !triangles.empty();
  }
  
  size_t TriangleCount() const {
    return triangles.size();
  }
  
  double TotalArea() const {
    double total = 0.0;
    for (const auto& tri : triangles) {
      total += tri.Area();
    }
    return total;
  }
};

// Triangulation algorithm types
enum class TriangulationAlgorithm {
  kSweepLine,        // Sweep line algorithm (top to bottom)
  kEarClipping,       // Ear clipping method
  kDelaunay,          // Delaunay triangulation
};

// Triangulation class
class Triangulation {
 public:
  // Triangulate a polygon
  static TriangulationResult Triangulate(
      const std::vector<Point2D>& polygon,
      TriangulationAlgorithm algorithm = TriangulationAlgorithm::kSweepLine);
  
  // Sweep line triangulation (top to bottom)
  static TriangulationResult SweepLineTriangulation(
      const std::vector<Point2D>& polygon);
  
  // Ear clipping triangulation
  static TriangulationResult EarClippingTriangulation(
      const std::vector<Point2D>& polygon);
  
  // Delaunay triangulation
  static TriangulationResult DelaunayTriangulation(
      const std::vector<Point2D>& points);
  
  // Get supported algorithms
  static std::vector<TriangulationAlgorithm> GetSupportedAlgorithms();
  
  // Get algorithm name
  static std::string GetAlgorithmName(TriangulationAlgorithm algorithm);
  
  // Get algorithm complexity
  static std::string GetAlgorithmComplexity(TriangulationAlgorithm algorithm);
  
 private:
  // Helper: Check if polygon is simple (no self-intersections)
  static bool IsSimplePolygon(const std::vector<Point2D>& polygon);
  
  // Helper: Check if polygon vertices are in CCW order
  static bool IsCCW(const std::vector<Point2D>& polygon);
  
  // Helper: Calculate signed area
  static double SignedArea(const std::vector<Point2D>& polygon);
  
  // Helper: Check if diagonal is valid (inside polygon)
  static bool IsValidDiagonal(
      const std::vector<Point2D>& polygon,
      size_t i, size_t j);
  
  // Helper: Check if diagonal is inside polygon
  static bool IsDiagonalInside(
      const std::vector<Point2D>& polygon,
      size_t i, size_t j);
  
  // Helper: Check if diagonal intersects any polygon edge
  static bool DiagonalIntersectsEdges(
      const std::vector<Point2D>& polygon,
      size_t i, size_t j);

  // Helper: Check if two vertices are adjacent in polygon
  static bool AreAdjacentInPolygon(
      size_t i,
      size_t j,
      const std::vector<Point2D>& polygon);
  
  // Helper: Calculate cross product (b-a) x (c-a)
  static double Cross(const Point2D& a, const Point2D& b, const Point2D& c);
  
  // Helper: Check if point is inside triangle (including boundary)
  static bool PointInTriangle(
      const Point2D& p,
      const Point2D& a,
      const Point2D& b,
      const Point2D& c);
  
  // Helper: Check if point is strictly inside triangle (excluding boundary)
  static bool PointStrictlyInTriangle(
      const Point2D& p,
      const Point2D& a,
      const Point2D& b,
      const Point2D& c);
  
  // Helper: Check if three points form a convex corner (CCW turn)
  static bool IsConvex(
      const Point2D& prev,
      const Point2D& curr,
      const Point2D& next);
  
  // Helper: Remove collinear vertices from polygon
  static void RemoveCollinear(std::vector<Point2D>& poly);
};

}  // namespace geometry

#endif  // GEOMETRY_TRIANGULATION_H_
