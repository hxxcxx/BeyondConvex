#include "voronoi_incremental.h"
#include "../dcel/dcel_builder.h"
#include <map>

namespace geometry {

VoronoiDiagramResult IncrementalVoronoi::Generate(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) {
  
  VoronoiDiagramResult result;
  
  if (sites.empty()) {
    return result;
  }
  
  if (sites.size() == 1) {
    // Single site: entire bounding box is one cell
    result.sites = sites;
    result.cells.push_back({0, sites[0], CreateBoundingBox(bounds), {}});
    return result;
  }
  
  result.sites = sites;
  
  // Compute Voronoi cell for each site
  for (size_t i = 0; i < sites.size(); ++i) {
    std::vector<Point2D> cell_vertices = ComputeCell(
        sites[i], sites, i, bounds);
    
    // Create cell
    VoronoiCell cell;
    cell.site_index = i;
    cell.site = sites[i];
    cell.vertices = cell_vertices;
    
    // Create edges from vertices
    for (size_t j = 0; j < cell_vertices.size(); ++j) {
      size_t next = (j + 1) % cell_vertices.size();
      cell.edges.emplace_back(cell_vertices[j], cell_vertices[next]);
    }
    
    result.cells.push_back(cell);
  }
  
  // Collect all unique edges
  std::set<Edge2D> edge_set;
  for (const auto& cell : result.cells) {
    for (const auto& edge : cell.edges) {
      edge_set.insert(edge);
    }
  }
  
  // Convert set to vector
  for (const auto& edge : edge_set) {
    result.edges.push_back(edge);
  }
  
  // Collect all unique vertices
  std::set<Point2D> vertex_set;
  for (const auto& cell : result.cells) {
    for (const auto& vertex : cell.vertices) {
      vertex_set.insert(vertex);
    }
  }
  
  // Convert set to vector
  for (const auto& vertex : vertex_set) {
    result.vertices.push_back(vertex);
  }
  
  return result;
}

DCEL* IncrementalVoronoi::GenerateDCEL(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) {
  
  // Generate result first, then convert to DCEL
  VoronoiDiagramResult result = Generate(sites, bounds);
  
  DCEL* dcel = new DCEL();
  
  // Map from Point2D to Vertex*
  std::map<Point2D, Vertex*> vertex_map;
  
  // Create all vertices
  for (const auto& p : result.vertices) {
    vertex_map[p] = dcel->CreateVertex(p);
  }
  
  // Create faces for each cell
  for (const auto& cell : result.cells) {
    if (!cell.IsValid()) continue;
    
    // Get vertices for this cell
    std::vector<Vertex*> cell_vertices;
    for (const auto& p : cell.vertices) {
      if (vertex_map.find(p) == vertex_map.end()) {
        vertex_map[p] = dcel->CreateVertex(p);
      }
      cell_vertices.push_back(vertex_map[p]);
    }
    
    // Build polygon using DCELBuilder
    if (cell_vertices.size() >= 3) {
      std::vector<Point2D> polygon_points;
      for (auto* v : cell_vertices) {
        polygon_points.push_back(v->GetCoordinates());
      }
      
      // Create face from polygon
      Face* face = DCELBuilder::BuildPolygon(polygon_points, dcel);
    }
  }
  
  // Merge shared edges
  std::map<std::pair<Point2D, Point2D>, std::vector<HalfEdge*>> edge_map;
  
  for (size_t i = 0; i < dcel->GetHalfEdgeCount(); ++i) {
    HalfEdge* edge = dcel->GetHalfEdge(i);
    if (!edge || !edge->GetOrigin() || !edge->GetDestination()) continue;
    
    Point2D p1 = edge->GetOrigin()->GetCoordinates();
    Point2D p2 = edge->GetDestination()->GetCoordinates();
    
    // Normalize edge direction
    if (p1.x > p2.x || (p1.x == p2.x && p1.y > p2.y)) {
      std::swap(p1, p2);
    }
    
    edge_map[{p1, p2}].push_back(edge);
  }
  
  // Set twin relationships
  for (auto& [key, edges] : edge_map) {
    if (edges.size() == 2) {
      edges[0]->SetTwin(edges[1]);
      edges[1]->SetTwin(edges[0]);
    }
  }
  
  return dcel;
}

std::vector<Point2D> IncrementalVoronoi::CreateBoundingBox(
    const VoronoiBounds& bounds) const {
  
  return {
    Point2D(bounds.min_x, bounds.min_y),
    Point2D(bounds.max_x, bounds.min_y),
    Point2D(bounds.max_x, bounds.max_y),
    Point2D(bounds.min_x, bounds.max_y)
  };
}

std::vector<Point2D> IncrementalVoronoi::ComputeCell(
    const Point2D& site,
    const std::vector<Point2D>& all_sites,
    size_t site_index,
    const VoronoiBounds& bounds) const {
  
  // Start with bounding box
  std::vector<Point2D> cell = CreateBoundingBox(bounds);
  
  // Clip against all other sites
  for (size_t j = 0; j < all_sites.size(); ++j) {
    if (j == site_index) continue;
    
    // Compute perpendicular bisector
    Point2D midpoint;
    Vector2D normal;
    HalfPlaneClipper::ComputeBisector(site, all_sites[j], midpoint, normal);
    
    // Clip cell by half-plane
    cell = HalfPlaneClipper::ClipPolygon(cell, midpoint, normal);
    
    // Early termination if cell becomes empty
    if (cell.empty()) {
      break;
    }
  }
  
  return cell;
}

}  // namespace geometry
