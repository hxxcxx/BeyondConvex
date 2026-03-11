#pragma once

#include "dcel.h"
#include "../core/point2d.h"
#include <vector>
#include <memory>

namespace geometry {

/**
 * @brief Helper class for building DCEL structures
 * 
 * Provides convenient methods for constructing common DCEL configurations
 * such as polygons, triangulations, and planar subdivisions.
 */
class DCELBuilder {
 public:
  /**
   * @brief Create a DCEL from a simple polygon
   * @param vertices Vertices of the polygon in CCW order
   * @param dcel DCEL to populate
   * @return Pointer to the face representing the polygon interior
   */
  static Face* BuildPolygon(const std::vector<Point2D>& vertices, DCEL* dcel);
  
  /**
   * @brief Create a DCEL from a polygon with holes
   * @param outer_vertices Outer boundary vertices in CCW order
   * @param holes Vector of hole vertex lists, each in CW order
   * @param dcel DCEL to populate
   * @return Pointer to the face representing the polygon interior
   */
  static Face* BuildPolygonWithHoles(
      const std::vector<Point2D>& outer_vertices,
      const std::vector<std::vector<Point2D>>& holes,
      DCEL* dcel);
  
  /**
   * @brief Create a DCEL from a triangulation
   * @param vertices All vertices
   * @param triangles Triangle indices (each triplet is a triangle)
   * @param dcel DCEL to populate
   * @return Vector of faces, one for each triangle
   */
  static std::vector<Face*> BuildTriangulation(
      const std::vector<Point2D>& vertices,
      const std::vector<std::array<int, 3>>& triangles,
      DCEL* dcel);
  
  /**
   * @brief Create a rectangular bounding box DCEL
   * @param min_x Minimum x coordinate
   * @param min_y Minimum y coordinate
   * @param max_x Maximum x coordinate
   * @param max_y Maximum y coordinate
   * @param dcel DCEL to populate
   * @return Pointer to the face representing the rectangle interior
   */
  static Face* BuildBoundingBox(
      double min_x, double min_y,
      double max_x, double max_y,
      DCEL* dcel);
  
  /**
   * @brief Create a DCEL from Voronoi diagram edges
   * @param sites Voronoi sites
   * @param edges Voronoi edges
   * @param bounds_min_x Minimum x bound
   * @param bounds_min_y Minimum y bound
   * @param bounds_max_x Maximum x bound
   * @param bounds_max_y Maximum y bound
   * @param dcel DCEL to populate
   * @return Vector of faces, one for each Voronoi cell
   */
  static std::vector<Face*> BuildVoronoiDiagram(
      const std::vector<Point2D>& sites,
      const std::vector<Edge2D>& edges,
      double bounds_min_x, double bounds_min_y,
      double bounds_max_x, double bounds_max_y,
      DCEL* dcel);
  
 private:
  /**
   * @brief Helper to find or create a vertex at given coordinates
   */
  static Vertex* FindOrCreateVertex(
      const Point2D& point,
      std::unordered_map<Point2D, Vertex*, Point2DHash>& vertex_map,
      DCEL* dcel);
  
  /**
   * @brief Helper to connect edges in a cycle
   */
  static void ConnectCycle(
      const std::vector<HalfEdge*>& edges,
      DCEL* dcel);
};

}  // namespace geometry
