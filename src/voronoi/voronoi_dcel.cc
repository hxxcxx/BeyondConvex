#include "voronoi_dcel.h"
#include <map>
#include <set>
#include <algorithm>
#include <iostream>

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
    Face* cell = CreateBoundingBoxCell(dcel, sites[0], bounds);
    return dcel;
  }
  
  // Create initial bounding box cell for each site
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
      
      // Get current cell vertices
      auto boundary_vertices = cells[i]->GetOuterBoundaryVertices();
      
      // Convert to Point2D vector
      std::vector<Point2D> polygon;
      for (auto* v : boundary_vertices) {
        polygon.push_back(v->GetCoordinates());
      }
      
      // Clip polygon using half-plane
      std::vector<Point2D> clipped_vertices = 
          HalfPlaneClipper::ClipPolygon(polygon, midpoint, normal);
      
      // Only update if we have a valid polygon
      if (clipped_vertices.size() >= 3) {
        // Create new vertices
        std::vector<Vertex*> new_vertices;
        for (const auto& p : clipped_vertices) {
          new_vertices.push_back(dcel->CreateVertex(p));
        }
        
        // Create new edges
        std::vector<HalfEdge*> new_edges;
        for (size_t k = 0; k < new_vertices.size(); ++k) {
          Vertex* v1 = new_vertices[k];
          Vertex* v2 = new_vertices[(k + 1) % new_vertices.size()];
          HalfEdge* he = dcel->CreateEdge(v1, v2);
          new_edges.push_back(he);
        }
        
        // Connect edges into a cycle
        for (size_t k = 0; k < new_edges.size(); ++k) {
          HalfEdge* he = new_edges[k];
          HalfEdge* next_he = new_edges[(k + 1) % new_edges.size()];
          dcel->ConnectHalfEdges(he, next_he);
        }
        
        // Update cell to point to new cycle
        dcel->SetFaceOfCycle(new_edges[0], cells[i]);
        cells[i]->SetOuterComponent(new_edges[0]);
      }
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
  face->SetOuterComponent(he0);  // ← 关键修复
  
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

Face* DCELVoronoi::FindFaceContainingPoint(
    DCEL* dcel,
    const Point2D& point,
    const std::map<Face*, Point2D>& face_sites) const {
  
  // Simple approach: check all faces
  for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
    Face* face = dcel->GetFace(i);
    if (face == nullptr || face->IsUnbounded()) {
      continue;
    }
    
    // Check if point is inside this face
    auto boundary_vertices = face->GetOuterBoundaryVertices();
    if (boundary_vertices.size() < 3) {
      continue;
    }
    
    // Use ray casting algorithm to check if point is inside polygon
    bool inside = false;
    for (size_t j = 0; j < boundary_vertices.size(); ++j) {
      Vertex* v1 = boundary_vertices[j];
      Vertex* v2 = boundary_vertices[(j + 1) % boundary_vertices.size()];
      
      double x1 = v1->GetCoordinates().x;
      double y1 = v1->GetCoordinates().y;
      double x2 = v2->GetCoordinates().x;
      double y2 = v2->GetCoordinates().y;
      
      // Check if ray from point to right intersects edge
      if (((y1 > point.y) != (y2 > point.y)) &&
          (point.x < (x2 - x1) * (point.y - y1) / (y2 - y1) + x1)) {
        inside = !inside;
      }
    }
    
    if (inside) {
      return face;
    }
  }
  
  return nullptr;
}

