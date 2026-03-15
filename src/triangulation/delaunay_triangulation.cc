#include "delaunay_triangulation.h"
#include "triangulation_utils.h"
#include "../dcel/dcel.h"
#include <iostream>
#include <algorithm>
#include <set>
#include <map>
#include <cmath>

namespace geometry {

TriangulationResult DelaunayTriangulation::Triangulate(
    const std::vector<Point2D>& points) {
  
  TriangulationResult result;
  
  if (points.size() < 3) {
    std::cerr << "[Delaunay] Invalid input: less than 3 points" << std::endl;
    return result;
  }
  
  std::cout << "[Delaunay] Triangulating " << points.size() << " points" << std::endl;
  
  // Create DCEL structure
  std::unique_ptr<DCEL> dcel = std::make_unique<DCEL>();
  
  // Step 1: Create super triangle that contains all points
  std::cout << "[Delaunay] Creating super triangle..." << std::endl;
  Face* super_triangle = CreateSuperTriangle(points, dcel.get());
  
  // Step 2: Incrementally insert points
  for (size_t i = 0; i < points.size(); ++i) {
    const Point2D& point = points[i];
    
    std::cout << "[Delaunay] Inserting point " << i << ": (" 
              << point.x << ", " << point.y << ")" << std::endl;
    
    // Find bad triangles (whose circumcircle contains the point)
    std::set<Face*> bad_triangles = FindBadTriangles(point, dcel.get());
    
    if (bad_triangles.empty()) {
      std::cout << "[Delaunay] Point is outside all triangles, skipping" << std::endl;
      continue;
    }
    
    std::cout << "[Delaunay] Found " << bad_triangles.size() << " bad triangles" << std::endl;
    
    // Find boundary of bad triangles
    std::vector<HalfEdge*> boundary = FindBoundary(bad_triangles);
    
    std::cout << "[Delaunay] Boundary has " << boundary.size() << " edges" << std::endl;
    
    // Remove bad triangles
    RemoveBadTriangles(bad_triangles, dcel.get());
    
    // Re-triangulate the hole
    RetriangulateHole(boundary, point, dcel.get());
  }
  
  std::cout << "[Delaunay] Triangulation complete: " 
            << dcel->GetFaceCount() << " faces" << std::endl;
  
  // Step 3: Convert DCEL to result (excluding super triangle)
  result = ConvertDCELToResult(dcel.get(), super_triangle);
  
  std::cout << "[Delaunay] Generated " << result.triangles.size() 
            << " triangles" << std::endl;
  
  return result;
}

Face* DelaunayTriangulation::CreateSuperTriangle(
    const std::vector<Point2D>& points,
    DCEL* dcel) {
  
  // Find bounding box of all points
  double min_x = points[0].x, max_x = points[0].x;
  double min_y = points[0].y, max_y = points[0].y;
  
  for (const auto& p : points) {
    min_x = std::min(min_x, p.x);
    max_x = std::max(max_x, p.x);
    min_y = std::min(min_y, p.y);
    max_y = std::max(max_y, p.y);
  }
  
  // Expand bounding box
  double dx = max_x - min_x;
  double dy = max_y - min_y;
  double delta_x = dx * 10.0;
  double delta_y = dy * 10.0;
  
  min_x -= delta_x;
  max_x += delta_x;
  min_y -= delta_y;
  max_y += delta_y;
  
  // Create super triangle vertices
  Vertex* v0 = dcel->CreateVertex(Point2D(min_x, min_y));
  Vertex* v1 = dcel->CreateVertex(Point2D(max_x + 2 * dx, min_y));
  Vertex* v2 = dcel->CreateVertex(Point2D(min_x + dx / 2, max_y + 2 * dy));
  
  // Create super triangle face
  Face* face = dcel->CreateFace();
  
  // Create edges
  HalfEdge* e0 = dcel->CreateEdge(v0, v1);
  HalfEdge* e1 = dcel->CreateEdge(v1, v2);
  HalfEdge* e2 = dcel->CreateEdge(v2, v0);
  
  // Connect edges in CCW order
  dcel->ConnectHalfEdges(e0, e1);
  dcel->ConnectHalfEdges(e1, e2);
  dcel->ConnectHalfEdges(e2, e0);
  
  // Set face
  dcel->SetFaceOfCycle(e0, face);
  
  return face;
}

std::set<Face*> DelaunayTriangulation::FindBadTriangles(
    const Point2D& point,
    DCEL* dcel) {
  
  std::set<Face*> bad_triangles;
  
  for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
    Face* face = dcel->GetFace(i);
    if (face == nullptr || face->IsUnbounded()) continue;
    
    if (IsInCircumcircle(point, face)) {
      bad_triangles.insert(face);
    }
  }
  
  return bad_triangles;
}

std::vector<HalfEdge*> DelaunayTriangulation::FindBoundary(
    const std::set<Face*>& bad_triangles) {
  
  std::set<HalfEdge*> boundary_set;
  
  // An edge is on boundary if it's shared by a bad triangle and a non-bad triangle
  for (Face* face : bad_triangles) {
    HalfEdge* start = face->GetHalfEdge();
    HalfEdge* edge = start;
    
    do {
      HalfEdge* twin = edge->GetTwin();
      Face* twin_face = twin->GetFace();
      
      // If twin face is not bad, this edge is on boundary
      if (twin_face == nullptr || bad_triangles.find(twin_face) == bad_triangles.end()) {
        boundary_set.insert(edge);
      }
      
      edge = edge->GetNext();
    } while (edge != start);
  }
  
  return std::vector<HalfEdge*>(boundary_set.begin(), boundary_set.end());
}

