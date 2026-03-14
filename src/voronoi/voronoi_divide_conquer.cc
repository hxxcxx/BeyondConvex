#include "voronoi_divide_conquer.h"
#include "dcel_helper.h"
#include <map>
#include <set>
#include <algorithm>
#include <iostream>

namespace geometry {

VoronoiDiagramResult DivideConquerVoronoi::Generate(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) {
  
  // Generate DCEL first, then convert
  DCEL* dcel = GenerateDCEL(sites, bounds);
  VoronoiDiagramResult result = DCELHelper::ConvertDCELToResult(dcel, sites);
  delete dcel;
  
  return result;
}

DCEL* DivideConquerVoronoi::GenerateDCEL(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) {
  
  DCEL* dcel = new DCEL();
  
  if (sites.empty()) {
    return dcel;
  }
  
  if (sites.size() == 1) {
    // Single site: entire bounding box is one cell
    DCELHelper::CreateBoundingBoxCell(dcel, bounds);
    return dcel;
  }
  
  // Sort sites by x-coordinate
  std::vector<Point2D> sorted_sites = sites;
  std::sort(sorted_sites.begin(), sorted_sites.end(),
      [](const Point2D& a, const Point2D& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
      });
  
  // Use divide and conquer
  dcel = DivideAndConquer(sorted_sites, bounds);
  
  return dcel;
}

DCEL* DivideConquerVoronoi::DivideAndConquer(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) const {
  
  return DivideAndConquerWithDepth(sites, bounds, 0);
}

DCEL* DivideConquerVoronoi::DivideAndConquerWithDepth(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds,
    int depth) const {
  
  std::string indent(depth * 2, ' ');
  std::cout << indent << "[DivideAndConquer] n=" << sites.size() << ", depth=" << depth << std::endl;
  
  DCEL* dcel = new DCEL();
  
  size_t n = sites.size();
  
  // Base case: small number of sites, compute directly
  if (n == 1) {
    std::cout << indent << "[Base case n=1] Creating single cell" << std::endl;
    // Single site: entire bounding box is one cell
    DCELHelper::CreateBoundingBoxCell(dcel, bounds);
    return dcel;
  }
  
  if (n == 2) {
    std::cout << indent << "[Base case n=2] Sites: (" 
              << sites[0].x << "," << sites[0].y << ") and ("
              << sites[1].x << "," << sites[1].y << ")" << std::endl;
    
    // Two sites: create two cells and clip each by the other's bisector
    std::vector<Face*> cells;
    for (size_t i = 0; i < 2; ++i) {
      Face* cell = DCELHelper::CreateBoundingBoxCell(dcel, bounds);
      cells.push_back(cell);
    }
    
    // Clip each cell by the other site's bisector
    for (size_t i = 0; i < 2; ++i) {
      Point2D midpoint;
      Vector2D normal;
      HalfPlaneClipper::ComputeBisector(sites[i], sites[1-i], midpoint, normal);
      
      DCELHelper::ClipFaceByHalfPlane(dcel, cells[i], midpoint, normal);
    }
    
    // Merge shared edges
    DCELHelper::MergeSharedEdges(dcel);
    
    std::cout << indent << "[Base case n=2] Done, faces=" << dcel->GetFaceCount() << std::endl;
    return dcel;
  }
  
  if (n == 3) {
    std::cout << indent << "[Base case n=3] Sites: ";
    for (size_t i = 0; i < 3; ++i) {
      std::cout << "(" << sites[i].x << "," << sites[i].y << ") ";
    }
    std::cout << std::endl;
    
    // Three sites: create three cells and clip each by the other sites' bisectors
    std::vector<Face*> cells;
    for (size_t i = 0; i < 3; ++i) {
      Face* cell = DCELHelper::CreateBoundingBoxCell(dcel, bounds);
      cells.push_back(cell);
    }
    
    // Clip each cell by the other sites' bisectors
    for (size_t i = 0; i < 3; ++i) {
      for (size_t j = 0; j < 3; ++j) {
        if (i == j) continue;
        
        Point2D midpoint;
        Vector2D normal;
        HalfPlaneClipper::ComputeBisector(sites[i], sites[j], midpoint, normal);
        
        DCELHelper::ClipFaceByHalfPlane(dcel, cells[i], midpoint, normal);
      }
    }
    
    // Merge shared edges
    DCELHelper::MergeSharedEdges(dcel);
    
    std::cout << indent << "[Base case n=3] Done, faces=" << dcel->GetFaceCount() << std::endl;
    return dcel;
  }
  
  // Divide: split sites into two halves
  size_t mid = n / 2;
  std::vector<Point2D> left_sites(sites.begin(), sites.begin() + mid);
  std::vector<Point2D> right_sites(sites.begin() + mid, sites.end());
  
  std::cout << indent << "[Divide] Split at x=" << sites[mid].x 
            << ", left=" << left_sites.size() << ", right=" << right_sites.size() << std::endl;
  std::cout << indent << "[Divide] Left sites: ";
  for (const auto& p : left_sites) {
    std::cout << "(" << p.x << "," << p.y << ") ";
  }
  std::cout << std::endl;
  std::cout << indent << "[Divide] Right sites: ";
  for (const auto& p : right_sites) {
    std::cout << "(" << p.x << "," << p.y << ") ";
  }
  std::cout << std::endl;
  
  // Conquer: recursively compute Voronoi diagrams
  DCEL* left_dcel = DivideAndConquerWithDepth(left_sites, bounds, depth + 1);
  DCEL* right_dcel = DivideAndConquerWithDepth(right_sites, bounds, depth + 1);
  
  std::cout << indent << "[Conquer] Left faces=" << left_dcel->GetFaceCount() 
            << ", Right faces=" << right_dcel->GetFaceCount() << std::endl;
  
  // Merge: combine the two diagrams
  dcel = MergeDiagrams(left_dcel, right_dcel, left_sites, right_sites, bounds);
  
  std::cout << indent << "[Merge] Done, merged faces=" << dcel->GetFaceCount() << std::endl;
  
  // Clean up
  delete left_dcel;
  delete right_dcel;
  
  return dcel;
}

DCEL* DivideConquerVoronoi::MergeDiagrams(
    DCEL* left_dcel,
    DCEL* right_dcel,
    const std::vector<Point2D>& left_sites,
    const std::vector<Point2D>& right_sites,
    const VoronoiBounds& bounds) const {
  
  std::cout << "  [MergeDiagrams] Starting merge, left_faces=" << left_dcel->GetFaceCount()
            << ", right_faces=" << right_dcel->GetFaceCount() << std::endl;
  
  DCEL* merged_dcel = new DCEL();
  
  // Copy all faces from left and right diagrams
  std::map<Point2D, Vertex*> vertex_map;
  
  // Copy left diagram
  DCELHelper::CopyFaces(left_dcel, merged_dcel, vertex_map);
  std::cout << "  [MergeDiagrams] Copied left faces, total=" << merged_dcel->GetFaceCount() << std::endl;
  
  // Copy right diagram
  DCELHelper::CopyFaces(right_dcel, merged_dcel, vertex_map);
  std::cout << "  [MergeDiagrams] Copied right faces, total=" << merged_dcel->GetFaceCount() << std::endl;
  
  // Clip faces by dividing curve
  std::cout << "  [MergeDiagrams] Clipping faces by dividing curve..." << std::endl;
  ClipFacesByDividingCurve(merged_dcel, left_sites, right_sites);
  std::cout << "  [MergeDiagrams] After clipping, faces=" << merged_dcel->GetFaceCount() << std::endl;
  
  // Merge shared edges
  DCELHelper::MergeSharedEdges(merged_dcel);
  
  return merged_dcel;
}

void DivideConquerVoronoi::ClipFacesByDividingCurve(
    DCEL* dcel,
    const std::vector<Point2D>& left_sites,
    const std::vector<Point2D>& right_sites) const {
  
  std::cout << "    [ClipFaces] Processing " << dcel->GetFaceCount() << " faces" << std::endl;
  std::cout << "    [ClipFaces] Before clipping: " << dcel->GetFaceCount() 
            << " faces, " << dcel->GetVertexCount() << " vertices, " 
            << dcel->GetHalfEdgeCount() << " half-edges" << std::endl;
  
  // Combine all sites
  std::vector<Point2D> all_sites = left_sites;
  all_sites.insert(all_sites.end(), right_sites.begin(), right_sites.end());
  
  // For each face, determine which side it's on and clip accordingly
  for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
    Face* face = dcel->GetFace(i);
    if (face == nullptr || face->IsUnbounded()) continue;
    
    auto boundary_vertices = face->GetOuterBoundaryVertices();
    if (boundary_vertices.size() < 3) continue;
    
    // Compute face center
    Point2D face_center = DCELHelper::ComputeFaceCenter(face);
    
    // Find the closest site among ALL sites
    Point2D closest_site = DCELHelper::FindClosestSite(face_center, all_sites);
    
    // Determine which side the closest site belongs to
    bool is_left_site = false;
    for (const auto& site : left_sites) {
      if (site.x == closest_site.x && site.y == closest_site.y) {
        is_left_site = true;
        break;
      }
    }
    
    // Clip by sites on the OTHER side
    std::vector<Point2D> clip_points;
    std::vector<Vector2D> clip_normals;
    
    if (is_left_site) {
      // Closest site is on left, clip by right sites
      std::cout << "      [Face " << i << "] Center=(" << face_center.x << "," << face_center.y 
                << "), closest is LEFT (" << closest_site.x << "," << closest_site.y 
                << "), clipping by " << right_sites.size() << " right sites" << std::endl;
      for (const auto& site : right_sites) {
        Point2D midpoint;
        Vector2D normal;
        HalfPlaneClipper::ComputeBisector(closest_site, site, midpoint, normal);
        clip_points.push_back(midpoint);
        clip_normals.push_back(normal);
      }
    } else {
      // Closest site is on right, clip by left sites
      std::cout << "      [Face " << i << "] Center=(" << face_center.x << "," << face_center.y 
                << "), closest is RIGHT (" << closest_site.x << "," << closest_site.y 
                << "), clipping by " << left_sites.size() << " left sites" << std::endl;
      for (const auto& site : left_sites) {
        Point2D midpoint;
        Vector2D normal;
        HalfPlaneClipper::ComputeBisector(closest_site, site, midpoint, normal);
        clip_points.push_back(midpoint);
        clip_normals.push_back(normal);
      }
    }
    
    // Clip by all half-planes at once
    if (!clip_points.empty()) {
      Face* result_face = DCELHelper::ClipFaceByMultipleHalfPlanes(dcel, face, clip_points, clip_normals);
      if (result_face == nullptr) {
        std::cout << "      [Face " << i << "] was completely clipped away!" << std::endl;
      } else {
        auto final_vertices = result_face->GetOuterBoundaryVertices();
        std::cout << "      [Face " << i << "] After clipping: " << final_vertices.size() << " vertices" << std::endl;
      }
    }
  }
  
  std::cout << "    [ClipFaces] After clipping: " << dcel->GetFaceCount() 
            << " faces, " << dcel->GetVertexCount() << " vertices, " 
            << dcel->GetHalfEdgeCount() << " half-edges" << std::endl;
}

}  // namespace geometry