Face* DCELVoronoi::SplitFaceByLine(
    DCEL* dcel,
    Face* face,
    const Point2D& point_on_line,
    const Vector2D& normal) const {
  
  // Get current boundary vertices
  auto boundary_vertices = face->GetOuterBoundaryVertices();
  
  if (boundary_vertices.size() < 3) {
    return nullptr;
  }
  
  // Convert to Point2D vector
  std::vector<Point2D> polygon;
  for (auto* v : boundary_vertices) {
    polygon.push_back(v->GetCoordinates());
  }
  
  // Clip polygon by half-plane (keep positive side)
  std::vector<Point2D> positive_vertices = 
      HalfPlaneClipper::ClipPolygon(polygon, point_on_line, normal);
  
  // Clip polygon by opposite half-plane (keep negative side)
  std::vector<Point2D> negative_vertices = 
      HalfPlaneClipper::ClipPolygon(polygon, point_on_line, -normal);
  
  // Check if both sides are valid
  if (positive_vertices.size() < 3 || negative_vertices.size() < 3) {
    return nullptr;
  }
  
  // Create new vertices and edges for positive side
  std::vector<Vertex*> pos_vertices;
  std::vector<HalfEdge*> pos_edges;
  
  for (const auto& p : positive_vertices) {
    pos_vertices.push_back(dcel->CreateVertex(p));
  }
  
  for (size_t i = 0; i < pos_vertices.size(); ++i) {
    Vertex* v1 = pos_vertices[i];
    Vertex* v2 = pos_vertices[(i + 1) % pos_vertices.size()];
    HalfEdge* he = dcel->CreateEdge(v1, v2);
    pos_edges.push_back(he);
  }
  
  // Connect edges into a cycle
  for (size_t i = 0; i < pos_edges.size(); ++i) {
    HalfEdge* he = pos_edges[i];
    HalfEdge* next_he = pos_edges[(i + 1) % pos_edges.size()];
    dcel->ConnectHalfEdges(he, next_he);
  }
  
  // Create new face for positive side
  Face* new_face = dcel->CreateFace();
  dcel->SetFaceOfCycle(pos_edges[0], new_face);
  
  // Update original face to point to negative side
  std::vector<Vertex*> neg_vertices;
  std::vector<HalfEdge*> neg_edges;
  
  for (const auto& p : negative_vertices) {
    neg_vertices.push_back(dcel->CreateVertex(p));
  }
  
  for (size_t i = 0; i < neg_vertices.size(); ++i) {
    Vertex* v1 = neg_vertices[i];
    Vertex* v2 = neg_vertices[(i + 1) % neg_vertices.size()];
    HalfEdge* he = dcel->CreateEdge(v1, v2);
    neg_edges.push_back(he);
  }
  
  // Connect edges into a cycle
  for (size_t i = 0; i < neg_edges.size(); ++i) {
    HalfEdge* he = neg_edges[i];
    HalfEdge* next_he = neg_edges[(i + 1) % neg_edges.size()];
    dcel->ConnectHalfEdges(he, next_he);
  }
  
  // Update original face
  dcel->SetFaceOfCycle(neg_edges[0], face);
  
  return new_face;
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
  
  // Collect vertices and edges from valid faces only
  std::set<Point2D> vertex_set;
  std::set<Edge2D> edge_set;
  
  for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
    Face* face = dcel->GetFace(i);
    if (face == nullptr || face->IsUnbounded()) {
      continue;
    }
    
    // Get boundary vertices for this face
    auto boundary_vertices = face->GetOuterBoundaryVertices();
    
    if (boundary_vertices.size() < 3) {
      continue;
    }
    
    // Collect vertices
    for (auto* v : boundary_vertices) {
      if (v) {
        vertex_set.insert(v->GetCoordinates());
      }
    }
    
    // Collect edges
    for (size_t j = 0; j < boundary_vertices.size(); ++j) {
      Vertex* v1 = boundary_vertices[j];
      Vertex* v2 = boundary_vertices[(j + 1) % boundary_vertices.size()];
      
      if (v1 && v2) {
        edge_set.emplace(v1->GetCoordinates(), v2->GetCoordinates());
      }
    }
  }
  
  // Convert sets to vectors
  for (const auto& v : vertex_set) {
    result.vertices.push_back(v);
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
      
      if (boundary_vertices.size() < 3) {
        continue;
      }
      
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
