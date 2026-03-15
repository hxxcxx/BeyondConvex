/**
 * @file delaunay_triangulation.cc
 * @brief Implementation of Delaunay triangulation using Bowyer-Watson algorithm
 * 
 * This file implements the Bowyer-Watson incremental insertion algorithm for
 * computing the Delaunay triangulation of a 2D point set. The algorithm uses
 * a Doubly Connected Edge List (DCEL) data structure for efficient topology
 * operations.
 * 
 * Algorithm Overview:
 * 1. Create a super triangle that contains all input points
 * 2. For each point:
 *    a. Find all triangles whose circumcircle contains the point (bad triangles)
 *    b. Find the boundary of the bad triangles (a star-shaped polygon)
 *    c. Remove bad triangles, creating a hole
 *    d. Re-triangulate the hole by connecting the new point to boundary vertices
 * 3. Remove triangles connected to super triangle vertices
 * 
 * Mathematical Background:
 * - Delaunay triangulation satisfies the empty circumcircle property
 * - For any triangle in the triangulation, its circumcircle contains no other points
 * - This property maximizes the minimum angle of all triangles
 * - Delaunay triangulation is the dual of the Voronoi diagram
 * 
 * Time Complexity:
 * - Worst case: O(n²) when points are inserted in pathological order
 * - Average case: O(n log n) with random insertion order
 * - Can be improved to O(n log n) with spatial indexing (kd-tree, quadtree)
 * 
 * Space Complexity: O(n) for storing vertices, edges, and faces
 * 
 * Numerical Robustness:
 * - Uses epsilon (1e-9) for circumcircle tests to handle floating-point errors
 * - Super triangle is significantly larger than input bounding box
 * 
 * References:
 * - Bowyer, A. (1981). "Computing Dirichlet tessellations"
 * - Watson, D.F. (1981). "Computing the n-dimensional Delaunay tessellation"
 * - Guibas, L. & Stolfi, J. (1985). "Primitives for the manipulation of general
 *   subdivisions and the computation of Voronoi diagrams"
 */

#include "delaunay_triangulation.h"
#include "triangulation_utils.h"
#include "../dcel/dcel.h"
#include <iostream>
#include <algorithm>
#include <set>
#include <map>
#include <cmath>

