#pragma once

#include "../../core/point2d.h"
#include "../spatial_index_common.h"
#include <memory>
#include <vector>

namespace geometry {

// KD-tree node for 2D points
class KDTreeNode {
 public:
  // Split axis: 0 for x-axis, 1 for y-axis
  enum SplitAxis { X_AXIS = 0, Y_AXIS = 1 };
  
  // Constructor
  KDTreeNode(const Point2D& point, SplitAxis axis);
  
  // Get the point stored in this node
  const Point2D& GetPoint() const { return point_; }
  
  // Get the split axis
  SplitAxis GetSplitAxis() const { return split_axis_; }
  
  // Get left and right children
  KDTreeNode* GetLeft() const { return left_.get(); }
  KDTreeNode* GetRight() const { return right_.get(); }
  
  // Check if leaf node
  bool IsLeaf() const { return left_ == nullptr && right_ == nullptr; }
  
  // Get next split axis (for building tree)
  SplitAxis GetNextAxis() const { return (split_axis_ == X_AXIS) ? Y_AXIS : X_AXIS; }
  
  // Insert a point (returns false if point already exists)
  bool Insert(const Point2D& point);
  
  // Search for exact point
  bool Contains(const Point2D& point) const;
  
  // Range query - find all points within bounding box
  void RangeQuery(const BoundingBox& range, std::vector<Point2D>& result) const;
  
  // Radius query - find all points within radius of center
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
  
  // Get tree depth
  int GetDepth() const;
  
  // Get node count
  int GetNodeCount() const;
  
  // Set children (for building)
  void SetLeft(std::unique_ptr<KDTreeNode> left) { left_ = std::move(left); }
  void SetRight(std::unique_ptr<KDTreeNode> right) { right_ = std::move(right); }
  
 private:
  Point2D point_;
  SplitAxis split_axis_;
  std::unique_ptr<KDTreeNode> left_;
  std::unique_ptr<KDTreeNode> right_;
  
  // Helper methods
  double GetSplitValue() const;
  bool IsInLeftSubtree(const Point2D& point) const;
  
  // Depth helper
  int GetDepthHelper(int current_depth) const;
};

}  // namespace geometry
