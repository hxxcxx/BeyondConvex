#include "dcel.h"
#include <iostream>
#include <stdexcept>

namespace geometry {

// ========== Vertex Operations ==========

Vertex* DCEL::CreateVertex(const Point2D& coordinates) {
  auto vertex = std::make_unique<Vertex>(coordinates, next_vertex_id_);
  Vertex* ptr = vertex.get();
  vertices_.push_back(std::move(vertex));
  next_vertex_id_++;
  return ptr;
}

Vertex* DCEL::GetVertex(int id) const {
  if (id < 0 || id >= static_cast<int>(vertices_.size())) {
    return nullptr;
  }
  return vertices_[id].get();
}

// ========== Half-Edge Operations ==========

HalfEdge* DCEL::CreateEdge(Vertex* origin, Vertex* twin_origin) {
  if (origin == nullptr || twin_origin == nullptr) {
    throw std::invalid_argument("Cannot create edge with null vertices");
  }
  
  // Create twin half-edges
  auto edge1 = std::make_unique<HalfEdge>(next_edge_id_);
  auto edge2 = std::make_unique<HalfEdge>(next_edge_id_ + 1);
  
  HalfEdge* ptr1 = edge1.get();
  HalfEdge* ptr2 = edge2.get();
  
  // Set origins
  ptr1->SetOrigin(origin);
  ptr2->SetOrigin(twin_origin);
  
  // Link as twins
  ptr1->SetTwin(ptr2);
  ptr2->SetTwin(ptr1);
  
  // Add to collection
  half_edges_.push_back(std::move(edge1));
  half_edges_.push_back(std::move(edge2));
  
  next_edge_id_ += 2;
  
  // Update vertex incident edges if not set
  if (origin->GetIncidentEdge() == nullptr) {
    origin->SetIncidentEdge(ptr1);
  }
  if (twin_origin->GetIncidentEdge() == nullptr) {
    twin_origin->SetIncidentEdge(ptr2);
  }
  
  return ptr1;
}

HalfEdge* DCEL::GetHalfEdge(int id) const {
  if (id < 0 || id >= static_cast<int>(half_edges_.size())) {
    return nullptr;
  }
  return half_edges_[id].get();
}

// ========== Face Operations ==========

Face* DCEL::CreateFace() {
  auto face = std::make_unique<Face>(next_face_id_);
  Face* ptr = face.get();
  faces_.push_back(std::move(face));
  next_face_id_++;
  return ptr;
}

Face* DCEL::GetFace(int id) const {
  if (id < 0 || id >= static_cast<int>(faces_.size())) {
    return nullptr;
  }
  return faces_[id].get();
}

// ========== Topology Operations ==========

void DCEL::ConnectHalfEdges(HalfEdge* prev, HalfEdge* next) {
  if (prev == nullptr || next == nullptr) {
    throw std::invalid_argument("Cannot connect null half-edges");
  }
  
  prev->SetNext(next);
  next->SetPrev(prev);
}

void DCEL::SetFaceOfCycle(HalfEdge* start, Face* face) {
  if (start == nullptr) {
    return;
  }
  
  HalfEdge* current = start;
  do {
    current->SetFace(face);
    current = current->GetNext();
  } while (current != nullptr && current != start);
}

HalfEdge* DCEL::SplitFace(Face* face, Vertex* v1, Vertex* v2) {
  // This is a complex operation that requires:
  // 1. Finding the correct position to insert the diagonal
  // 2. Creating new half-edges
  // 3. Updating face connectivity
  // For now, return nullptr as a placeholder
  // TODO: Implement this properly
  return nullptr;
}

Face* DCEL::MergeFaces(HalfEdge* edge) {
  // This is a complex operation that requires:
  // 1. Removing the shared edge
  // 2. Merging the two face cycles
  // 3. Updating all pointers
  // For now, return nullptr as a placeholder
  // TODO: Implement this properly
  return nullptr;
}

// ========== Validation ==========

bool DCEL::Validate() const {
  // Check all half-edges have valid twins
  for (const auto& edge : half_edges_) {
    if (edge->GetTwin() == nullptr) {
      std::cerr << "Error: Half-edge " << edge->GetId() << " has no twin\n";
      return false;
    }
    if (edge->GetTwin()->GetTwin() != edge.get()) {
      std::cerr << "Error: Half-edge " << edge->GetId() 
                << " has invalid twin relationship\n";
      return false;
    }
  }
  
  // Check all vertices have valid incident edges
  for (const auto& vertex : vertices_) {
    if (vertex->GetIncidentEdge() == nullptr) {
      std::cerr << "Warning: Vertex " << vertex->GetId() 
                << " has no incident edge\n";
    }
  }
  
  // Check face boundaries are valid cycles
  for (const auto& face : faces_) {
    if (face->HasOuterComponent()) {
      HalfEdge* start = face->GetOuterComponent();
      HalfEdge* current = start;
      int count = 0;
      do {
        if (current == nullptr) {
          std::cerr << "Error: Face " << face->GetId() 
                    << " has broken boundary cycle\n";
          return false;
        }
        if (current->GetFace() != face.get()) {
          std::cerr << "Error: Half-edge " << current->GetId() 
                    << " has incorrect face pointer\n";
          return false;
        }
        current = current->GetNext();
        count++;
        if (count > 10000) {  // Prevent infinite loop
          std::cerr << "Error: Face " << face->GetId() 
                    << " has invalid boundary (too many edges)\n";
          return false;
        }
      } while (current != start);
    }
  }
  
  return true;
}

void DCEL::Clear() {
  vertices_.clear();
  half_edges_.clear();
  faces_.clear();
  next_vertex_id_ = 0;
  next_edge_id_ = 0;
  next_face_id_ = 0;
}

void DCEL::PrintStats() const {
  std::cout << "DCEL Statistics:\n";
  std::cout << "  Vertices: " << vertices_.size() << "\n";
  std::cout << "  Half-Edges: " << half_edges_.size() << "\n";
  std::cout << "  Undirected Edges: " << half_edges_.size() / 2 << "\n";
  std::cout << "  Faces: " << faces_.size() << "\n";
  
  // Count bounded faces
  int bounded_faces = 0;
  for (const auto& face : faces_) {
    if (!face->IsUnbounded()) {
      bounded_faces++;
    }
  }
  std::cout << "  Bounded Faces: " << bounded_faces << "\n";
  std::cout << "  Unbounded Faces: " << (faces_.size() - bounded_faces) << "\n";
}

}  // namespace geometry
