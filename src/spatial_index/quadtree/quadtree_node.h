#pragma once

#include "../../core/point2d.h"
#include "../spatial_index_common.h"
#include <vector>
#include <memory>

namespace geometry {

// Quadtree node class
class QuadTreeNode {
 public:
  // Child quadrant indices
  enum ChildIndex {
    NW = 0,  // Northwest
    NE = 1,  // Northeast
    SW = 2,  // Southwest
    SE = 3   // Southeast
  };
  
  explicit QuadTreeNode(const BoundingBox& bounds);
  
  // Insert a point into this node
  bool Insert(const Point2D& point, int capacity = 4);
  
  // Remove a point from this node
  bool Remove(const Point2D& point);
  
  // Query points within a bounding box
  void RangeQuery(const BoundingBox& range, std::vector<Point2D>& result) const;
  
  // Find the nearest neighbor to a point
  bool NearestNeighbor(const Point2D& query, Point2D& nearest, 
                      double& min_dist) const;
  
  // Check if a point exists in the tree
  bool Contains(const Point2D& point) const;
  
  // Get all points in this node and its children
  std::vector<Point2D> GetAllPoints() const;
  
  // Get node statistics
  int GetDepth() const;
  int GetNodeCount() const;
  int GetPointCount() const;
  
  // Clear all points and children
  void Clear();
  
  // Getters
  const BoundingBox& GetBounds() const { return bounds_; }
  bool IsLeaf() const { return children_[0] == nullptr; }
  const std::vector<Point2D>& GetPoints() const { return points_; }
  const std::vector<std::unique_ptr<QuadTreeNode>>& GetChildren() const { 
    return children_; 
  }
  
 private:
  BoundingBox bounds_;
  std::vector<Point2D> points_;
  std::vector<std::unique_ptr<QuadTreeNode>> children_;
  
  // Subdivide this node into four children
  void Subdivide();
  
  // Get the appropriate child index for a point
  int GetChildIndex(const Point2D& point) const;
  
  // Helper for depth calculation
  int GetDepthHelper(int current_depth) const;
  
  // Helper for node count
  void GetNodeCountHelper(int& count) const;
};

}  // namespace geometry
