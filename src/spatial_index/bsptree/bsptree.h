#pragma once

#include "../spatial_index_common.h"
#include <vector>
#include <memory>
#include <cmath>

namespace geometry {

// Forward declarations
class BSPNode;

// Plane definition for BSP tree (2D line)
struct Plane {
  Point2D point;      // Point on the plane
  Vector2D normal;    // Normal vector (normalized)
  
  Plane() : normal(1.0, 0.0) {}
  Plane(const Point2D& p, const Vector2D& n) : point(p), normal(n) {
    // Normalize the normal vector
    double length = std::sqrt(normal.x * normal.x + normal.y * normal.y);
    if (length > 1e-10) {
      normal.x /= length;
      normal.y /= length;
    }
  }
  
  // Calculate signed distance from point to plane
  double SignedDistance(const Point2D& p) const {
    return (p.x - point.x) * normal.x + (p.y - point.y) * normal.y;
  }
  
  // Classify point position relative to plane
  enum Classification {
    FRONT,      // Point is in front of plane (distance > 0)
    BACK,       // Point is behind plane (distance < 0)
    COPLANAR    // Point is on the plane (distance ≈ 0)
  };
  
  Classification Classify(const Point2D& p, double epsilon = 1e-10) const {
    double dist = SignedDistance(p);
    if (dist > epsilon) return FRONT;
    if (dist < -epsilon) return BACK;
    return COPLANAR;
  }
};

// BSP Tree Node
class BSPNode {
 public:
  BSPNode(const BoundingBox& bounds);
  
  // Insert a point into the tree
  bool Insert(const Point2D& point, int capacity);
  
  // Remove a point from the tree
  bool Remove(const Point2D& point);
  
  // Check if tree contains a point
  bool Contains(const Point2D& point) const;
  
  // Range query
  void RangeQuery(const BoundingBox& range, std::vector<Point2D>& result) const;
  
  // Radius query
  void RadiusQuery(const Point2D& center, double radius, 
                  std::vector<Point2D>& result) const;
  
  // Nearest neighbor search
  bool NearestNeighbor(const Point2D& query, Point2D& nearest, 
                      double& min_dist) const;
  
  // K-nearest neighbors search
  void KNearestNeighbors(const Point2D& query, int k,
                        std::vector<Point2D>& result,
                        std::vector<double>& distances) const;
  
  // Get all points in subtree
  std::vector<Point2D> GetAllPoints() const;
  
  // Tree information
  bool IsLeaf() const { return !front_child_ && !back_child_; }
  int GetDepth() const;
  int GetNodeCount() const;
  int GetPointCount() const;
  const BoundingBox& GetBounds() const { return bounds_; }
  const std::vector<Point2D>& GetPoints() const { return points_; }
  BSPNode* GetFrontChild() const { return front_child_.get(); }
  BSPNode* GetBackChild() const { return back_child_.get(); }
  const Plane& GetSplitPlane() const { return split_plane_; }
  
  // Clear the node
  void Clear();
  
 private:
  BoundingBox bounds_;
  Plane split_plane_;
  std::vector<Point2D> points_;
  std::unique_ptr<BSPNode> front_child_;
  std::unique_ptr<BSPNode> back_child_;
  
  // Subdivide the node
  void Subdivide();
  
  // Calculate optimal split plane
  Plane CalculateOptimalSplitPlane() const;
  
  // Split bounding box
  std::pair<BoundingBox, BoundingBox> SplitBounds(const Plane& plane) const;
  
  // Helper functions
  int GetDepthHelper(int current_depth) const;
  void GetNodeCountHelper(int& count) const;
};

// BSP Tree main class
class BSPTree {
 public:
  BSPTree(const BoundingBox& bounds, int capacity = 4);
  
  // Insert operations
  bool Insert(const Point2D& point);
  void Insert(const std::vector<Point2D>& points);
  
  // Remove operations
  bool Remove(const Point2D& point);
  
  // Query operations
  bool Contains(const Point2D& point) const;
  std::vector<Point2D> RangeQuery(const BoundingBox& range) const;
  std::vector<Point2D> RadiusQuery(const Point2D& center, double radius) const;
  bool NearestNeighbor(const Point2D& query, Point2D& nearest) const;
  std::vector<Point2D> KNearestNeighbors(const Point2D& query, int k) const;
  
  // Tree operations
  std::vector<Point2D> GetAllPoints() const;
  void Clear();
  void Rebuild();
  
  // Tree information
  int GetDepth() const;
  int GetNodeCount() const;
  int GetPointCount() const;
  const BoundingBox& GetBounds() const { return bounds_; }
  BSPNode* GetRoot() const { return root_.get(); }
  
 private:
  std::unique_ptr<BSPNode> root_;
  BoundingBox bounds_;
  int capacity_;
  
  // Helper for K-nearest neighbors
  void KNearestNeighborsHelper(const BSPNode* node,
                               const Point2D& query,
                               int k,
                               std::vector<Point2D>& result,
                               std::vector<double>& distances) const;
};

}  // namespace geometry