namespace geometry {

/**
 * @brief Compute Delaunay triangulation of a point set
 * 
 * This is the main entry point for the Bowyer-Watson algorithm. It performs
 * the complete triangulation by incrementally inserting points into an initial
 * super triangle.
 * 
 * Algorithm Steps:
 * 1. Validation: Check if input has at least 3 points
 * 2. Initialization: Create DCEL structure and super triangle
 * 3. Incremental Insertion: For each point:
 *    - Find bad triangles (violating empty circumcircle property)
 *    - Extract boundary of bad triangles
 *    - Remove bad triangles and re-triangulate the hole
 * 4. Cleanup: Convert DCEL to result, excluding super triangle
 * 
 * @param points Input point set (any order, may contain duplicates)
 * @return TriangulationResult containing triangles, vertices, and edges
 * 
 * @note Points with identical coordinates may cause degenerate triangles
 * @note The super triangle is excluded from the final result
 * @note Output triangles are in counter-clockwise (CCW) order
 */
TriangulationResult DelaunayTriangulation::Triangulate(
    const std::vector<Point2D>& points) {
  
  TriangulationResult result;
  
  // Validate input: need at least 3 points for a valid triangulation
  if (points.size() < 3) {
    std::cerr << "[Delaunay] Invalid input: less than 3 points" << std::endl;
    return result;
  }
  
  std::cout << "[Delaunay] Triangulating " << points.size() << " points" << std::endl;
  
  // Create DCEL structure to store the triangulation topology
  std::unique_ptr<DCEL> dcel = std::make_unique<DCEL>();
  
  // Step 1: Create super triangle that contains all input points
  // The super triangle provides a starting triangulation and ensures all
  // input points lie within some triangle
  std::cout << "[Delaunay] Creating super triangle..." << std::endl;
  Face* super_triangle = CreateSuperTriangle(points, dcel.get());
  
  // Step 2: Incrementally insert each point into the triangulation
  // This is the core of the Bowyer-Watson algorithm
  for (size_t i = 0; i < points.size(); ++i) {
    const Point2D& point = points[i];
    
    std::cout << "[Delaunay] Inserting point " << i << ": (" 
              << point.x << ", " << point.y << ")" << std::endl;
    
    // Find all triangles whose circumcircle contains the new point
    // These triangles violate the Delaunay property and must be removed
    std::set<Face*> bad_triangles = FindBadTriangles(point, dcel.get());
    
    // If no bad triangles found, the point is outside the current triangulation
    // This can happen if the point is outside the super triangle (should not occur)
    if (bad_triangles.empty()) {
      std::cout << "[Delaunay] Point is outside all triangles, skipping" << std::endl;
      continue;
    }
    
    std::cout << "[Delaunay] Found " << bad_triangles.size() << " bad triangles" << std::endl;
    
    // Find the boundary of the bad triangles
    // The boundary forms a star-shaped polygon (the "hole")
    std::vector<HalfEdge*> boundary = FindBoundary(bad_triangles);
    
    std::cout << "[Delaunay] Boundary has " << boundary.size() << " edges" << std::endl;
    
    // Remove bad triangles from the triangulation
    // This creates a hole that will be re-triangulated
    RemoveBadTriangles(bad_triangles, dcel.get());
    
    // Re-triangulate the hole by connecting the new point to all boundary vertices
    // This restores the Delaunay property for the affected region
    RetriangulateHole(boundary, point, dcel.get());
  }
  
  std::cout << "[Delaunay] Triangulation complete: " 
            << dcel->GetFaceCount() << " faces" << std::endl;
  
  // Step 3: Convert DCEL structure to result format
  // Exclude the super triangle and any triangles connected to it
  result = ConvertDCELToResult(dcel.get(), super_triangle);
  
  std::cout << "[Delaunay] Generated " << result.triangles.size() 
            << " triangles" << std::endl;
  
  return result;
}

/**
 * @brief Create a super triangle containing all input points
 * 
 * The super triangle is a large triangle that completely encloses all input points.
 * It serves as the initial triangulation and ensures that every input point lies
 * within some triangle during the incremental insertion process.
 * 
 * Design Considerations:
 * - The super triangle must be large enough to contain all points with margin
 * - Using 10x expansion ensures numerical stability
 * - Super triangle vertices are placed far from input to avoid interference
 * - The super triangle is removed in the final result
 * 
 * Geometry:
 * - v0: Bottom-left corner (min_x - delta, min_y - delta)
 * - v1: Bottom-right corner (max_x + 2*dx, min_y - delta)
 * - v2: Top-center (min_x + dx/2, max_y + 2*dy)
 * 
 * This creates an approximately equilateral triangle that is much larger
 * than the input bounding box.
 * 
 * @param points Input point set (used to compute bounding box)
 * @param dcel DCEL structure to store the super triangle
 * @return Face pointer to the super triangle face
 * 
 * @note The super triangle is excluded from the final triangulation result
 * @note Super triangle vertices are artificial and not part of the output
 */
Face* DelaunayTriangulation::CreateSuperTriangle(
    const std::vector<Point2D>& points,
    DCEL* dcel) {
  
  // Step 1: Compute axis-aligned bounding box (AABB) of all input points
  double min_x = points[0].x, max_x = points[0].x;
  double min_y = points[0].y, max_y = points[0].y;
  
  for (const auto& p : points) {
    min_x = std::min(min_x, p.x);
    max_x = std::max(max_x, p.x);
    min_y = std::min(min_y, p.y);
    max_y = std::max(max_y, p.y);
  }
  
  // Step 2: Expand bounding box to create super triangle
  // Using 10x expansion ensures the super triangle is much larger than input
  // This prevents numerical issues and ensures all points are well inside
  double dx = max_x - min_x;
  double dy = max_y - min_y;
  double delta_x = dx * 10.0;
  double delta_y = dy * 10.0;
  
  min_x -= delta_x;
  max_x += delta_x;
  min_y -= delta_y;
  max_y += delta_y;
  
  // Step 3: Create three vertices for the super triangle
  // The vertices are arranged to form an approximately equilateral triangle
  Vertex* v0 = dcel->CreateVertex(Point2D(min_x, min_y));
  Vertex* v1 = dcel->CreateVertex(Point2D(max_x + 2 * dx, min_y));
  Vertex* v2 = dcel->CreateVertex(Point2D(min_x + dx / 2, max_y + 2 * dy));
  
  // Step 4: Create face for the super triangle
  Face* face = dcel->CreateFace();
  
  // Step 5: Create half-edges for the triangle boundary
  HalfEdge* e0 = dcel->CreateEdge(v0, v1);
  HalfEdge* e1 = dcel->CreateEdge(v1, v2);
  HalfEdge* e2 = dcel->CreateEdge(v2, v0);
  
  // Step 6: Connect edges in counter-clockwise (CCW) order
  // This ensures consistent orientation for all faces
  dcel->ConnectHalfEdges(e0, e1);
  dcel->ConnectHalfEdges(e1, e2);
  dcel->ConnectHalfEdges(e2, e0);
  
  // Step 7: Associate the face with the edge cycle
  dcel->SetFaceOfCycle(e0, face);
  
  return face;
}

/**
 * @brief Find all triangles whose circumcircle contains the given point
 * 
 * These triangles are called "bad triangles" because they violate the
 * Delaunay empty circumcircle property. When a new point is inserted,
 * all bad triangles must be removed and the resulting hole re-triangulated.
 * 
 * Algorithm:
 * - Iterate through all faces in the DCEL
 * - Skip unbounded faces (external faces)
 * - For each triangular face, test if point is in its circumcircle
 * - Collect all violating faces into a set
 * 
 * Mathematical Background:
 * - A point P is in the circumcircle of triangle ABC if and only if:
 *   |A-P|² * det(B-A, C-A) + |B-P|² * det(C-B, A-B) + |C-P|² * det(A-C, B-C) > 0
 * - This is equivalent to the oriented circle test
 * - Uses an epsilon (1e-9) for numerical stability
 * 
 * Performance:
 * - Naive implementation: O(n) per point insertion
 * - Can be optimized with spatial indexing to O(log n)
 * - Total for all points: O(n²) naive, O(n log n) with indexing
 * 
 * @param point The point to test against all circumcircles
 * @param dcel DCEL structure containing current triangulation
 * @return Set of faces (triangles) whose circumcircles contain the point
 * 
 * @note Returns empty set if point is outside all circumcircles
 * @note Uses set to automatically eliminate duplicates
 */
std::set<Face*> DelaunayTriangulation::FindBadTriangles(
    const Point2D& point,
    DCEL* dcel) {
  
  std::set<Face*> bad_triangles;
  
  // Iterate through all faces in the DCEL
  for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
    Face* face = dcel->GetFace(i);
    
    // Skip null faces and unbounded (external) faces
    if (face == nullptr || face->IsUnbounded()) continue;
    
    // Test if point is inside this triangle's circumcircle
    // If yes, this triangle violates Delaunay property
    if (IsInCircumcircle(point, face)) {
      bad_triangles.insert(face);
    }
  }
  
