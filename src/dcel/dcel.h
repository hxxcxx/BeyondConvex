#pragma once

#include "vertex.h"
#include "half_edge.h"
#include "face.h"
#include <vector>
#include <memory>
#include <unordered_map>

namespace geometry {

/**
 * @brief Doubly Connected Edge List (DCEL) data structure
 * 
 * DCEL is a data structure for representing planar subdivisions.
 * It maintains three types of records: vertices, half-edges, and faces.
 * 
 * Key properties:
 * - Each edge is represented by two half-edges (twins)
 * - Half-edges are oriented counter-clockwise around faces
 * - Each vertex points to one incident half-edge
 * - Each face points to one half-edge on its boundary
 */
class DCEL {
 public:
  // Constructors
  DCEL() : next_vertex_id_(0), next_edge_id_(0), next_face_id_(0) {}
  
  // Disable copying
  DCEL(const DCEL&) = delete;
  DCEL& operator=(const DCEL&) = delete;
  
  // Enable moving
  DCEL(DCEL&&) = default;
  DCEL& operator=(DCEL&&) = default;
  
  // ========== Vertex Operations ==========
  
  /**
   * @brief Create a new vertex
   * @param coordinates Position of the vertex
   * @return Pointer to the created vertex
   */
  Vertex* CreateVertex(const Point2D& coordinates);
  
  /**
   * @brief Get all vertices
   */
  const std::vector<std::unique_ptr<Vertex>>& GetVertices() const { 
    return vertices_; 
  }
  
  /**
   * @brief Get vertex by ID
   */
  Vertex* GetVertex(int id) const;
  
  /**
   * @brief Get number of vertices
   */
  size_t GetVertexCount() const { return vertices_.size(); }
  
  // ========== Half-Edge Operations ==========
  
  /**
   * @brief Create a new half-edge pair (undirected edge)
   * @param origin Origin vertex of the first half-edge
   * @param twin_origin Origin vertex of the twin half-edge
   * @return Pointer to the first half-edge
   */
  HalfEdge* CreateEdge(Vertex* origin, Vertex* twin_origin);
  
  /**
   * @brief Get all half-edges
   */
  const std::vector<std::unique_ptr<HalfEdge>>& GetHalfEdges() const { 
    return half_edges_; 
  }
  
  /**
   * @brief Get half-edge by ID
   */
  HalfEdge* GetHalfEdge(int id) const;
  
  /**
   * @brief Get number of half-edges
   */
  size_t GetHalfEdgeCount() const { return half_edges_.size(); }
  
  /**
   * @brief Get number of undirected edges
   */
  size_t GetEdgeCount() const { return half_edges_.size() / 2; }
  
  // ========== Face Operations ==========
  
  /**
   * @brief Create a new face
   * @return Pointer to the created face
   */
  Face* CreateFace();
  
  /**
   * @brief Get all faces
   */
  const std::vector<std::unique_ptr<Face>>& GetFaces() const { 
    return faces_; 
  }
  
  /**
   * @brief Get face by ID
   */
  Face* GetFace(int id) const;
  
  /**
   * @brief Get number of faces
   */
  size_t GetFaceCount() const { return faces_.size(); }
  
  // ========== Topology Operations ==========
  
  /**
   * @brief Connect two half-edges (set next/prev pointers)
   * @param prev The previous half-edge
   * @param next The next half-edge
   */
  void ConnectHalfEdges(HalfEdge* prev, HalfEdge* next);
  
  /**
   * @brief Set the face of a cycle of half-edges
   * @param start Starting half-edge of the cycle
   * @param face Face to assign
   */
  void SetFaceOfCycle(HalfEdge* start, Face* face);
  
  /**
   * @brief Split a face by adding a diagonal edge
   * @param face Face to split
   * @param v1 First vertex
   * @param v2 Second vertex
   * @return Pointer to the new half-edge from v1 to v2
   */
  HalfEdge* SplitFace(Face* face, Vertex* v1, Vertex* v2);
  
  /**
   * @brief Merge two adjacent faces by removing the shared edge
   * @param edge The edge separating the two faces
   * @return Pointer to the merged face
   */
  Face* MergeFaces(HalfEdge* edge);
  
  // ========== Validation ==========
  
  /**
   * @brief Validate the DCEL structure
   * @return true if the structure is valid
   */
  bool Validate() const;
  
  /**
   * @brief Clear all data
   */
  void Clear();
  
  /**
   * @brief Print statistics for debugging
   */
  void PrintStats() const;
  
 private:
  // Data storage
  std::vector<std::unique_ptr<Vertex>> vertices_;
  std::vector<std::unique_ptr<HalfEdge>> half_edges_;
  std::vector<std::unique_ptr<Face>> faces_;
  
  // ID counters
  int next_vertex_id_;
  int next_edge_id_;
  int next_face_id_;
};

}  // namespace geometry
