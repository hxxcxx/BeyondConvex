#include "voronoi_dcel.h"
#include "voronoi_incremental.h"
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
  
  // Use IncrementalVoronoi to generate the result first
  // This is simpler and more reliable
  IncrementalVoronoi incremental;
  VoronoiDiagramResult result = incremental.Generate(sites, bounds);
  
  // Convert result to DCEL
  DCEL* dcel = new DCEL();
  
  if (result.sites.empty()) {
    return dcel;
  }
  
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
    
    // Build polygon
    if (cell_vertices.size() >= 3) {
      // Create edges
      std::vector<HalfEdge*> edges;
      for (size_t i = 0; i < cell_vertices.size(); ++i) {
        Vertex* v1 = cell_vertices[i];
        Vertex* v2 = cell_vertices[(i + 1) % cell_vertices.size()];
        HalfEdge* he = dcel->CreateEdge(v1, v2);
        edges.push_back(he);
      }
      
      // Connect edges into a cycle
      for (size_t i = 0; i < edges.size(); ++i) {
        HalfEdge* he = edges[i];
        HalfEdge* next_he = edges[(i + 1) % edges.size()];
        dcel->ConnectHalfEdges(he, next_he);
      }
      
      // Create face
      Face* face = dcel->CreateFace();
      dcel->SetFaceOfCycle(edges[0], face);
    }
  }
  
  // Merge shared edges (set twin relationships)
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
  
  // Convert to Point2D vector
  std::vector<Point2D> polygon;
  for (auto* v : boundary_vertices) {
    polygon.push_back(v->GetCoordinates());
  }
  
  // Clip polygon using half-plane
  std::vector<Point2D> clipped_vertices = 
      HalfPlaneClipper::ClipPolygon(polygon, point_on_line, normal);
  
  // Rebuild the face with clipped polygon
  if (clipped_vertices.size() >= 3) {
    // Remove old half-edges of this face
    // (Simplified: we'll create new edges and update the face)
    
    // Create new vertices and edges for clipped polygon
    std::vector<Vertex*> new_vertices;
    std::vector<HalfEdge*> new_edges;
    
    for (const auto& p : clipped_vertices) {
      Vertex* v = dcel->CreateVertex(p);
      new_vertices.push_back(v);
    }
    
    // Create edges
    for (size_t i = 0; i < new_vertices.size(); ++i) {
      Vertex* v1 = new_vertices[i];
      Vertex* v2 = new_vertices[(i + 1) % new_vertices.size()];
      HalfEdge* he = dcel->CreateEdge(v1, v2);
      new_edges.push_back(he);
    }
    
    // Connect edges into a cycle
    for (size_t i = 0; i < new_edges.size(); ++i) {
      HalfEdge* he = new_edges[i];
      HalfEdge* next_he = new_edges[(i + 1) % new_edges.size()];
      dcel->ConnectHalfEdges(he, next_he);
    }
    
    // Update face to point to new cycle
    if (!new_edges.empty()) {
      dcel->SetFaceOfCycle(new_edges[0], face);
    }
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
      // Only add edge if it has a valid twin (to avoid duplicates)
      // Or if it doesn't have a twin, add it anyway
      HalfEdge* twin = he->GetTwin();
      if (twin == nullptr || he->GetId() < twin->GetId()) {
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
