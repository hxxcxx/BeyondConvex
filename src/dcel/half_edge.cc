#include "half_edge.h"
#include "vertex.h"
#include "face.h"

namespace geometry {

Vertex* HalfEdge::GetDestination() const {
  if (twin_ && twin_->GetOrigin()) {
    return twin_->GetOrigin();
  }
  return nullptr;
}

bool HalfEdge::IsOnBoundary() const {
  // A half-edge is on the boundary if:
  // 1. It has no face (unbounded face), or
  // 2. Its twin has no face
  return (face_ == nullptr) || (twin_ != nullptr && twin_->GetFace() == nullptr);
}

std::vector<HalfEdge*> HalfEdge::GetAdjacentEdges() const {
  std::vector<HalfEdge*> adjacent;
  
  if (!HasFace()) {
    return adjacent;
  }
  
  // Traverse all edges in the face boundary
  HalfEdge* current = const_cast<HalfEdge*>(this);
  do {
    adjacent.push_back(current);
    current = current->GetNext();
  } while (current != nullptr && current != const_cast<HalfEdge*>(this));
  
  return adjacent;
}

}  // namespace geometry
