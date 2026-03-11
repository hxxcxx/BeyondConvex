#include "vertex.h"
#include "half_edge.h"

namespace geometry {

int Vertex::Degree() const {
  if (!HasIncidentEdge()) {
    return 0;
  }
  
  int degree = 0;
  HalfEdge* edge = incident_edge_;
  
  // Count all edges originating from this vertex
  // by traversing around the vertex
  do {
    degree++;
    if (edge->GetTwin() && edge->GetTwin()->GetNext()) {
      edge = edge->GetTwin()->GetNext();
    } else {
      break;
    }
  } while (edge != incident_edge_ && edge != nullptr);
  
  return degree;
}

bool Vertex::IsOnBoundary() const {
  if (!HasIncidentEdge()) {
    return false;
  }
  
  HalfEdge* edge = incident_edge_;
  
  // Check if any incident edge is on the boundary
  do {
    if (edge->IsOnBoundary()) {
      return true;
    }
    if (edge->GetTwin() && edge->GetTwin()->GetNext()) {
      edge = edge->GetTwin()->GetNext();
    } else {
      break;
    }
  } while (edge != incident_edge_ && edge != nullptr);
  
  return false;
}

}  // namespace geometry
