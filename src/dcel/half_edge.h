#pragma once

#include <memory>
#include <vector>

namespace geometry {

// Forward declarations
class Vertex;
class Face;

/**
 * @brief Half-edge in a DCEL structure
 * 
 * A half-edge represents a directed edge in the planar subdivision.
 * Each undirected edge is represented by two half-edges (twins).
 * The half-edge structure allows efficient traversal of the subdivision.
 */
class HalfEdge {
 public:
  // Constructors
  HalfEdge() : id_(-1), origin_(nullptr), twin_(nullptr), 
               next_(nullptr), prev_(nullptr), face_(nullptr) {}
  
  explicit HalfEdge(int id) : id_(id), origin_(nullptr), twin_(nullptr),
                              next_(nullptr), prev_(nullptr), face_(nullptr) {}
  
  // Getters
  int GetId() const { return id_; }
  void SetId(int id) { id_ = id; }
  
  Vertex* GetOrigin() const { return origin_; }
  void SetOrigin(Vertex* vertex) { origin_ = vertex; }
  
  Vertex* GetDestination() const;
  
  HalfEdge* GetTwin() const { return twin_; }
  void SetTwin(HalfEdge* twin) { twin_ = twin; }
  
  HalfEdge* GetNext() const { return next_; }
  void SetNext(HalfEdge* next) { next_ = next; }
  
  HalfEdge* GetPrev() const { return prev_; }
  void SetPrev(HalfEdge* prev) { prev_ = prev; }
  
  Face* GetFace() const { return face_; }
  void SetFace(Face* face) { face_ = face; }
  
  // Utility functions
  bool HasTwin() const { return twin_ != nullptr; }
  bool HasNext() const { return next_ != nullptr; }
  bool HasPrev() const { return prev_ != nullptr; }
  bool HasFace() const { return face_ != nullptr; }
  
  // Check if this half-edge is on the boundary
  bool IsOnBoundary() const;
  
  // Get the adjacent half-edges (share the same face)
  std::vector<HalfEdge*> GetAdjacentEdges() const;
  
 private:
  int id_;              // Unique identifier
  Vertex* origin_;      // Origin vertex of this half-edge
  HalfEdge* twin_;      // Oppositely directed half-edge
  HalfEdge* next_;      // Next half-edge in the face boundary (CCW)
  HalfEdge* prev_;      // Previous half-edge in the face boundary
  Face* face_;          // Face to the left of this half-edge
};

}  // namespace geometry