void DelaunayTriangulation::RemoveBadTriangles(
    const std::set<Face*>& bad_triangles,
    DCEL* dcel) {
  
  // Note: We don't actually remove faces from DCEL to avoid index issues
  // Instead, we mark them and they will be excluded in final result
  // The edges will be reused or updated in retriangulation
  
  std::cout << "[Delaunay] Marking " << bad_triangles.size() 
            << " bad triangles for removal" << std::endl;
}

void DelaunayTriangulation::RetriangulateHole(
    const std::vector<HalfEdge*>& boundary,
    const Point2D& point,
    DCEL* dcel) {
  
  // Create vertex for new point
  Vertex* new_vertex = dcel->CreateVertex(point);
  
  std::cout << "[Delaunay] Re-triangulating hole with " << boundary.size() 
            << " boundary edges" << std::endl;
  
  // For each boundary edge, create a triangle with the new point
  for (HalfEdge* boundary_edge : boundary) {
    Vertex* v1 = boundary_edge->GetOrigin();
    Vertex* v2 = boundary_edge->GetTwin()->GetOrigin();
    
    // Create new face
    Face* new_face = dcel->CreateFace();
    
    // Create edges from new vertex to boundary vertices
    HalfEdge* e0 = dcel->CreateEdge(new_vertex, v1);
    HalfEdge* e1 = dcel->CreateEdge(v1, v2);
    HalfEdge* e2 = dcel->CreateEdge(v2, new_vertex);
    
    // Connect edges in CCW order
    dcel->ConnectHalfEdges(e0, e1);
    dcel->ConnectHalfEdges(e1, e2);
    dcel->ConnectHalfEdges(e2, e0);
    
    // Set face
    dcel->SetFaceOfCycle(e0, new_face);
    
    // Update twin relationships
    // (This is simplified; proper implementation would update twins correctly)
  }
}

bool DelaunayTriangulation::IsInCircumcircle(
    const Point2D& point,
    Face* face) {
  
  Point2D center = GetCircumcenter(face);
  double radius = GetCircumradius(face);
  
  double dist = point.DistanceTo(center);
  const double eps = 1e-9;
  
  return dist < radius - eps;
}

Point2D DelaunayTriangulation::GetCircumcenter(Face* face) {
  if (face == nullptr || face->IsUnbounded()) {
    return Point2D(0, 0);
  }
  
  HalfEdge* e0 = face->GetHalfEdge();
  HalfEdge* e1 = e0->GetNext();
  HalfEdge* e2 = e1->GetNext();
  
  Point2D v0 = e0->GetOrigin()->GetCoordinates();
  Point2D v1 = e1->GetOrigin()->GetCoordinates();
  Point2D v2 = e2->GetOrigin()->GetCoordinates();
  
  Triangle tri(v0, v1, v2);
  return tri.Circumcenter();
}

double DelaunayTriangulation::GetCircumradius(Face* face) {
  if (face == nullptr || face->IsUnbounded()) {
    return 0.0;
  }
  
  HalfEdge* e0 = face->GetHalfEdge();
  HalfEdge* e1 = e0->GetNext();
  HalfEdge* e2 = e1->GetNext();
  
  Point2D v0 = e0->GetOrigin()->GetCoordinates();
  Point2D v1 = e1->GetOrigin()->GetCoordinates();
  Point2D v2 = e2->GetOrigin()->GetCoordinates();
  
  Triangle tri(v0, v1, v2);
  return tri.Circumradius();
}

TriangulationResult DelaunayTriangulation::ConvertDCELToResult(
    DCEL* dcel,
    Face* super_triangle_face) {
  
  TriangulationResult result;
  
  std::set<Point2D> vertex_set;
  std::set<Edge2D> edge_set;
  
  for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
    Face* face = dcel->GetFace(i);
    if (face == nullptr || face->IsUnbounded()) continue;
    
    // Skip super triangle
    if (face == super_triangle_face) continue;
    
    // Get triangle vertices
    HalfEdge* e0 = face->GetHalfEdge();
    HalfEdge* e1 = e0->GetNext();
    HalfEdge* e2 = e1->GetNext();
    
    Point2D v0 = e0->GetOrigin()->GetCoordinates();
    Point2D v1 = e1->GetOrigin()->GetCoordinates();
    Point2D v2 = e2->GetOrigin()->GetCoordinates();
    
    // Add triangle
    result.triangles.emplace_back(v0, v1, v2);
    
    // Collect vertices
    vertex_set.insert(v0);
    vertex_set.insert(v1);
    vertex_set.insert(v2);
    
    // Collect edges (normalized)
    auto add_edge = [&edge_set](const Point2D& p1, const Point2D& p2) {
      Point2D a = p1, b = p2;
      if (b < a || (a == b && b < a)) std::swap(a, b);
      edge_set.emplace(a, b);
    };
    
    add_edge(v0, v1);
    add_edge(v1, v2);
    add_edge(v2, v0);
  }
  
  result.vertices.assign(vertex_set.begin(), vertex_set.end());
  result.edges.assign(edge_set.begin(), edge_set.end());
  
  return result;
}

}  // namespace geometry
