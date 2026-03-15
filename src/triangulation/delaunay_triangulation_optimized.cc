/**
 * @file delaunay_triangulation_optimized.cc
 * @brief Implementation of optimized Delaunay triangulation with spatial grid
 */

#include "delaunay_triangulation_optimized.h"
#include "triangulation_utils.h"
#include "../dcel/dcel.h"
#include <iostream>
#include <sstream>

namespace geometry {

TriangulationResult DelaunayTriangulationOptimized::Triangulate(
    const std::vector<Point2D>& points) {
  
  TriangulationResult result;
  
  // Validate input
  if (points.size() < 3) {
    std::cerr << "[Delaunay Optimized] Invalid input: less than 3 points" << std::endl;
    return result;
  }
  
  std::cout << "[Delaunay Optimized] Triangulating " << points.size() 
            << " points with spatial grid" << std::endl;
  
  // Step 1: Create spatial grid
  double cell_size;
  if (auto_cell_size_) {
    // Automatically determine cell size from point distribution
    cell_size = SpatialGrid::RecommendCellSize(points);
    std::cout << "[Delaunay Optimized] Auto-detected cell size: " << cell_size << std::endl;
  } else {
    // Use default cell size
    cell_size = 1.0;
  }
  
  spatial_grid_ = std::make_unique<SpatialGrid>(cell_size);
  spatial_grid_->UpdateBoundsFromPoints(points);
  
  std::cout << "[Delaunay Optimized] Spatial grid created" << std::endl;
  
  // Step 2: Create DCEL structure
  std::unique_ptr<DCEL> dcel = std::make_unique<DCEL>();
  
  // Step 3: Create super triangle
  std::cout << "[Delaunay Optimized] Creating super triangle..." << std::endl;
  Face* super_triangle = CreateSuperTriangle(points, dcel.get());
  
  // Step 4: Insert super triangle into spatial grid
  spatial_grid_->InsertTriangle(super_triangle, dcel.get());
  
  // Step 5: Incrementally insert points
  size_t last_face_count = dcel->GetFaceCount();
  
  for (size_t i = 0; i < points.size(); ++i) {
    const Point2D& point = points[i];
    
    if (i % 1000 == 0) {
      std::cout << "[Delaunay Optimized] Inserting point " << i << "/" 
                << points.size() << std::endl;
    }
    
    // Find bad triangles using spatial grid (OPTIMIZED)
    std::set<Face*> bad_triangles = FindBadTrianglesOptimized(point, dcel.get());
    
    if (bad_triangles.empty()) {
      // Point is outside all triangles (should not happen with super triangle)
      continue;
    }
    
    // Find boundary of bad triangles
    std::vector<HalfEdge*> boundary = FindBoundary(bad_triangles);
    
    // Remove bad triangles
    RemoveBadTriangles(bad_triangles, dcel.get());
    
    // Re-triangulate the hole
    RetriangulateHole(boundary, point, dcel.get());
    
    // Update spatial grid with new triangles
    UpdateSpatialGrid(dcel.get(), last_face_count);
    last_face_count = dcel->GetFaceCount();
  }
  
  std::cout << "[Delaunay Optimized] Triangulation complete: " 
            << dcel->GetFaceCount() << " faces" << std::endl;
  
  // Print grid statistics
  std::cout << GetGridStatistics() << std::endl;
  
  // Step 6: Convert DCEL to result
  result = ConvertDCELToResult(dcel.get(), super_triangle);
  
  std::cout << "[Delaunay Optimized] Generated " << result.triangles.size() 
            << " triangles" << std::endl;
  
  return result;
}

std::set<Face*> DelaunayTriangulationOptimized::FindBadTrianglesOptimized(
    const Point2D& point,
    DCEL* dcel) {
  
  std::set<Face*> bad_triangles;
  
  // Use spatial grid to get nearby triangles (OPTIMIZED)
  // This is O(1) average instead of O(n) for checking all triangles
  std::unordered_set<Face*> nearby_triangles = 
      spatial_grid_->QueryNearbyTriangles(point);
  
  std::cout << "[Delaunay Optimized] Checking " << nearby_triangles.size() 
            << " nearby triangles (vs " << dcel->GetFaceCount() 
            << " total)" << std::endl;
  
  // Test only nearby triangles for circumcircle condition
  for (Face* face : nearby_triangles) {
    if (face == nullptr || face->IsUnbounded()) continue;
    
    // Skip if already marked as removed (optional optimization)
    // if (face->IsRemoved()) continue;
    
    if (IsInCircumcircle(point, face)) {
      bad_triangles.insert(face);
    }
  }
  
  return bad_triangles;
}

void DelaunayTriangulationOptimized::UpdateSpatialGrid(
    DCEL* dcel,
    size_t start_index) {
  
  // Add new triangles to the spatial grid
  // New triangles are those added after start_index
  for (size_t i = start_index; i < dcel->GetFaceCount(); ++i) {
    Face* face = dcel->GetFace(i);
    if (face != nullptr && !face->IsUnbounded()) {
      spatial_grid_->InsertTriangle(face, dcel);
    }
  }
}

}  // namespace geometry
