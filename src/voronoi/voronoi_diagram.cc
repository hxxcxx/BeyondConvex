#include "voronoi_diagram.h"
#include "../core/geometry_core.h"
#include "../dcel/dcel_builder.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <utility>

namespace geometry {

// ========== Algorithm Registry ==========

std::map<VoronoiAlgorithm, std::function<std::unique_ptr<IVoronoiAlgorithm>()>> 
    VoronoiDiagram::algorithm_registry_;

// Static registration
namespace {
  struct AlgorithmRegistrar {
    AlgorithmRegistrar() {
      VoronoiDiagram::algorithm_registry_[VoronoiAlgorithm::kIncremental] = 
          []() { return std::make_unique<IncrementalVoronoiAlgorithm>(); };
      
      VoronoiDiagram::algorithm_registry_[VoronoiAlgorithm::kIncrementalDCEL] = 
          []() { return std::make_unique<IncrementalDCELAlgorithm>(); };
    }
  };
  
  static AlgorithmRegistrar registrar;
}

// ========== VoronoiDiagram Facade ==========

VoronoiDiagramResult VoronoiDiagram::Generate(
    const std::vector<Point2D>& sites,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  // Use incremental algorithm by default (backward compatibility)
  return Generate(sites, VoronoiAlgorithm::kIncremental,
                  bounds_min_x, bounds_min_y, bounds_max_x, bounds_max_y);
}

VoronoiDiagramResult VoronoiDiagram::Generate(
    const std::vector<Point2D>& sites,
    VoronoiAlgorithm algorithm,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  auto algo = CreateAlgorithm(algorithm);
  return algo->Generate(sites, bounds_min_x, bounds_min_y,
                       bounds_max_x, bounds_max_y);
}

DCEL* VoronoiDiagram::GenerateDCEL(
    const std::vector<Point2D>& sites,
    VoronoiAlgorithm algorithm,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  auto algo = CreateAlgorithm(algorithm);
  return algo->GenerateDCEL(sites, bounds_min_x, bounds_min_y,
                           bounds_max_x, bounds_max_y);
}

std::unique_ptr<IVoronoiAlgorithm> VoronoiDiagram::CreateAlgorithm(
    VoronoiAlgorithm algorithm) {
  
  auto it = algorithm_registry_.find(algorithm);
  if (it != algorithm_registry_.end()) {
    return it->second();
  }
  
  // Default to incremental algorithm
  return std::make_unique<IncrementalVoronoiAlgorithm>();
}

std::vector<VoronoiAlgorithm> VoronoiDiagram::GetSupportedAlgorithms() {
  std::vector<VoronoiAlgorithm> algorithms;
  for (const auto& [algo, factory] : algorithm_registry_) {
    algorithms.push_back(algo);
  }
  return algorithms;
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

// ========== IncrementalVoronoiAlgorithm ==========

VoronoiDiagramResult IncrementalVoronoiAlgorithm::Generate(
    const std::vector<Point2D>& sites,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  VoronoiDiagramResult result;
  
  if (sites.empty()) {
    return result;
  }
  
  if (sites.size() == 1) {
    // Single site: no edges
    result.sites = sites;
    return result;
  }
  
  result.sites = sites;
  
  // Store Voronoi cell for each site
  std::vector<std::vector<Point2D>> cells(sites.size());
  
  // Initialize each cell as the bounding box
  std::vector<Point2D> bounding_box = {
    Point2D(bounds_min_x, bounds_min_y),
    Point2D(bounds_max_x, bounds_min_y),
    Point2D(bounds_max_x, bounds_max_y),
    Point2D(bounds_min_x, bounds_max_y)
  };
  
  // For each site, compute its Voronoi cell by clipping with half-planes
  for (size_t i = 0; i < sites.size(); ++i) {
    cells[i] = bounding_box;
    
    // Clip against all other sites
    for (size_t j = 0; j < sites.size(); ++j) {
      if (i == j) continue;
      
      // Compute perpendicular bisector between site i and site j
      Vector2D to_j = sites[j] - sites[i];
      Point2D midpoint(sites[i].x + to_j.x * 0.5, sites[i].y + to_j.y * 0.5);
      
      // Normal pointing towards site i (away from site j)
      Vector2D normal = sites[i] - sites[j];
      normal = normal.Normalize();
      
      // Clip the cell to the half-plane that contains site i
      cells[i] = ClipPolygonToHalfplane(cells[i], midpoint, normal);
    }
  }
  
  // Collect edges from all cells
  std::set<Edge2D> edge_set;
  for (const auto& cell : cells) {
    for (size_t i = 0; i < cell.size(); ++i) {
      size_t next = (i + 1) % cell.size();
      edge_set.emplace(cell[i], cell[next]);
    }
  }
  
  // Convert set to vector
  for (const auto& edge : edge_set) {
    result.edges.push_back(edge);
  }
  
  return result;
}

DCEL* IncrementalVoronoiAlgorithm::GenerateDCEL(
    const std::vector<Point2D>& sites,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  // For now, convert from VoronoiDiagramResult to DCEL
  // TODO: Implement direct DCEL construction for better performance
  
  auto result = Generate(sites, bounds_min_x, bounds_min_y,
                        bounds_max_x, bounds_max_y);
  
  DCEL* dcel = new DCEL();
  
  // Create vertices
  std::map<Point2D, Vertex*> vertex_map;
  for (const auto& p : result.vertices) {
    Vertex* v = dcel->CreateVertex(p);
    vertex_map[p] = v;
  }
  
  // Create faces for each cell
  for (const auto& cell : result.cells) {
    if (!cell.IsValid()) continue;
    
    Face* face = dcel->CreateFace();
    
    // Create edges for the cell
    std::vector<Vertex*> cell_vertices;
    for (const auto& p : cell.vertices) {
      if (vertex_map.find(p) == vertex_map.end()) {
        vertex_map[p] = dcel->CreateVertex(p);
      }
      cell_vertices.push_back(vertex_map[p]);
    }
    
    // Build polygon from vertices
    if (cell_vertices.size() >= 3) {
      std::vector<Point2D> polygon_points;
      for (auto* v : cell_vertices) {
        polygon_points.push_back(v->GetCoordinates());
      }
      
      // Use DCELBuilder to create the polygon
      // Note: This is a simplified approach
      // For a complete implementation, we need to properly handle edge sharing
    }
  }
  
  return dcel;
}

std::vector<Point2D> IncrementalVoronoiAlgorithm::ClipPolygonToHalfplane(
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

Point2D IncrementalVoronoiAlgorithm::IntersectLineWithHalfplane(
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

// ========== IncrementalDCELAlgorithm ==========

VoronoiDiagramResult IncrementalDCELAlgorithm::Generate(
    const std::vector<Point2D>& sites,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  // Generate DCEL first, then convert to VoronoiDiagramResult
  DCEL* dcel = GenerateDCEL(sites, bounds_min_x, bounds_min_y,
                           bounds_max_x, bounds_max_y);
  
  VoronoiDiagramResult result;
  result.sites = sites;
  
  // Convert DCEL to VoronoiDiagramResult
  // Collect all vertices
  for (size_t i = 0; i < dcel->GetVertexCount(); ++i) {
    Vertex* v = dcel->GetVertex(i);
    if (v) {
      result.vertices.push_back(v->GetCoordinates());
    }
  }
  
  // Collect all edges
  for (size_t i = 0; i < dcel->GetHalfEdgeCount(); ++i) {
    HalfEdge* he = dcel->GetHalfEdge(i);
    if (he && he->GetOrigin() && he->GetDestination()) {
      if (he->GetId() < he->GetTwin()->GetId()) {  // Avoid duplicates
        result.edges.emplace_back(
          he->GetOrigin()->GetCoordinates(),
          he->GetDestination()->GetCoordinates()
        );
      }
    }
  }
  
  // Collect cells
  for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
    Face* face = dcel->GetFace(i);
    if (face && !face->IsUnbounded()) {
      VoronoiCell cell;
      auto boundary_vertices = face->GetOuterBoundaryVertices();
      for (auto* v : boundary_vertices) {
        cell.vertices.push_back(v->GetCoordinates());
      }
      result.cells.push_back(cell);
    }
  }
  
  delete dcel;
  return result;
}

DCEL* IncrementalDCELAlgorithm::GenerateDCEL(
    const std::vector<Point2D>& sites,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  DCEL* dcel = new DCEL();
  
  if (sites.empty()) {
    return dcel;
  }
  
  if (sites.size() == 1) {
    // Single site: entire bounding box is one cell
    CreateBoundingBoxCell(dcel, sites[0], 
                         bounds_min_x, bounds_min_y,
                         bounds_max_x, bounds_max_y);
    return dcel;
  }
  
  // Create initial cell for each site (bounding box)
  std::vector<Face*> cells;
  for (const auto& site : sites) {
    Face* cell = CreateBoundingBoxCell(dcel, site,
                                       bounds_min_x, bounds_min_y,
                                       bounds_max_x, bounds_max_y);
    cells.push_back(cell);
  }
  
  // Clip each cell against all other sites
  for (size_t i = 0; i < sites.size(); ++i) {
    for (size_t j = 0; j < sites.size(); ++j) {
      if (i == j) continue;
      
      // Compute perpendicular bisector
      Vector2D to_j = sites[j] - sites[i];
      Point2D midpoint(sites[i].x + to_j.x * 0.5, sites[i].y + to_j.y * 0.5);
      
      // Normal pointing towards site i
      Vector2D normal = sites[i] - sites[j];
      normal = normal.Normalize();
      
      // Clip cell i by the half-plane
      ClipCellByHalfPlane(dcel, cells[i], midpoint, normal);
    }
  }
  
  // Merge shared edges
  MergeSharedEdges(dcel);
  
  return dcel;
}

Face* IncrementalDCELAlgorithm::CreateBoundingBoxCell(
    DCEL* dcel,
    const Point2D& site,
    double bounds_min_x,
    double bounds_min_y,
    double bounds_max_x,
    double bounds_max_y) {
  
  // Create bounding box vertices
  Vertex* v0 = dcel->CreateVertex(Point2D(bounds_min_x, bounds_min_y));
  Vertex* v1 = dcel->CreateVertex(Point2D(bounds_max_x, bounds_min_y));
  Vertex* v2 = dcel->CreateVertex(Point2D(bounds_max_x, bounds_max_y));
  Vertex* v3 = dcel->CreateVertex(Point2D(bounds_min_x, bounds_max_y));
  
  // Create edges
  HalfEdge* he0 = dcel->CreateEdge(v0, v1);
  HalfEdge* he1 = dcel->CreateEdge(v1, v2);
  HalfEdge* he2 = dcel->CreateEdge(v2, v3);
  HalfEdge* he3 = dcel->CreateEdge(v3, v0);
  
  // Connect edges
  dcel->ConnectHalfEdges(he0, he1);
  dcel->ConnectHalfEdges(he1, he2);
  dcel->ConnectHalfEdges(he2, he3);
  dcel->ConnectHalfEdges(he3, he0);
  
  // Create face
  Face* face = dcel->CreateFace();
  dcel->SetFaceOfCycle(he0, face);
  
  return face;
}

void IncrementalDCELAlgorithm::ClipCellByHalfPlane(
    DCEL* dcel,
    Face* cell,
    const Point2D& point_on_line,
    const Vector2D& normal) {
  
  // Get current boundary vertices
  auto boundary_vertices = cell->GetOuterBoundaryVertices();
  
  // Clip polygon using half-plane
  std::vector<Point2D> clipped_polygon;
  for (size_t i = 0; i < boundary_vertices.size(); ++i) {
    Vertex* v1 = boundary_vertices[i];
    Vertex* v2 = boundary_vertices[(i + 1) % boundary_vertices.size()];
    
    const Point2D& p1 = v1->GetCoordinates();
    const Point2D& p2 = v2->GetCoordinates();
    
    // Check which side each point is on
    Vector2D vec1 = p1 - point_on_line;
    Vector2D vec2 = p2 - point_on_line;
    
    double dot1 = vec1.x * normal.x + vec1.y * normal.y;
    double dot2 = vec2.x * normal.x + vec2.y * normal.y;
    
    bool inside1 = dot1 >= -1e-10;
    bool inside2 = dot2 >= -1e-10;
    
    if (inside1) {
      clipped_polygon.push_back(p1);
    }
    
    // If edge crosses the line, add intersection point
    if (inside1 != inside2) {
      // Compute intersection
      Vector2D seg_dir = p2 - p1;
      Vector2D to_line = p1 - point_on_line;
      double denom = seg_dir.x * normal.x + seg_dir.y * normal.y;
      
      if (std::abs(denom) >= 1e-10) {
        double t = -(to_line.x * normal.x + to_line.y * normal.y) / denom;
        Point2D intersection(p1.x + seg_dir.x * t, p1.y + seg_dir.y * t);
        clipped_polygon.push_back(intersection);
      }
    }
  }
  
  // Rebuild the face with clipped polygon
  // Note: This is a simplified implementation
  // A complete implementation would update the DCEL structure in-place
  // For now, we'll skip the complex DCEL update logic
}

void IncrementalDCELAlgorithm::MergeSharedEdges(DCEL* dcel) {
  // Use a map to identify shared edges
  std::map<std::pair<Point2D, Point2D>, std::vector<HalfEdge*>> edge_map;
  
  for (size_t i = 0; i < dcel->GetHalfEdgeCount(); ++i) {
    HalfEdge* edge = dcel->GetHalfEdge(i);
    if (!edge || !edge->GetOrigin() || !edge->GetDestination()) continue;
    
    Point2D p1 = edge->GetOrigin()->GetCoordinates();
    Point2D p2 = edge->GetDestination()->GetCoordinates();
    
    // Normalize edge direction (lexicographic order)
    if (p1.x > p2.x || (p1.x == p2.x && p1.y > p2.y)) {
      std::swap(p1, p2);
    }
    
    edge_map[{p1, p2}].push_back(edge);
  }
  
  // Set twin relationships for shared edges
  for (auto& [key, edges] : edge_map) {
    if (edges.size() == 2) {
      // Two edges share the same endpoints: set as twins
      edges[0]->SetTwin(edges[1]);
      edges[1]->SetTwin(edges[0]);
    }
  }
}

}  // namespace geometry