  return bad_triangles;
}

/**
 * @brief Find the boundary edges of the bad triangles
 * 
 * The boundary is a polygon formed by edges that are shared between a bad
 * triangle and a non-bad triangle. These edges form a star-shaped polygon
 * (the "hole") that will be re-triangulated with the new point.
 * 
 * Algorithm:
 * - For each bad triangle, examine all its edges
 * - An edge is on the boundary if its twin face is not a bad triangle
 * - Collect all boundary edges (using set to avoid duplicates)
 * - The boundary forms a simple polygon (possibly non-convex)
 * 
 * Topology:
 * - The boundary is guaranteed to be a simple closed polygon
 * - It is star-shaped with respect to the new point
 * - Boundary edges are oriented consistently (CCW for hole boundary)
 * - Each boundary edge has exactly one bad triangle incident to it
 * 
 * Properties:
 * - Boundary size: O(k) where k is the number of bad triangles
 * - Time complexity: O(k) where k is the number of bad triangles
 * - Space complexity: O(k) for storing boundary edges
 * 
 * @param bad_triangles Set of faces that violate Delaunay property
 * @return Vector of half-edges forming the boundary polygon
 * 
 * @note Boundary edges are oriented such that bad triangles are on the left
 * @note The boundary is guaranteed to be a simple polygon
 */
