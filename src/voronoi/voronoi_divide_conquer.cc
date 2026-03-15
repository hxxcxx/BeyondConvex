#include "voronoi_divide_conquer.h"
#include "dcel_helper.h"
#include "../dcel/dcel.h"
#include <map>
#include <set>
#include <algorithm>
#include <iostream>

namespace geometry {

VoronoiDiagramResult DivideConquerVoronoi::Generate(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) {

  // Generate DCEL first (this also populates face_to_site_mapping_)
  DCEL* dcel = GenerateDCEL(sites, bounds);

  // Convert using face-to-site mapping
  // FIX: Use sorted_sites_ because face_to_site_mapping_ indices refer to sorted order.
  VoronoiDiagramResult result;
  if (face_to_site_mapping_.empty()) {
    result = DCELHelper::ConvertDCELToResult(dcel, sorted_sites_);
  } else {
    result = DCELHelper::ConvertDCELToResultWithMapping(dcel, sorted_sites_, face_to_site_mapping_);
  }

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
    face_to_site_mapping_.clear();
    face_to_site_mapping_.push_back(0);
    sorted_sites_ = sites;
    return dcel;
  }

  // Sort sites by x-coordinate
  sorted_sites_ = sites;
  std::sort(sorted_sites_.begin(), sorted_sites_.end(),
      [](const Point2D& a, const Point2D& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
      });

  // Use divide and conquer with face-to-site mapping
  face_to_site_mapping_.clear();
  delete dcel;
  dcel = DivideAndConquerWithDepth(sorted_sites_, bounds, 0, face_to_site_mapping_);

  return dcel;
}

