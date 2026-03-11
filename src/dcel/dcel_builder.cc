#include "dcel_builder.h"
#include <stdexcept>
#include <unordered_map>
#include <map>
#include <array>

namespace geometry {

Face* DCELBuilder::BuildPolygon(
    const std::vector<Point2D>& vertices,
    DCEL* dcel) {
  
  if (vertices.size() < 3) {
    throw std::invalid_argument("Polygon must have at least 3 vertices");
  }
  
  if (dcel == nullptr) {
    throw std::invalid_argument("DCEL cannot be null");
  }
  
  // Create vertices
  std::vector<Vertex*> vertex_ptrs;
  for (const auto& v : vertices) {
    vertex_ptrs.push_back(dcel->CreateVertex(v));
  }
  
  // Create half-edges for the polygon boundary
  std::vector<HalfEdge*> boundary_edges;
  for (size_t i = 0; i < vertex_ptrs.size(); ++i) {
    size_t next_i = (i + 1) % vertex_ptrs.size();
    HalfEdge* edge = dcel->CreateEdge(vertex_ptrs[i], vertex_ptrs[next_i]);
    boundary_edges.push_back(edge);
  }
  
  // Connect edges in CCW order
  for (size_t i = 0; i < boundary_edges.size(); ++i) {
    size_t next_i = (i + 1) % boundary_edges.size();
    dcel->ConnectHalfEdges(boundary_edges[i], boundary_edges[next_i]);
  }
  
  // Create face and set its outer component
  Face* face = dcel->CreateFace();
  face->SetOuterComponent(boundary_edges[0]);
  
  // Set face for all boundary edges
  dcel->SetFaceOfCycle(boundary_edges[0], face);
  
  return face;
}

Face* DCELBuilder::BuildPolygonWithHoles(
    const std::vector<Point2D>& outer_vertices,
    const std::vector<std::vector<Point2D>>& holes,
    DCEL* dcel) {
  
  // Build outer polygon
  Face* face = BuildPolygon(outer_vertices, dcel);
  
  // Add holes
  for (const auto& hole_vertices : holes) {
    if (hole_vertices.size() < 3) {
      continue;  // Skip invalid holes
    }
    
    // Create hole vertices
    std::vector<Vertex*> hole_vertex_ptrs;
    for (const auto& v : hole_vertices) {
      hole_vertex_ptrs.push_back(dcel->CreateVertex(v));
    }
    
    // Create half-edges for the hole boundary
    std::vector<HalfEdge*> hole_edges;
    for (size_t i = 0; i < hole_vertex_ptrs.size(); ++i) {
      size_t next_i = (i + 1) % hole_vertex_ptrs.size();
      HalfEdge* edge = dcel->CreateEdge(hole_vertex_ptrs[i], hole_vertex_ptrs[next_i]);
      hole_edges.push_back(edge);
    }
    
    // Connect edges in CW order (holes are oriented opposite to outer boundary)
    for (size_t i = 0; i < hole_edges.size(); ++i) {
      size_t next_i = (i + 1) % hole_edges.size();
      dcel->ConnectHalfEdges(hole_edges[i], hole_edges[next_i]);
    }
    
    // Add hole to face
    face->AddInnerComponent(hole_edges[0]);
    
    // Set face for all hole edges
    dcel->SetFaceOfCycle(hole_edges[0], face);
  }
  
  return face;
}

std::vector<Face*> DCELBuilder::BuildTriangulation(
    const std::vector<Point2D>& vertices,
    const std::vector<std::array<int, 3>>& triangles,
    DCEL* dcel) {
  
  if (dcel == nullptr) {
    throw std::invalid_argument("DCEL cannot be null");
  }
  
  // Create all vertices
  std::vector<Vertex*> vertex_ptrs;
  for (const auto& v : vertices) {
    vertex_ptrs.push_back(dcel->CreateVertex(v));
  }
  
  // Map to track created edges (avoid duplicates)
  std::map<std::pair<int, int>, HalfEdge*> edge_map;
  
  std::vector<Face*> faces;
  
  for (const auto& tri : triangles) {
    int i0 = tri[0], i1 = tri[1], i2 = tri[2];
    Vertex* v0 = vertex_ptrs[i0];
    Vertex* v1 = vertex_ptrs[i1];
    Vertex* v2 = vertex_ptrs[i2];
    
    // Create or get edges for this triangle
    HalfEdge* edge01, *edge12, *edge20;
    
    auto key01 = std::make_pair(i0, i1);
    auto key12 = std::make_pair(i1, i2);
    auto key20 = std::make_pair(i2, i0);
    
    if (edge_map.find(key01) != edge_map.end()) {
      edge01 = edge_map[key01]->GetTwin();  // Use existing edge's twin
    } else {
      edge01 = dcel->CreateEdge(v0, v1);
      edge_map[key01] = edge01;
    }
    
    if (edge_map.find(key12) != edge_map.end()) {
      edge12 = edge_map[key12]->GetTwin();
    } else {
      edge12 = dcel->CreateEdge(v1, v2);
      edge_map[key12] = edge12;
    }
    
    if (edge_map.find(key20) != edge_map.end()) {
      edge20 = edge_map[key20]->GetTwin();
    } else {
      edge20 = dcel->CreateEdge(v2, v0);
      edge_map[key20] = edge20;
    }
    
    // Connect edges in CCW order
    dcel->ConnectHalfEdges(edge01, edge12);
    dcel->ConnectHalfEdges(edge12, edge20);
    dcel->ConnectHalfEdges(edge20, edge01);
    
    // Create face
    Face* face = dcel->CreateFace();
    face->SetOuterComponent(edge01);
    dcel->SetFaceOfCycle(edge01, face);
    
    faces.push_back(face);
  }
  
  return faces;
}

Face* DCELBuilder::BuildBoundingBox(
    double min_x, double min_y,
    double max_x, double max_y,
    DCEL* dcel) {
  
  std::vector<Point2D> vertices = {
    Point2D(min_x, min_y),
    Point2D(max_x, min_y),
    Point2D(max_x, max_y),
    Point2D(min_x, max_y)
  };
  
  return BuildPolygon(vertices, dcel);
}

std::vector<Face*> DCELBuilder::BuildVoronoiDiagram(
    const std::vector<Point2D>& sites,
    const std::vector<Edge2D>& edges,
    double bounds_min_x, double bounds_min_y,
    double bounds_max_x, double bounds_max_y,
    DCEL* dcel) {
  
  // This is a complex operation that requires:
  // 1. Building Voronoi cells as polygons
  // 2. Handling unbounded cells
  // 3. Properly connecting edges
  
  // For now, return empty vector as placeholder
  // TODO: Implement this properly
  return {};
}

Vertex* DCELBuilder::FindOrCreateVertex(
    const Point2D& point,
    std::unordered_map<Point2D, Vertex*, std::function<size_t(const Point2D&)>>& vertex_map,
    DCEL* dcel) {
  
  auto it = vertex_map.find(point);
  if (it != vertex_map.end()) {
    return it->second;
  }
  
  Vertex* vertex = dcel->CreateVertex(point);
  vertex_map[point] = vertex;
  return vertex;
}

void DCELBuilder::ConnectCycle(
    const std::vector<HalfEdge*>& edges,
    DCEL* dcel) {
  
  for (size_t i = 0; i < edges.size(); ++i) {
    size_t next_i = (i + 1) % edges.size();
    dcel->ConnectHalfEdges(edges[i], edges[next_i]);
  }
}

}  // namespace geometry