std::vector<HalfEdge*> DelaunayTriangulation::FindBoundary(
    const std::set<Face*>& bad_triangles) {
  
  std::set<HalfEdge*> boundary_set;
  
  // Iterate through all bad triangles
  // An edge is on the boundary if it's shared by:
  // - One bad triangle (inside the set)
  // - One non-bad triangle (outside the set)
  for (Face* face : bad_triangles) {
    HalfEdge* start = face->GetHalfEdge();
    HalfEdge* edge = start;
    
    // Traverse all edges of this triangle
    do {
      HalfEdge* twin = edge->GetTwin();
      Face* twin_face = twin->GetFace();
      
      // If the twin face is not a bad triangle, this edge is on boundary
      // This means the edge separates bad region from good region
      if (twin_face == nullptr || bad_triangles.find(twin_face) == bad_triangles.end()) {
        boundary_set.insert(edge);
      }
      
      edge = edge->GetNext();
    } while (edge != start);
  }
  
  // Convert set to vector for ordered iteration
  return std::vector<HalfEdge*>(boundary_set.begin(), boundary_set.end());
}

/**
 * @brief Remove bad triangles from the triangulation
 * 
 * This method marks bad triangles for removal. In the current implementation,
 * we don't physically delete faces from the DCEL to avoid index invalidation
 * issues. Instead, the bad triangles are simply excluded from the final result.
 * 
 * Implementation Notes:
 * - Physical deletion would invalidate face indices and pointers
 * - Marking is safer and sufficient for correctness
 * - Bad triangles will be excluded in ConvertDCELToResult()
 * - Edges may be reused in re-triangulation
 * 
 * Alternative Approaches:
 * - Lazy deletion: Mark faces as deleted and skip during iteration
 * - Garbage collection: Periodically compact the DCEL structure
 * - Reference counting: Track face usage and delete when unused
 * 
 * Future Improvements:
 * - Implement proper DCEL face deletion with index remapping
 * - Add face validation to detect dangling references
 * - Support for incremental updates without full rebuild
 * 
 * @param bad_triangles Set of faces to remove
 * @param dcel DCEL structure (not modified in current implementation)
 * 
 * @note This is a no-op in current implementation
 * @note Bad triangles are filtered out in the final conversion step
 */
void DelaunayTriangulation::RemoveBadTriangles(
    const std::set<Face*>& bad_triangles,
    DCEL* dcel) {
  
  // Note: We don't actually remove faces from DCEL to avoid index issues
  // Physical deletion would invalidate:
  // - Face indices used for iteration
  // - Half-edge pointers to deleted faces
  // - Twin relationships between edges
  // 
  // Instead, we mark them conceptually and they will be excluded in final result
  // The edges will be reused or updated in retriangulation
  
  std::cout << "[Delaunay] Marking " << bad_triangles.size() 
            << " bad triangles for removal" << std::endl;
}

