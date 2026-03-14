#include "voronoi_dcel.h"

namespace geometry {

VoronoiDiagramResult DCELVoronoi::Generate(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) {
  
  // Generate DCEL first, then convert
  DCEL* dcel = GenerateDCEL(sites, bounds);
  VoronoiDiagramResult result = DCELHelper::ConvertDCELToResult(dcel, sites);
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
    DCELHelper::CreateBoundingBoxCell(dcel, bounds);
    return dcel;
  }
  
  // Create initial bounding box cell for each site
  std::vector<Face*> cells;
  for (size_t i = 0; i < sites.size(); ++i) {
    Face* cell = DCELHelper::CreateBoundingBoxCell(dcel, bounds);
    cells.push_back(cell);
  }
  
  // Clip each cell against all other sites
  for (size_t i = 0; i < sites.size(); ++i) {
    ClipCellByAllSites(dcel, cells[i], sites, i, bounds);
  }
  
  // Merge shared edges
  DCELHelper::MergeSharedEdges(dcel);
  
  return dcel;
}

void DCELVoronoi::ClipCellByAllSites(
    DCEL* dcel,
    Face* cell,
    const std::vector<Point2D>& sites,
    size_t cell_index,
    const VoronoiBounds& bounds) const {
  
  for (size_t j = 0; j < sites.size(); ++j) {
    if (cell_index == j) continue;
    
    // Compute perpendicular bisector
    Point2D midpoint;
    Vector2D normal;
    HalfPlaneClipper::ComputeBisector(sites[cell_index], sites[j], midpoint, normal);
    
    // Clip cell by the half-plane
    DCELHelper::ClipFaceByHalfPlane(dcel, cell, midpoint, normal);
  }
}

}  // namespace geometry
