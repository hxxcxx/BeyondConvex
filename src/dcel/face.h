#pragma once

#include <vector>
#include <memory>

namespace geometry {

// Forward declarations
class HalfEdge;

/**
 * @brief Face in a DCEL structure
 * 
 * A face represents a region in the planar subdivision.
 * It can have an outer boundary and multiple inner boundaries (holes).
 */
class Face {
 public:
  // Constructors
  Face() : id_(-1), outer_component_(nullptr) {}
  
  explicit Face(int id) : id_(id), outer_component_(nullptr) {}
  
  // Getters
  int GetId() const { return id_; }
  void SetId(int id) { id_ = id; }
  
  HalfEdge* GetOuterComponent() const { return outer_component_; }
  void SetOuterComponent(HalfEdge* edge) { outer_component_ = edge; }
  
  const std::vector<HalfEdge*>& GetInnerComponents() const { 
    return inner_components_; 
  }
  
  std::vector<HalfEdge*>& GetInnerComponents() { 
    return inner_components_; 
  }
  
  void AddInnerComponent(HalfEdge* edge) {
    inner_components_.push_back(edge);
  }
  
  // Utility functions
  bool HasOuterComponent() const { return outer_component_ != nullptr; }
  bool HasHoles() const { return !inner_components_.empty(); }
  
  // Get all vertices on the outer boundary (in CCW order)
  std::vector<class Vertex*> GetOuterBoundaryVertices() const;
  
  // Get all half-edges on the outer boundary (in CCW order)
  std::vector<HalfEdge*> GetOuterBoundaryEdges() const;
  
  // Get all vertices of a specific hole (in CW order)
  std::vector<class Vertex*> GetHoleVertices(size_t hole_index) const;
  
  // Get all half-edges of a specific hole (in CW order)
  std::vector<HalfEdge*> GetHoleEdges(size_t hole_index) const;
  
  // Count the number of edges on the outer boundary
  size_t OuterBoundarySize() const;
  
  // Count the total number of edges (outer + all holes)
  size_t TotalBoundarySize() const;
  
  // Check if this face is the unbounded face (outside the subdivision)
  bool IsUnbounded() const;
  
 private:
  int id_;                           // Unique identifier
  HalfEdge* outer_component_;        // Half-edge on outer boundary
  std::vector<HalfEdge*> inner_components_;  // Half-edges on hole boundaries
};

}  // namespace geometry
