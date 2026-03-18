#pragma once

#include "quadtree_node.h"
#include <vector>
#include <memory>

namespace geometry {

// Main Quadtree class for spatial indexing
class Quadtree {
 public:
  // Constructor with optional bounds (auto-calculated if not provided)
  explicit Quadtree(const BoundingBox& bounds = BoundingBox(), 
                   int capacity = 4);
  
  // Insert a point into the quadtree
  bool Insert(const Point2D& point);
  
  // Insert multiple points
  void Insert(const std::vector<Point2D>& points);
  
  // Remove a point from the quadtree
  bool Remove(const Point2D& point);
  
  // Query all points within a bounding box
  std::vector<Point2D> RangeQuery(const BoundingBox& range) const;
  
  // Query all points within a circular radius
  std::vector<Point2D> RadiusQuery(const Point2D& center, double radius) const;
  
  // Find the nearest neighbor to a query point
  bool NearestNeighbor(const Point2D& query, Point2D& nearest) const;
  
  // Find k nearest neighbors
  std::vector<Point2D> KNearestNeighbors(const Point2D& query, int k) const;
  
  // Check if a point exists in the tree
  bool Contains(const Point2D& point) const;
  
  // Get all points in the tree
  std::vector<Point2D> GetAllPoints() const;
  
  // Clear all points
  void Clear();
  
  // Rebuild the tree with current points
  void Rebuild();
  
  // Get statistics
  int GetDepth() const;
  int GetNodeCount() const;
  int GetPointCount() const;
  int GetCapacity() const { return capacity_; }
  const BoundingBox& GetBounds() const { return bounds_; }
  
  // Get the root node (for visualization)
  const QuadTreeNode* GetRoot() const { return root_.get(); }
  
  // Set auto-bounds mode (automatically expand bounds when needed)
  void SetAutoBounds(bool auto_bounds) { auto_bounds_ = auto_bounds; }
  
  // Calculate bounds from a set of points (public for convenience)
  static BoundingBox CalculateBounds(const std::vector<Point2D>& points);
  
 private:
  std::unique_ptr<QuadTreeNode> root_;
  BoundingBox bounds_;
  int capacity_;
  bool auto_bounds_;
  
  // Expand bounds to include a point
  void ExpandBounds(const Point2D& point);
  
  // Helper for k-nearest neighbors
  void KNearestNeighborsHelper(const QuadTreeNode* node, const Point2D& query,
                              int k, std::vector<Point2D>& result,
                              std::vector<double>& distances) const;
};

}  // namespace geometry
