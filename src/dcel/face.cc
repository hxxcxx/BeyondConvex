#include "face.h"
#include "half_edge.h"
#include "vertex.h"

namespace geometry {

std::vector<Vertex*> Face::GetOuterBoundaryVertices() const {
  std::vector<Vertex*> vertices;
  
  if (!HasOuterComponent()) {
    return vertices;
  }
  
  HalfEdge* start = outer_component_;
  HalfEdge* current = start;
  
  do {
    if (current->GetOrigin()) {
      vertices.push_back(current->GetOrigin());
    }
    current = current->GetNext();
  } while (current != nullptr && current != start);
  
  return vertices;
}

std::vector<HalfEdge*> Face::GetOuterBoundaryEdges() const {
  std::vector<HalfEdge*> edges;
  
  if (!HasOuterComponent()) {
    return edges;
  }
  
  HalfEdge* start = outer_component_;
  HalfEdge* current = start;
  
  do {
    edges.push_back(current);
    current = current->GetNext();
  } while (current != nullptr && current != start);
  
  return edges;
}

std::vector<Vertex*> Face::GetHoleVertices(size_t hole_index) const {
  std::vector<Vertex*> vertices;
  
  if (hole_index >= inner_components_.size()) {
    return vertices;
  }
  
  HalfEdge* start = inner_components_[hole_index];
  HalfEdge* current = start;
  
  do {
    if (current->GetOrigin()) {
      vertices.push_back(current->GetOrigin());
    }
    current = current->GetNext();
  } while (current != nullptr && current != start);
  
  return vertices;
}

std::vector<HalfEdge*> Face::GetHoleEdges(size_t hole_index) const {
  std::vector<HalfEdge*> edges;
  
  if (hole_index >= inner_components_.size()) {
    return edges;
  }
  
  HalfEdge* start = inner_components_[hole_index];
  HalfEdge* current = start;
  
  do {
    edges.push_back(current);
    current = current->GetNext();
  } while (current != nullptr && current != start);
  
  return edges;
}

size_t Face::OuterBoundarySize() const {
  if (!HasOuterComponent()) {
    return 0;
  }
  
  size_t count = 0;
  HalfEdge* start = outer_component_;
  HalfEdge* current = start;
  
  do {
    count++;
    current = current->GetNext();
  } while (current != nullptr && current != start);
  
  return count;
}

size_t Face::TotalBoundarySize() const {
  size_t total = OuterBoundarySize();
  
  for (const auto& hole : inner_components_) {
    HalfEdge* start = hole;
    HalfEdge* current = start;
    
    do {
      total++;
      current = current->GetNext();
    } while (current != nullptr && current != start);
  }
  
  return total;
}

bool Face::IsUnbounded() const {
  // The unbounded face is typically the one without an outer component
  // or with a null outer component
  return outer_component_ == nullptr;
}

}  // namespace geometry
