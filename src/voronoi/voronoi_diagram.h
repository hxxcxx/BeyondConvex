#ifndef GEOMETRY_VORONOI_DIAGRAM_H_
#define GEOMETRY_VORONOI_DIAGRAM_H_

#include "../core/point2d.h"
#include "../core/edge2d.h"
#include "../core/vector2d.h"
#include <vector>
#include <memory>
#include <set>

namespace geometry {

// Voronoi cell representation
struct VoronoiCell {
  size_t site_index;           // Index of the site point
  Point2D site;                // The site point
  std::vector<Point2D> vertices; // Vertices of the cell (polygon)
  std::vector<Edge2D> edges;   // Edges of the cell
  
  bool IsValid() const {
    return !vertices.empty() && !edges.empty();
  }
};

// Voronoi diagram result
struct VoronoiDiagramResult {
  std::vector<Point2D> sites;      // Input sites
  std::vector<VoronoiCell> cells;  // Voronoi cells
  std::vector<Edge2D> edges;       // All Voronoi edges
  std::vector<Point2D> vertices;   // All Voronoi vertices
  
  bool IsValid() const {
    return !cells.empty();
  }
  
  size_t CellCount() const {
    return cells.size();
  }
};

// Voronoi diagram class
class VoronoiDiagram {
 public:
  // Generate Voronoi diagram from sites
  static VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      double bounds_min_x = -100.0,
      double bounds_min_y = -100.0,
      double bounds_max_x = 100.0,
      double bounds_max_y = 100.0);
  
  // Simple incremental algorithm (O(n²))
  static VoronoiDiagramResult GenerateIncremental(
      const std::vector<Point2D>& sites,
      double bounds_min_x,
      double bounds_min_y,
      double bounds_max_x,
      double bounds_max_y);
  
  // Fortune's algorithm (O(n log n)) - more complex
  static VoronoiDiagramResult GenerateFortune(
      const std::vector<Point2D>& sites,
      double bounds_min_x,
      double bounds_min_y,
      double bounds_max_x,
      double bounds_max_y);
  
 private:
  // Helper: Compute perpendicular bisector of two points
  static Edge2D ComputeBisector(
      const Point2D& p1,
      const Point2D& p2);
  
  // Helper: Compute circumcenter of three points
  static Point2D ComputeCircumcenter(
      const Point2D& p1,
      const Point2D& p2,
      const Point2D& p3);
  
  // Helper: Check if a point is closer to site1 than site2
  static bool IsCloser(
      const Point2D& point,
      const Point2D& site1,
      const Point2D& site2);
  
  // Helper: Clip polygon to half-plane defined by line
  static std::vector<Point2D> ClipPolygonToHalfplane(
      const std::vector<Point2D>& polygon,
      const Point2D& point_on_line,
      const Vector2D& normal);
  
  // Helper: Intersect line segment with half-plane
  static Point2D IntersectLineWithHalfplane(
      const Point2D& seg_start,
      const Point2D& seg_end,
      const Point2D& point_on_line,
      const Vector2D& normal);
  
  // Helper: Clip line segment to bounding box
  static std::vector<Point2D> ClipLineToBoundingBox(
      const Point2D& p1,
      const Point2D& p2,
      double bounds_min_x,
      double bounds_min_y,
      double bounds_max_x,
      double bounds_max_y);
};

}  // namespace geometry

#endif  // GEOMETRY_VORONOI_DIAGRAM_H_
