#pragma once

#include "../core/point2d.h"
#include <memory>

namespace geometry {

// Forward declarations
class HalfEdge;

/**
 * @brief Vertex in a DCEL structure
 * 
 * A vertex represents a point in the planar subdivision.
 * It maintains a pointer to one of the half-edges that originates from it.
 */
class Vertex {
 public:
  // Constructors
  Vertex() : id_(-1) {}
  
  explicit Vertex(const Point2D& coordinates, int id = -1)
      : coordinates_(coordinates), id_(id), incident_edge_(nullptr) {}
  
  // Getters
  const Point2D& GetCoordinates() const { return coordinates_; }
  Point2D& GetCoordinates() { return coordinates_; }
  
  int GetId() const { return id_; }
  void SetId(int id) { id_ = id; }
  
  HalfEdge* GetIncidentEdge() const { return incident_edge_; }
  void SetIncidentEdge(HalfEdge* edge) { incident_edge_ = edge; }
  
  // Utility functions
  bool HasIncidentEdge() const { return incident_edge_ != nullptr; }
  
  // Compute degree (number of edges incident to this vertex)
  int Degree() const;
  
  // Check if this vertex is on the boundary of the subdivision
  bool IsOnBoundary() const;
  
 private:
  Point2D coordinates_;      // Position of the vertex
  int id_;                   // Unique identifier
  HalfEdge* incident_edge_;  // One half-edge originating from this vertex
};

}  // namespace geometry
