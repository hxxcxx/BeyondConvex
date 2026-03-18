#pragma once

#include "kdtree_node.h"
#include <vector>
#include <memory>
#include <functional>

namespace geometry {

// KD-tree for 2D spatial indexing
class KDTree {
 public:
  // Constructor
  KDTree();
  
  // Insert a single point
  bool Insert(const Point2D& point);
  
  // Insert multiple points
  void Insert(const std::vector<Point2D>& points);
  
  // Build tree from points (more efficient than inserting one by one)
  void Build(const std::vector<Point2D>& points);
  
  // Remove a point (not implemented for simplicity)
  // bool Remove(const Point2D& point);
  
  // Check if point exists
  bool Contains(const Point2D& point) const;
  
  // Range query - find all points within bounding box
  std::vector<Point2D> RangeQuery(const BoundingBox& range) const;
  
  // Radius query - find all points within radius of center
  std::vector<Point2D> RadiusQuery(const Point2D& center, double radius) const;
  
  // Find nearest neighbor
  bool NearestNeighbor(const Point2D& query, Point2D& nearest) const;
  
  // Find k nearest neighbors
  std::vector<Point2D> KNearestNeighbors(const Point2D& query, int k) const;
  
  // Get all points
  std::vector<Point2D> GetAllPoints() const;
  
  // Clear the tree
  void Clear();
  
  // Statistics
  int GetSize() const { return size_; }
  int GetDepth() const;
  int GetNodeCount() const;
  bool IsEmpty() const { return root_ == nullptr; }
  
  // Get root (for visualization)
  const KDTreeNode* GetRoot() const { return root_.get(); }
  
 private:
  std::unique_ptr<KDTreeNode> root_;
  int size_;
  
  // Build helper - recursively build tree from sorted points
  std::unique_ptr<KDTreeNode> BuildHelper(
      std::vector<Point2D>& points, int start, int end, 
      KDTreeNode::SplitAxis axis);
  
  // Sort points by axis
  void SortByAxis(std::vector<Point2D>& points, int start, int end,
                  KDTreeNode::SplitAxis axis);
};

}  // namespace geometry
