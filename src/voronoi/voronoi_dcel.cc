#include "voronoi_dcel.h"
#include <map>
#include <set>
#include <algorithm>

namespace geometry {

VoronoiDiagramResult DCELVoronoi::Generate(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) {
  
  // Generate DCEL first, then convert
  DCEL* dcel = GenerateDCEL(sites, bounds);
  VoronoiDiagramResult result = ConvertDCELToResult(dcel, sites);
  delete dcel;
  
  return result;
}

DCEL* DCELVoronoi::GenerateDCEL(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) {
  
  DCEL* dcel = new DCEL();
  
  if (sites.empty()) {
    return dcel;
  }
  
  if (sites.size() == 1) {
    // Single site: entire bounding box is one cell
    CreateBoundingBoxCell(dcel, sites[0], bounds);
    return dcel;
  }
  
  // Create initial cell for each site
  std::vector<Face*> cells;
  for (const auto& site : sites) {
    Face* cell = CreateBoundingBoxCell(dcel, site, bounds);
    cells.push_back(cell);
  }
  
  // Clip each cell against all other sites
  for (size_t i = 0; i < sites.size(); ++i) {
    for (size_t j = 0; j < sites.size(); ++j) {
      if (i == j) continue;
      
      // Compute perpendicular bisector
      Point2D midpoint;
      Vector2D normal;
      HalfPlaneClipper::ComputeBisector(sites[i], sites[j], midpoint, normal);
      
      // Clip cell i by the half-plane
      ClipFaceByHalfPlane(dcel, cells[i], midpoint, normal);
    }
  }
  
  // Merge shared edges
  MergeSharedEdges(dcel);
  
  return dcel;
}

Face* DCELVoronoi::CreateBoundingBoxCell(
    DCEL* dcel,
    const Point2D& site,
    const VoronoiBounds& bounds) const {
  
  // Create bounding box vertices
  Vertex* v0 = dcel->CreateVertex(Point2D(bounds.min_x, bounds.min_y));
  Vertex* v1 = dcel->CreateVertex(Point2D(bounds.max_x, bounds.min_y));
  Vertex* v2 = dcel->CreateVertex(Point2D(bounds.max_x, bounds.max_y));
  Vertex* v3 = dcel->CreateVertex(Point2D(bounds.min_x, bounds.max_y));
  
  // Create edges
  HalfEdge* he0 = dcel->CreateEdge(v0, v1);
  HalfEdge* he1 = dcel->CreateEdge(v1, v2);
  HalfEdge* he2 = dcel->CreateEdge(v2, v3);
  HalfEdge* he3 = dcel->CreateEdge(v3, v0);
  
  // Connect edges into a cycle
  dcel->ConnectHalfEdges(he0, he1);
  dcel->ConnectHalfEdges(he1, he2);
  dcel->ConnectHalfEdges(he2, he3);
  dcel->ConnectHalfEdges(he3, he0);
  
  // Create face
  Face* face = dcel->CreateFace();
  dcel->SetFaceOfCycle(he0, face);
  
  return face;
}

void DCELVoronoi::ClipFaceByHalfPlane(
    DCEL* dcel,
    Face* face,
    const Point2D& point_on_line,
    const Vector2D& normal) const {
  
  // Get current boundary vertices
  auto boundary_vertices = face->GetOuterBoundaryVertices();
  
  if (boundary_vertices.empty()) {
    return;
  }
  
  // Clip polygon using half-plane
  std::vector<Point2D> clipped_vertices;
  
  for (size_t i = 0; i < boundary_vertices.size(); ++i) {
    Vertex* v1 = boundary_vertices[i];
    Vertex* v2 = boundary_vertices[(i + 1) % boundary_vertices.size()];
    
    const Point2D& p1 = v1->GetCoordinates();
    const Point2D& p2 = v2->GetCoordinates();
    
    bool inside1 = HalfPlaneClipper::IsInsideHalfPlane(p1, point_on_line, normal);
    bool inside2 = HalfPlaneClipper::IsInsideHalfPlane(p2, point_on_line, normal);
    
    if (inside1) {
      clipped_vertices.push_back(p1);
    }
    
    if (inside1 != inside2) {
      Point2D intersection = HalfPlaneClipper::IntersectSegmentWithLine(
          p1, p2, point_on_line, normal);
      clipped_vertices.push_back(intersection);
    }
  }
  
  // Rebuild the face with clipped polygon
  // Note: This is a simplified implementation
  // A complete implementation would update the DCEL structure in-place
  // For now, we create a new polygon and replace the old one
  
  if (clipped_vertices.size() >= 3) {
    // Clear old face data
    // (In a complete implementation, we would properly update the DCEL)
    // For now, this is a placeholder
  }
}

void DCELVoronoi::MergeSharedEdges(DCEL* dcel) const {
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

VoronoiDiagramResult DCELVoronoi::ConvertDCELToResult(
    const DCEL* dcel,
    const std::vector<Point2D>& sites) const {
  
  VoronoiDiagramResult result;
  result.sites = sites;
  
  // Collect all vertices
  std::set<Point2D> vertex_set;
  for (size_t i = 0; i < dcel->GetVertexCount(); ++i) {
    Vertex* v = dcel->GetVertex(i);
    if (v) {
      vertex_set.insert(v->GetCoordinates());
    }
  }
  
  for (const auto& v : vertex_set) {
    result.vertices.push_back(v);
  }
  
  // Collect all edges
  std::set<Edge2D> edge_set;
  for (size_t i = 0; i < dcel->GetHalfEdgeCount(); ++i) {
    HalfEdge* he = dcel->GetHalfEdge(i);
    if (he && he->GetOrigin() && he->GetDestination()) {
      if (he->GetId() < he->GetTwin()->GetId()) {
        edge_set.emplace(
          he->GetOrigin()->GetCoordinates(),
          he->GetDestination()->GetCoordinates()
        );
      }
    }
  }
  
  for (const auto& e : edge_set) {
    result.edges.push_back(e);
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
      
      // Create edges
      for (size_t j = 0; j < cell.vertices.size(); ++j) {
        size_t next = (j + 1) % cell.vertices.size();
        cell.edges.emplace_back(cell.vertices[j], cell.vertices[next]);
      }
      
      result.cells.push_back(cell);
    }
  }
  
  return result;
}

}  // namespace geometry