DCEL* DivideConquerVoronoi::DivideAndConquerWithDepth(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds,
    int depth,
    std::vector<size_t>& face_to_site) const {

  std::string indent(depth * 2, ' ');
  std::cout << indent << "[DivideAndConquer] n=" << sites.size() << ", depth=" << depth << std::endl;

  size_t n = sites.size();

  // Base case: small number of sites, compute directly
  if (n == 1) {
    std::cout << indent << "[Base case n=1] Creating single cell" << std::endl;
    DCEL* dcel = new DCEL();
    DCELHelper::CreateBoundingBoxCell(dcel, bounds);
    face_to_site.push_back(0);
    return dcel;
  }

  if (n == 2) {
    std::cout << indent << "[Base case n=2] Sites: ("
              << sites[0].x << "," << sites[0].y << ") and ("
              << sites[1].x << "," << sites[1].y << ")" << std::endl;

    DCEL* dcel = new DCEL();
    std::vector<Face*> cells;
    for (size_t i = 0; i < 2; ++i) {
      Face* cell = DCELHelper::CreateBoundingBoxCell(dcel, bounds);
      cells.push_back(cell);
      face_to_site.push_back(i);
    }

    for (size_t i = 0; i < 2; ++i) {
      Point2D midpoint;
      Vector2D normal;
      HalfPlaneClipper::ComputeBisector(sites[i], sites[1-i], midpoint, normal);
      DCELHelper::ClipFaceByHalfPlane(dcel, cells[i], midpoint, normal);
    }

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

    DCEL* dcel = new DCEL();
    std::vector<Face*> cells;
    for (size_t i = 0; i < 3; ++i) {
      Face* cell = DCELHelper::CreateBoundingBoxCell(dcel, bounds);
      cells.push_back(cell);
      face_to_site.push_back(i);
    }

    for (size_t i = 0; i < 3; ++i) {
      for (size_t j = 0; j < 3; ++j) {
        if (i == j) continue;
        Point2D midpoint;
        Vector2D normal;
        HalfPlaneClipper::ComputeBisector(sites[i], sites[j], midpoint, normal);
        DCELHelper::ClipFaceByHalfPlane(dcel, cells[i], midpoint, normal);
      }
    }

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

  // Conquer: recursively compute Voronoi diagrams
  std::vector<size_t> left_face_to_site, right_face_to_site;
  DCEL* left_dcel = DivideAndConquerWithDepth(left_sites, bounds, depth + 1, left_face_to_site);
  DCEL* right_dcel = DivideAndConquerWithDepth(right_sites, bounds, depth + 1, right_face_to_site);

  std::cout << indent << "[Conquer] Left faces=" << left_dcel->GetFaceCount()
            << ", Right faces=" << right_dcel->GetFaceCount() << std::endl;

  // Merge: combine the two diagrams (no leaked DCEL here)
  DCEL* dcel = MergeDiagrams(left_dcel, right_dcel, left_sites, right_sites, bounds,
                      left_face_to_site, right_face_to_site, face_to_site);

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
    const VoronoiBounds& bounds,
    const std::vector<size_t>& left_face_to_site,
    const std::vector<size_t>& right_face_to_site,
    std::vector<size_t>& out_face_to_site) const {

  std::cout << "  [MergeDiagrams] Starting merge, left_faces=" << left_dcel->GetFaceCount()
            << ", right_faces=" << right_dcel->GetFaceCount() << std::endl;

  DCEL* merged_dcel = new DCEL();

  // Copy all faces from left and right diagrams
  std::map<Point2D, Vertex*> vertex_map;

  // Copy left diagram (only valid faces, mapping kept in sync)
  DCELHelper::CopyFacesWithMapping(left_dcel, merged_dcel, vertex_map,
                                    left_face_to_site, 0, out_face_to_site);
  std::cout << "  [MergeDiagrams] Copied left faces, total=" << merged_dcel->GetFaceCount() << std::endl;

  // Copy right diagram (site indices offset by left_sites.size())
  DCELHelper::CopyFacesWithMapping(right_dcel, merged_dcel, vertex_map,
                                    right_face_to_site, left_sites.size(), out_face_to_site);
  std::cout << "  [MergeDiagrams] Copied right faces, total=" << merged_dcel->GetFaceCount() << std::endl;

  // FIX: Pass face-to-site mapping to ClipFacesByDividingCurve instead of centroid heuristic
  std::cout << "  [MergeDiagrams] Clipping faces by dividing curve..." << std::endl;

  std::vector<Point2D> all_sites = left_sites;
  all_sites.insert(all_sites.end(), right_sites.begin(), right_sites.end());

  ClipFacesByDividingCurve(merged_dcel, left_sites, right_sites, all_sites, out_face_to_site);
  std::cout << "  [MergeDiagrams] After clipping, faces=" << merged_dcel->GetFaceCount() << std::endl;

  // Merge shared edges
  DCELHelper::MergeSharedEdges(merged_dcel);

  return merged_dcel;
}

void DivideConquerVoronoi::ClipFacesByDividingCurve(
    DCEL* dcel,
    const std::vector<Point2D>& left_sites,
    const std::vector<Point2D>& right_sites,
    const std::vector<Point2D>& all_sites,
    const std::vector<size_t>& face_to_site) const {

  std::cout << "    [ClipFaces] Processing " << dcel->GetFaceCount() << " faces" << std::endl;
  std::cout << "    [ClipFaces] Before clipping: " << dcel->GetFaceCount()
            << " faces, " << dcel->GetVertexCount() << " vertices, "
            << dcel->GetHalfEdgeCount() << " half-edges" << std::endl;

  size_t left_site_count = left_sites.size();

  // Capture face count before clipping to avoid iteration issues
  size_t face_count = dcel->GetFaceCount();

  for (size_t i = 0; i < face_count && i < face_to_site.size(); ++i) {
    Face* face = dcel->GetFace(i);
    if (face == nullptr || face->IsUnbounded()) continue;

    auto boundary_vertices = face->GetOuterBoundaryVertices();
    if (boundary_vertices.size() < 3) continue;

    // FIX: Use the known site index from the mapping directly
    size_t site_index = face_to_site[i];
    if (site_index >= all_sites.size()) continue;

    Point2D assigned_site = all_sites[site_index];
    bool is_left_site = (site_index < left_site_count);

    // Clip by sites on the OTHER side
    std::vector<Point2D> clip_points;
    std::vector<Vector2D> clip_normals;

    if (is_left_site) {
      std::cout << "      [Face " << i << "] assigned to LEFT site " << site_index
                << " (" << assigned_site.x << "," << assigned_site.y
                << "), clipping by " << right_sites.size() << " right sites" << std::endl;
      for (const auto& site : right_sites) {
        Point2D midpoint;
        Vector2D normal;
        HalfPlaneClipper::ComputeBisector(assigned_site, site, midpoint, normal);
        clip_points.push_back(midpoint);
        clip_normals.push_back(normal);
      }
    } else {
      std::cout << "      [Face " << i << "] assigned to RIGHT site " << site_index
                << " (" << assigned_site.x << "," << assigned_site.y
                << "), clipping by " << left_sites.size() << " left sites" << std::endl;
      for (const auto& site : left_sites) {
        Point2D midpoint;
        Vector2D normal;
        HalfPlaneClipper::ComputeBisector(assigned_site, site, midpoint, normal);
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