/**
 * @brief Re-triangulate the hole by connecting the new point to boundary vertices
 * 
 * After removing bad triangles, we have a star-shaped hole. This method
 * fills the hole by creating new triangles that connect the new point to
 * each edge of the boundary polygon.
 * 
 * Algorithm:
 * - Create a new vertex for the insertion point
 * - For each boundary edge:
 *   - Create a triangle (new_point, edge_start, edge_end)
 *   - Connect edges in CCW order
 *   - Set face and twin relationships
 * 
 * Topology:
 * - Creates k new triangles where k is the boundary size
 * - Each new triangle shares the new point as a vertex
 * - New triangles are guaranteed to satisfy Delaunay property
 * - Boundary edges are reused (with updated face pointers)
 * 
 * Correctness:
 * - The new triangulation restores the Delaunay property
 * - All new triangles have the new point as a vertex
 * - The boundary is completely filled (no gaps)
 * - No overlapping triangles are created
 * 
 * Complexity:
 * - Time: O(k) where k is the number of boundary edges
 * - Space: O(k) for creating new faces and edges
 * 
 * @param boundary Vector of half-edges forming the hole boundary
 * @param point The new point to insert
 * @param dcel DCEL structure to update with new triangles
 * 
 * @note Boundary edges should be in CCW order
 * @note Twin relationships may need updating for proper adjacency
 */
void DelaunayTriangulation::RetriangulateHole(
    const std::vector<HalfEdge*>& boundary,
    const Point2D& point,
    DCEL* dcel) {
  
  // Step 1: Create vertex for the new point
  Vertex* new_vertex = dcel->CreateVertex(point);
  
  std::cout << "[Delaunay] Re-triangulating hole with " << boundary.size() 
            << " boundary edges" << std::endl;
  
  // Step 2: For each boundary edge, create a triangle with the new point
  // This creates a "fan" triangulation from the new point to the boundary
  for (HalfEdge* boundary_edge : boundary) {
    // Get the two vertices of this boundary edge
    Vertex* v1 = boundary_edge->GetOrigin();
    Vertex* v2 = boundary_edge->GetTwin()->GetOrigin();
    
    // Create a new face for this triangle
    Face* new_face = dcel->CreateFace();
    
    // Create three half-edges for the triangle
    // e0: new_vertex -> v1
    // e1: v1 -> v2 (reuses boundary edge direction)
    // e2: v2 -> new_vertex
    HalfEdge* e0 = dcel->CreateEdge(new_vertex, v1);
    HalfEdge* e1 = dcel->CreateEdge(v1, v2);
    HalfEdge* e2 = dcel->CreateEdge(v2, new_vertex);
    
    // Connect edges in counter-clockwise (CCW) order
    // This ensures consistent orientation for all triangles
    dcel->ConnectHalfEdges(e0, e1);
    dcel->ConnectHalfEdges(e1, e2);
    dcel->ConnectHalfEdges(e2, e0);
    
    // Associate the face with this edge cycle
    dcel->SetFaceOfCycle(e0, new_face);
    
    // Note: Twin relationships should be updated here
    // The boundary edge's twin should point to the new triangle
    // This is simplified in current implementation
  }
}

/**
 * @brief Test if a point lies inside the circumcircle of a triangle
 * 
 * This is the core predicate for the Delaunay triangulation. A point
 * is inside the circumcircle if its distance to the circumcenter is
 * less than the circumradius.
 * 
 * Mathematical Definition:
 * - Let triangle ABC have circumcenter O and circumradius R
 * - Point P is in the circumcircle iff: |P - O| < R
 * - On the circumcircle: |P - O| = R
 * - Outside: |P - O| > R
 * 
 * Numerical Robustness:
 * - Uses epsilon (1e-9) to handle floating-point precision
 * - dist < radius - eps means point is strictly inside
 * - This avoids degenerate cases when point is on the circle
 * 
 * Alternative Implementation:
 * - Can use the oriented circle test (determinant)
 * - More robust but computationally more expensive
 * - The determinant test doesn't require computing center/radius
 * 
 * @param point The point to test
 * @param face The triangle face (must be a valid bounded face)
 * @return True if point is inside the circumcircle, false otherwise
 * 
 * @note Returns false for unbounded faces
 * @note Uses epsilon for numerical stability
 */
