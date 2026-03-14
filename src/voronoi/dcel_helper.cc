#include "dcel_helper.h"
#include <algorithm>

namespace geometry {

Face* DCELHelper::CreateBoundingBoxCell(
    DCEL* dcel,
    const VoronoiBounds& bounds) {
  
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
  face->SetOuterComponent(he0);
  
  return face;
}

Face* DCELHelper::CreatePolygonCell(
    DCEL* dcel,
    const std::vector<Point2D>& vertices) {
  
  if (vertices.size() < 3) {
    return nullptr;
  }
  
  // Create vertices
  std::vector<Vertex*> vertex_ptrs;
  for (const auto& p : vertices) {
    vertex_ptrs.push_back(dcel->CreateVertex(p));
  }
  
  // Create edges
  std::vector<HalfEdge*> edges;
  for (size_t i = 0; i < vertex_ptrs.size(); ++i) {
    Vertex* v1 = vertex_ptrs[i];
    Vertex* v2 = vertex_ptrs[(i + 1) % vertex_ptrs.size()];
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
  face->SetOuterComponent(edges[0]);
  
  return face;
}

Face* DCELHelper::ClipFaceByHalfPlane(
    DCEL* dcel,
    Face* face,
    const Point2D& point_on_line,
    const Vector2D& normal) {
  
  // Get current boundary vertices
  auto boundary_vertices = face->GetOuterBoundaryVertices();
  
  if (boundary_vertices.empty()) {
    std::cout << "      [ClipFaceByHalfPlane] Face has no boundary, skipping" << std::endl;
    return face;
  }
  
  // Convert to Point2D vector
  std::vector<Point2D> polygon;
  for (auto* v : boundary_vertices) {
    polygon.push_back(v->GetCoordinates());
  }
  
  std::cout << "      [ClipFaceByHalfPlane] Before clip: " << polygon.size() << " vertices" << std::endl;
  std::cout << "      [ClipFaceByHalfPlane] Clip line: point=(" << point_on_line.x << "," << point_on_line.y 
            << "), normal=(" << normal.x << "," << normal.y << ")" << std::endl;
  
  // Clip polygon using half-plane
  std::vector<Point2D> clipped_vertices = 
      HalfPlaneClipper::ClipPolygon(polygon, point_on_line, normal);
  
  std::cout << "      [ClipFaceByHalfPlane] After clip: " << clipped_vertices.size() << " vertices" << std::endl;
  
  if (clipped_vertices.size() != polygon.size()) {
    std::cout << "      [ClipFaceByHalfPlane] Vertex count changed: " 
              << polygon.size() << " -> " << clipped_vertices.size() << std::endl;
  }
  
  if (clipped_vertices.size() <= 3) {
    std::cout << "      [ClipFaceByHalfPlane] WARNING: Face has only " 
              << clipped_vertices.size() << " vertices after clipping!" << std::endl;
    for (const auto& v : clipped_vertices) {
      std::cout << "        (" << v.x << ", " << v.y << ")" << std::endl;
    }
  }
  
  // Rebuild the face with clipped polygon
  if (clipped_vertices.size() >= 3) {
    // Create new vertices and edges for clipped polygon
    std::vector<Vertex*> new_vertices;
    std::vector<HalfEdge*> new_edges;
    
    for (const auto& p : clipped_vertices) {
      new_vertices.push_back(dcel->CreateVertex(p));
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
    dcel->SetFaceOfCycle(new_edges[0], face);
    face->SetOuterComponent(new_edges[0]);
    
    return face;
  }
  
  // Face was completely clipped away - return nullptr to indicate it should be removed
  std::cout << "      [ClipFaceByHalfPlane] Face completely clipped away (only " 
            << clipped_vertices.size() << " vertices)" << std::endl;
  return nullptr;
}

void DCELHelper::MergeSharedEdges(DCEL* dcel) {
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

VoronoiDiagramResult DCELHelper::ConvertDCELToResult(
    const DCEL* dcel,
    const std::vector<Point2D>& sites) {
  
  VoronoiDiagramResult result;
  result.sites = sites;
  
  std::cout << "[ConvertDCELToResult] Total faces in DCEL: " << dcel->GetFaceCount() << std::endl;
  
  // Collect vertices and edges from valid faces only
  std::set<Point2D> vertex_set;
  std::set<Edge2D> edge_set;
  
  size_t valid_face_count = 0;
  size_t invalid_face_count = 0;
  
  for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
    Face* face = dcel->GetFace(i);
    if (face == nullptr || face->IsUnbounded()) {
      invalid_face_count++;
      continue;
    }
    
    // Get boundary vertices for this face
    auto boundary_vertices = face->GetOuterBoundaryVertices();
    
    if (boundary_vertices.size() < 3) {
      std::cout << "[ConvertDCELToResult] Face " << i << " has only " 
                << boundary_vertices.size() << " vertices, skipping" << std::endl;
      invalid_face_count++;
      continue;
    }
    
    valid_face_count++;
    
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
  
  std::cout << "[ConvertDCELToResult] Valid faces: " << valid_face_count 
            << ", Invalid faces: " << invalid_face_count << std::endl;
  
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
    if (face == nullptr || face->IsUnbounded()) continue;
    
    auto boundary_vertices = face->GetOuterBoundaryVertices();
    if (boundary_vertices.size() < 3) continue;
    
    VoronoiCell cell;
    
    // Find the closest site for this cell
    Point2D face_center = ComputeFaceCenter(face);
    cell.site = FindClosestSite(face_center, sites);
    
    // Find site index
    for (size_t j = 0; j < sites.size(); ++j) {
      if (sites[j].x == cell.site.x && sites[j].y == cell.site.y) {
        cell.site_index = j;
        break;
      }
    }
    
    // Collect vertices
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
  
  std::cout << "[ConvertDCELToResult] Generated " << result.cells.size() 
            << " cells for " << sites.size() << " sites" << std::endl;
  
  // Check if multiple cells map to the same site
  std::map<Point2D, int> site_cell_count;
  for (const auto& cell : result.cells) {
    site_cell_count[cell.site]++;
  }
  
  bool has_duplicate = false;
  for (const auto& [site, count] : site_cell_count) {
    if (count > 1) {
      std::cout << "[ConvertDCELToResult] WARNING: Site (" << site.x << "," << site.y 
                << ") has " << count << " cells!" << std::endl;
      has_duplicate = true;
    }
  }
  
  if (has_duplicate) {
    std::cout << "[ConvertDCELToResult] ERROR: Multiple cells map to the same site!" << std::endl;
  }
  
  return result;
}

void DCELHelper::CopyFaces(
    DCEL* src_dcel,
    DCEL* dst_dcel,
    std::map<Point2D, Vertex*>& vertex_map) {
  
  for (size_t i = 0; i < src_dcel->GetFaceCount(); ++i) {
    Face* face = src_dcel->GetFace(i);
    if (face == nullptr || face->IsUnbounded()) continue;
    
    auto boundary_vertices = face->GetOuterBoundaryVertices();
    if (boundary_vertices.size() < 3) continue;
    
    // Create or get vertices
    std::vector<Vertex*> new_vertices;
    for (auto* v : boundary_vertices) {
      Point2D p = v->GetCoordinates();
      if (vertex_map.find(p) == vertex_map.end()) {
        vertex_map[p] = dst_dcel->CreateVertex(p);
      }
      new_vertices.push_back(vertex_map[p]);
    }
    
    // Create edges
    std::vector<HalfEdge*> new_edges;
    for (size_t j = 0; j < new_vertices.size(); ++j) {
      Vertex* v1 = new_vertices[j];
      Vertex* v2 = new_vertices[(j + 1) % new_vertices.size()];
      HalfEdge* he = dst_dcel->CreateEdge(v1, v2);
      new_edges.push_back(he);
    }
    
    // Connect edges into a cycle
    for (size_t j = 0; j < new_edges.size(); ++j) {
      HalfEdge* he = new_edges[j];
      HalfEdge* next_he = new_edges[(j + 1) % new_edges.size()];
      dst_dcel->ConnectHalfEdges(he, next_he);
    }
    
    // Create face
    Face* new_face = dst_dcel->CreateFace();
    dst_dcel->SetFaceOfCycle(new_edges[0], new_face);
    new_face->SetOuterComponent(new_edges[0]);
  }
}

Face* DCELHelper::ClipFaceByMultipleHalfPlanes(
    DCEL* dcel,
    Face* face,
    const std::vector<Point2D>& clip_points,
    const std::vector<Vector2D>& clip_normals) {
  
  if (clip_points.size() != clip_normals.size()) {
    return face;
  }
  
  Face* current_face = face;
  
  for (size_t i = 0; i < clip_points.size(); ++i) {
    Face* result = ClipFaceByHalfPlane(
        dcel, current_face, clip_points[i], clip_normals[i]);
    
    if (result == nullptr) {
      std::cout << "      [ClipFaceByMultipleHalfPlanes] Face completely clipped away at step " 
                << i << "/" << clip_points.size() << std::endl;
      return nullptr;
    }
    
    current_face = result;
  }
  
  return current_face;
}

Point2D DCELHelper::ComputeFaceCenter(Face* face) {
  if (face == nullptr || face->IsUnbounded()) {
    return Point2D(0, 0);
  }
  
  auto boundary_vertices = face->GetOuterBoundaryVertices();
  if (boundary_vertices.empty()) {
    return Point2D(0, 0);
  }
  
  Point2D center(0, 0);
  for (auto* v : boundary_vertices) {
    center.x += v->GetCoordinates().x;
    center.y += v->GetCoordinates().y;
  }
  
  center.x /= boundary_vertices.size();
  center.y /= boundary_vertices.size();
  
  return center;
}

Point2D DCELHelper::FindClosestSite(
    const Point2D& face_center,
    const std::vector<Point2D>& sites) {
  
  if (sites.empty()) {
    return Point2D(0, 0);
  }
  
  Point2D closest = sites[0];
  double min_dist = (face_center - sites[0]).LengthSquared();
  
  for (const auto& site : sites) {
    double dist = (face_center - site).LengthSquared();
    if (dist < min_dist) {
      min_dist = dist;
      closest = site;
    }
  }
  
  return closest;
}

}  // namespace geometry
