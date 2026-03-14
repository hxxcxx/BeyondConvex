#ifndef GEOMETRY_VORONOI_DIAGRAM_H_
#define GEOMETRY_VORONOI_DIAGRAM_H_

#include "../core/point2d.h"
#include "../core/edge2d.h"
#include "../core/vector2d.h"
#include "../dcel/dcel.h"
#include <vector>
#include <memory>
#include <set>
#include <map>
#include <functional>

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

// Voronoi algorithm type
enum class VoronoiAlgorithm {
  kIncremental,      // Incremental half-plane clipping (current implementation)
  kIncrementalDCEL,  // Incremental algorithm using DCEL structure
  // Future algorithms:
  // kFortune,         // Fortune's sweep line algorithm
  // kDivideConquer,   // Divide and conquer algorithm
};

// Forward declaration
class IVoronoiAlgorithm;

// Voronoi diagram class (Facade)
class VoronoiDiagram {
 public:
  // Generate Voronoi diagram from sites (default: incremental algorithm)
  static VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      double bounds_min_x = -100.0,
      double bounds_min_y = -100.0,
      double bounds_max_x = 100.0,
      double bounds_max_y = 100.0);
  
  // Generate Voronoi diagram with specified algorithm
  static VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      VoronoiAlgorithm algorithm,
      double bounds_min_x = -100.0,
      double bounds_min_y = -100.0,
      double bounds_max_x = 100.0,
      double bounds_max_y = 100.0);
  
  // Generate DCEL-format Voronoi diagram
  static DCEL* GenerateDCEL(
      const std::vector<Point2D>& sites,
      VoronoiAlgorithm algorithm = VoronoiAlgorithm::kIncrementalDCEL,
      double bounds_min_x = -100.0,
      double bounds_min_y = -100.0,
      double bounds_max_x = 100.0,
      double bounds_max_y = 100.0);
  
  // Get algorithm instance
  static std::unique_ptr<IVoronoiAlgorithm> CreateAlgorithm(
      VoronoiAlgorithm algorithm);
  
  // Get all supported algorithms
  static std::vector<VoronoiAlgorithm> GetSupportedAlgorithms();
  
  // Algorithm registry (public for registration)
  static std::map<VoronoiAlgorithm, 
                  std::function<std::unique_ptr<IVoronoiAlgorithm>()>> 
      algorithm_registry_;
  
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
};

// Voronoi algorithm interface (Abstract Strategy)
class IVoronoiAlgorithm {
 public:
  virtual ~IVoronoiAlgorithm() = default;
  
  // Generate Voronoi diagram
  virtual VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      double bounds_min_x,
      double bounds_min_y,
      double bounds_max_x,
      double bounds_max_y) = 0;
  
  // Generate DCEL-format Voronoi diagram
  virtual DCEL* GenerateDCEL(
      const std::vector<Point2D>& sites,
      double bounds_min_x,
      double bounds_min_y,
      double bounds_max_x,
      double bounds_max_y) = 0;
  
  // Algorithm information
  virtual std::string Name() const = 0;
  virtual std::string Complexity() const = 0;
  virtual std::string Description() const = 0;
};

// Incremental half-plane clipping algorithm (current implementation)
class IncrementalVoronoiAlgorithm : public IVoronoiAlgorithm {
 public:
  VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      double bounds_min_x,
      double bounds_min_y,
      double bounds_max_x,
      double bounds_max_y) override;
  
  DCEL* GenerateDCEL(
      const std::vector<Point2D>& sites,
      double bounds_min_x,
      double bounds_min_y,
      double bounds_max_x,
      double bounds_max_y) override;
  
  std::string Name() const override { 
    return "Incremental Half-Plane Clipping"; 
  }
  
  std::string Complexity() const override { 
    return "O(n²) ~ O(n³)"; 
  }
  
  std::string Description() const override {
    return "Incremental algorithm using half-plane clipping. "
           "Simple implementation but slower for large datasets.";
  }
  
 private:
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
};

// Incremental algorithm using DCEL structure
class IncrementalDCELAlgorithm : public IVoronoiAlgorithm {
 public:
  VoronoiDiagramResult Generate(
      const std::vector<Point2D>& sites,
      double bounds_min_x,
      double bounds_min_y,
      double bounds_max_x,
      double bounds_max_y) override;
  
  DCEL* GenerateDCEL(
      const std::vector<Point2D>& sites,
      double bounds_min_x,
      double bounds_min_y,
      double bounds_max_x,
      double bounds_max_y) override;
  
  std::string Name() const override { 
    return "Incremental DCEL"; 
  }
  
  std::string Complexity() const override { 
    return "O(n²)"; 
  }
  
  std::string Description() const override {
    return "Incremental algorithm using DCEL data structure. "
           "Maintains topological relationships between cells.";
  }
  
 private:
  // Helper: Create initial bounding box cell
  static Face* CreateBoundingBoxCell(
      DCEL* dcel,
      const Point2D& site,
      double bounds_min_x,
      double bounds_min_y,
      double bounds_max_x,
      double bounds_max_y);
  
  // Helper: Clip cell by half-plane
  static void ClipCellByHalfPlane(
      DCEL* dcel,
      Face* cell,
      const Point2D& point_on_line,
      const Vector2D& normal);
  
  // Helper: Merge shared edges
  static void MergeSharedEdges(DCEL* dcel);
};

}  // namespace geometry

#endif  // GEOMETRY_VORONOI_DIAGRAM_H_