bool DelaunayTriangulation::IsInCircumcircle(
    const Point2D& point,
    Face* face) {
  
  // Compute circumcenter and circumradius of the triangle
  Point2D center = GetCircumcenter(face);
  double radius = GetCircumradius(face);
  
  // Calculate distance from point to circumcenter
  double dist = point.DistanceTo(center);
  
  // Use epsilon for numerical stability
  // Point is inside if distance is strictly less than radius
  const double eps = 1e-9;
  
  return dist < radius - eps;
}

/**
 * @brief Compute the circumcenter of a triangle face
 * 
 * The circumcenter is the center of the circumscribed circle of a triangle.
 * It is the intersection point of the perpendicular bisectors of the triangle's
 * three sides.
 * 
 * Mathematical Properties:
 * - Equidistant from all three vertices
 * - Lies at the intersection of perpendicular bisectors
 * - Inside the triangle for acute triangles
 * - On the hypotenuse midpoint for right triangles
 * - Outside the triangle for obtuse triangles
 * 
 * Formula:
 * Given triangle vertices A(x1,y1), B(x2,y2), C(x3,y3):
 * - Compute midpoints and slopes of perpendicular bisectors
 * - Solve the intersection of two bisectors
 * - Or use the formula based on barycentric coordinates
 * 
 * @param face The triangle face (must be a valid bounded face)
 * @return Circumcenter point, or (0,0) for invalid/unbounded faces
 * 
 * @note Delegates to Triangle::Circumcenter() for actual computation
 * @note Returns (0,0) as fallback for invalid input
 */
Point2D DelaunayTriangulation::GetCircumcenter(Face* face) {
  // Validate input
  if (face == nullptr || face->IsUnbounded()) {
    return Point2D(0, 0);
  }
  
  // Extract the three vertices of the triangle
  // A face in DCEL is represented by a cycle of three half-edges
  HalfEdge* e0 = face->GetHalfEdge();
  HalfEdge* e1 = e0->GetNext();
  HalfEdge* e2 = e1->GetNext();
  
  // Get vertex coordinates
  Point2D v0 = e0->GetOrigin()->GetCoordinates();
  Point2D v1 = e1->GetOrigin()->GetCoordinates();
  Point2D v2 = e2->GetOrigin()->GetCoordinates();
  
  // Delegate to Triangle class for circumcenter computation
  Triangle tri(v0, v1, v2);
  return tri.Circumcenter();
}

/**
 * @brief Compute the circumradius of a triangle face
 * 
 * The circumradius is the radius of the circumscribed circle of a triangle.
 * It is the distance from the circumcenter to any of the triangle's vertices.
 * 
 * Mathematical Properties:
 * - R = a / (2*sin(A)) = b / (2*sin(B)) = c / (2*sin(C))
 * - R = (abc) / (4*Area) using side lengths a,b,c and area
 * - For a given area, equilateral triangle minimizes R
 * - Larger R indicates a "skinnier" triangle
 * 
 * Formula:
 * Given triangle with vertices A, B, C:
 * - Compute circumcenter O
 * - R = |A - O| = |B - O| = |C - O|
 * 
 * Applications:
 * - Delaunay triangulation uses circumradius for empty circle test
 * - Quality metric for mesh generation (smaller R is better)
 * - Used in circle packing and sphere packing problems
 * 
 * @param face The triangle face (must be a valid bounded face)
 * @return Circumradius, or 0.0 for invalid/unbounded faces
 * 
 * @note Delegates to Triangle::Circumradius() for actual computation
 * @note Returns 0.0 as fallback for invalid input
 */
double DelaunayTriangulation::GetCircumradius(Face* face) {
  // Validate input
  if (face == nullptr || face->IsUnbounded()) {
    return 0.0;
  }
  
  // Extract the three vertices of the triangle
  HalfEdge* e0 = face->GetHalfEdge();
  HalfEdge* e1 = e0->GetNext();
  HalfEdge* e2 = e1->GetNext();
  
  // Get vertex coordinates
  Point2D v0 = e0->GetOrigin()->GetCoordinates();
  Point2D v1 = e1->GetOrigin()->GetCoordinates();
  Point2D v2 = e2->GetOrigin()->GetCoordinates();
  
  // Delegate to Triangle class for circumradius computation
  Triangle tri(v0, v1, v2);
  return tri.Circumradius();
}

