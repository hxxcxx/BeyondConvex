#include "voronoi_diagram.h"
#include "../core/geometry_core.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <utility>

namespace geometry {

VoronoiDiagramResult VoronoiDiagram::Generate(
    const std::vector<Point2D>& sites,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  // Use incremental algorithm for simplicity
  return GenerateIncremental(sites, bounds_min_x, bounds_min_y, 
                             bounds_max_x, bounds_max_y);
}

VoronoiDiagramResult VoronoiDiagram::GenerateIncremental(
    const std::vector<Point2D>& sites,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  VoronoiDiagramResult result;
  
  if (sites.empty()) {
    return result;
  }
  
  result.sites = sites;
  
  // Initialize with bounding box
  std::vector<Point2D> bounds = {
    Point2D(bounds_min_x, bounds_min_y),
    Point2D(bounds_max_x, bounds_min_y),
    Point2D(bounds_max_x, bounds_max_y),
    Point2D(bounds_min_x, bounds_max_y)
  };
  
  // For each site, compute its Voronoi cell
  for (size_t i = 0; i < sites.size(); ++i) {
    VoronoiCell cell;
    cell.site_index = i;
    cell.site = sites[i];
    
    // Start with bounding box
    std::vector<Point2D> cell_polygon = bounds;
    
    // Clip against half-planes defined by bisectors with other sites
    for (size_t j = 0; j < sites.size(); ++j) {
      if (i == j) continue;
      
      // Compute bisector between site i and site j
      Vector2D to_j = sites[j] - sites[i];
      Point2D midpoint(sites[i].x + to_j.x * 0.5, sites[i].y + to_j.y * 0.5);
      
      // Normal points towards site i (perpendicular to bisector)
      Vector2D normal(-to_j.y, to_j.x);
      
      // Clip polygon to half-plane: points closer to site i than to site j
      // A point p is in the half-plane if: (p - midpoint) · normal >= 0
      cell_polygon = ClipPolygonToHalfplane(cell_polygon, midpoint, normal);
      
      if (cell_polygon.empty()) {
        break;  // Cell is empty
      }
    }
    
    // Store cell vertices and edges
    if (!cell_polygon.empty()) {
      cell.vertices = cell_polygon;
      
      // Create edges from vertices
      for (size_t k = 0; k < cell_polygon.size(); ++k) {
        size_t next = (k + 1) % cell_polygon.size();
        cell.edges.emplace_back(cell_polygon[k], cell_polygon[next]);
      }
      
      result.cells.push_back(cell);
    }
  }
  
  // Collect unique Voronoi edges (edges that are shared by two cells)
  std::map<std::pair<Point2D, Point2D>, int> edge_count;
  
  for (const auto& cell : result.cells) {
    for (const auto& edge : cell.edges) {
      // Create a canonical representation of the edge
      Point2D p1 = edge.p1;
      Point2D p2 = edge.p2;
      if (p2 < p1) std::swap(p1, p2);
      
      auto key = std::make_pair(p1, p2);
      edge_count[key]++;
    }
  }
  
  // Only add edges that appear in multiple cells (shared edges)
  // These are the true Voronoi edges, not boundary edges
  for (const auto& [edge, count] : edge_count) {
    if (count >= 2) {
      result.edges.emplace_back(edge.first, edge.second);
    }
  }
  
  // Collect unique vertices
  std::set<Point2D> unique_vertices;
  for (const auto& cell : result.cells) {
    for (const auto& v : cell.vertices) {
      unique_vertices.insert(v);
    }
  }
  
  for (const auto& v : unique_vertices) {
    result.vertices.push_back(v);
  }
  
  return result;
}

Edge2D VoronoiDiagram::ComputeBisector(
    const Point2D& p1,
    const Point2D& p2) {
  
  // Compute perpendicular bisector
  Vector2D to_p2 = p2 - p1;
  Point2D midpoint(p1.x + to_p2.x * 0.5, p1.y + to_p2.y * 0.5);
  
  // Perpendicular direction
  Vector2D perp(-to_p2.y, to_p2.x);
  perp = perp.Normalize();
  
  Point2D p_start(midpoint.x + perp.x * -1000.0, midpoint.y + perp.y * -1000.0);
  Point2D p_end(midpoint.x + perp.x * 1000.0, midpoint.y + perp.y * 1000.0);
  
  return Edge2D(p_start, p_end);
}

Point2D VoronoiDiagram::ComputeCircumcenter(
    const Point2D& p1,
    const Point2D& p2,
    const Point2D& p3) {
  
  // Using the formula for circumcenter
  double ax = p1.x, ay = p1.y;
  double bx = p2.x, by = p2.y;
  double cx = p3.x, cy = p3.y;
  
  double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
  
  if (std::abs(d) < 1e-10) {
    // Degenerate case, return centroid
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

bool VoronoiDiagram::IsCloser(
    const Point2D& point,
    const Point2D& site1,
    const Point2D& site2) {
  
  double dist1 = (point - site1).LengthSquared();
  double dist2 = (point - site2).LengthSquared();
  
  return dist1 < dist2;
}

std::vector<Point2D> VoronoiDiagram::ClipPolygonToHalfplane(
    const std::vector<Point2D>& polygon,
    const Point2D& point_on_line,
    const Vector2D& normal) {
  
  std::vector<Point2D> result;
  
  if (polygon.empty()) {
    return result;
  }
  
  for (size_t i = 0; i < polygon.size(); ++i) {
    size_t next = (i + 1) % polygon.size();
    
    const Point2D& p1 = polygon[i];
    const Point2D& p2 = polygon[next];
    
    // Check which side each point is on
    // Point is inside if (p - point_on_line) · normal >= 0
    Vector2D v1 = p1 - point_on_line;
    Vector2D v2 = p2 - point_on_line;
    
    double dot1 = v1.x * normal.x + v1.y * normal.y;
    double dot2 = v2.x * normal.x + v2.y * normal.y;
    
    bool inside1 = dot1 >= -1e-10;
    bool inside2 = dot2 >= -1e-10;
    
    if (inside1) {
      result.push_back(p1);
    }
    
    // If edges cross the line, add intersection point
    if (inside1 != inside2) {
      Point2D intersection = IntersectLineWithHalfplane(p1, p2, point_on_line, normal);
      result.push_back(intersection);
    }
  }
  
  return result;
}

Point2D VoronoiDiagram::IntersectLineWithHalfplane(
    const Point2D& seg_start,
    const Point2D& seg_end,
    const Point2D& point_on_line,
    const Vector2D& normal) {
  
  Vector2D seg_dir = seg_end - seg_start;
  Vector2D to_line = seg_start - point_on_line;
  
  // Line: (p - point_on_line) · normal = 0
  // Segment: p = seg_start + t * seg_dir
  // Solve: (seg_start + t * seg_dir - point_on_line) · normal = 0
  
  double denom = seg_dir.x * normal.x + seg_dir.y * normal.y;
  
  if (std::abs(denom) < 1e-10) {
    return seg_start;  // Parallel, return start point
  }
  
  double t = -(to_line.x * normal.x + to_line.y * normal.y) / denom;
  
  return Point2D(seg_start.x + seg_dir.x * t, seg_start.y + seg_dir.y * t);
}

VoronoiDiagramResult VoronoiDiagram::GenerateFortune(
    const std::vector<Point2D>& sites,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  // Fortune's algorithm is complex, use incremental for now
  return GenerateIncremental(sites, bounds_min_x, bounds_min_y,
                             bounds_max_x, bounds_max_y);
}

}  // namespace geometry
