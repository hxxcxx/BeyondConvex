#pragma once

#include "../spatial_index_common.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <queue>
#include <cmath>

namespace geometry {

// BVH Node - Bounding Volume Hierarchy Node
class BVHNode {
 public:
  explicit BVHNode(const BoundingBox& bounds);
  BVHNode(const BoundingBox& bounds, const std::vector<Point2D>& points);
  
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
  
  // Create bounding box from points (public for BVHTree::Build)
  static BoundingBox CalculateBounds(const std::vector<Point2D>& points);
  
  // Tree information
  bool IsLeaf() const { return left_ == nullptr && right_ == nullptr; }
  int GetDepth() const;
  int GetNodeCount() const;
  int GetPointCount() const;
  const BoundingBox& GetBounds() const { return bounds_; }
  const std::vector<Point2D>& GetPoints() const { return points_; }
  BVHNode* GetLeft() const { return left_.get(); }
  BVHNode* GetRight() const { return right_.get(); }
  
  // Set children (for building)
  void SetLeft(std::unique_ptr<BVHNode> left) { left_ = std::move(left); }
  void SetRight(std::unique_ptr<BVHNode> right) { right_ = std::move(right); }
  
  // Clear the node
  void Clear();
  
 private:
  BoundingBox bounds_;
  std::vector<Point2D> points_;
  std::unique_ptr<BVHNode> left_;
  std::unique_ptr<BVHNode> right_;
  
  // Subdivide the node
  void Subdivide();
  
  // Calculate optimal split axis and position
  struct SplitInfo {
    int axis;  // 0 for x-axis, 1 for y-axis
    double position;
    double cost;
  };
  SplitInfo CalculateOptimalSplit() const;
  
  // Split points into left and right based on split plane
  std::pair<std::vector<Point2D>, std::vector<Point2D>> 
  SplitPoints(const SplitInfo& split_info) const;
  
  // Helper functions
  int GetDepthHelper(int current_depth) const;
  void GetNodeCountHelper(int& count) const;
};

// BVH Tree main class
class BVHTree {
 public:
  BVHTree(const BoundingBox& bounds = BoundingBox(), int capacity = 4);
  
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
  BVHNode* GetRoot() const { return root_.get(); }
  
  // Calculate SAH cost (static method for use in BVHNode)
  static double CalculateSAHCost(const BoundingBox& parent_bounds,
                                const BoundingBox& left_bounds,
                                const BoundingBox& right_bounds,
                                int left_count, int right_count);
  
  // Build tree from points (more efficient)
  void Build(const std::vector<Point2D>& points);
  
 private:
  std::unique_ptr<BVHNode> root_;
  BoundingBox bounds_;
  int capacity_;
  
  // Build helper - recursively build tree from points
  std::unique_ptr<BVHNode> BuildHelper(
      std::vector<Point2D>& points, int start, int end, 
      const BoundingBox& current_bounds, int depth);
  
  // Sort points by axis
  void SortByAxis(std::vector<Point2D>& points, int start, int end, int axis);
};

}  // namespace geometry