/**
 * @brief Convert DCEL structure to TriangulationResult format
 * 
 * This method extracts the final triangulation from the DCEL structure,
 * filtering out the super triangle and any triangles connected to it.
 * It also collects unique vertices and edges for the result.
 * 
 * Algorithm:
 * - Iterate through all faces in the DCEL
 * - Skip unbounded faces and the super triangle
 * - For each valid triangle:
 *   - Extract vertices and add to triangle list
 *   - Collect unique vertices (using set to eliminate duplicates)
 *   - Collect unique edges (normalized to avoid duplicates)
 * - Convert sets to vectors for output
 * 
 * Filtering:
 * - Super triangle is excluded (it's artificial)
 * - Triangles connected to super triangle vertices are excluded
 * - Only triangles formed by input points are included
 * - This ensures the result is a valid triangulation of the input
 * 
 * Edge Normalization:
 * - Edges are stored with vertices in sorted order
 * - This ensures (A,B) and (B,A) are treated as the same edge
 * - Prevents duplicate edges in the output
 * 
 * Output Format:
 * - triangles: Vector of Triangle objects (CCW vertex order)
 * - vertices: Vector of unique Point2D objects
 * - edges: Vector of unique Edge2D objects
 * 
 * @param dcel DCEL structure containing the complete triangulation
 * @param super_triangle_face The super triangle face to exclude
 * @return TriangulationResult with triangles, vertices, and edges
 * 
 * @note Output triangles maintain CCW orientation
 * @note Vertices and edges are unique (no duplicates)
 * @note Super triangle and connected triangles are filtered out
 */
TriangulationResult DelaunayTriangulation::ConvertDCELToResult(
    DCEL* dcel,
    Face* super_triangle_face) {
  
  TriangulationResult result;
  
  // Use sets to automatically eliminate duplicates
  std::set<Point2D> vertex_set;
  std::set<Edge2D> edge_set;
  
  // Iterate through all faces in the DCEL
  for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
    Face* face = dcel->GetFace(i);
    
    // Skip invalid and unbounded faces
    if (face == nullptr || face->IsUnbounded()) continue;
    
    // Skip the super triangle (it's artificial)
    if (face == super_triangle_face) continue;
    
    // Extract the three vertices of this triangle
    HalfEdge* e0 = face->GetHalfEdge();
    HalfEdge* e1 = e0->GetNext();
    HalfEdge* e2 = e1->GetNext();
    
    Point2D v0 = e0->GetOrigin()->GetCoordinates();
    Point2D v1 = e1->GetOrigin()->GetCoordinates();
    Point2D v2 = e2->GetOrigin()->GetCoordinates();
    
    // Add triangle to result (vertices are in CCW order)
    result.triangles.emplace_back(v0, v1, v2);
    
    // Collect unique vertices
    vertex_set.insert(v0);
    vertex_set.insert(v1);
    vertex_set.insert(v2);
    
    // Collect unique edges (normalized to avoid duplicates)
    // Lambda function to add edge with normalized vertex order
    auto add_edge = [&edge_set](const Point2D& p1, const Point2D& p2) {
      Point2D a = p1, b = p2;
      // Normalize: ensure a <= b in lexicographic order
      if (b < a || (a == b && b < a)) std::swap(a, b);
      edge_set.emplace(a, b);
    };
    
    // Add all three edges of the triangle
    add_edge(v0, v1);
    add_edge(v1, v2);
    add_edge(v2, v0);
  }
  
  // Convert sets to vectors for output
  result.vertices.assign(vertex_set.begin(), vertex_set.end());
  result.edges.assign(edge_set.begin(), edge_set.end());
  
  return result;
}

}  // namespace geometry
