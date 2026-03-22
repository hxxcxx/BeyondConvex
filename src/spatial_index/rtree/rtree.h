#pragma once

#include "rtree_node.h"
#include <vector>
#include <memory>
#include <functional>

namespace geometry {

// R-tree for spatial indexing of rectangles
// Unlike KD-tree and Quadtree which are optimized for points,
// R-tree is designed for rectangle objects (polygons, bounding boxes, etc.)
class RTree {
 public:
  // Constructor
  // max_entries: maximum entries per node (default 4 for visualization)
  // min_entries: minimum entries per node (default 2, should be <= max_entries/2)
  explicit RTree(int max_entries = 4, int min_entries = 2);
  
  // Insert a rectangle with associated data ID
  bool Insert(const BoundingBox& bounds, int data_id);
  
  // Insert multiple rectangles
  void Insert(const std::vector<std::pair<BoundingBox, int>>& rectangles);
  
  // Remove a rectangle (by data ID)
  bool Remove(int data_id);
  
  // Range query - find all rectangles that intersect with query range
  std::vector<int> RangeQuery(const BoundingBox& range) const;
  
  // Point query - find all rectangles containing a point
  std::vector<int> PointQuery(const Point2D& point) const;
  
  // Find all rectangles that intersect with a given rectangle
  std::vector<int> IntersectionQuery(const BoundingBox& bounds) const;
  
  // Check if a data ID exists
  bool Contains(int data_id);
  
  // Get all data IDs
  std::vector<int> GetAllData() const;
  
  // Get all rectangles with their IDs
  std::vector<std::pair<BoundingBox, int>> GetAllRectangles() const;
  
  // Clear the tree
  void Clear();
  
  // Rebuild the tree with current data
  void Rebuild();
  
  // Statistics
  int GetSize() const { return size_; }
  int GetDepth() const;
  int GetNodeCount() const;
  int GetHeight() const;
  bool IsEmpty() const { return root_ == nullptr; }
  
  // Get root (for visualization)
  const RTreeNode* GetRoot() const { return root_.get(); }
  
  // Get parameters
  int GetMaxEntries() const { return max_entries_; }
  int GetMinEntries() const { return min_entries_; }
  
  // Calculate bounds from a set of rectangles
  static BoundingBox CalculateBounds(
      const std::vector<std::pair<BoundingBox, int>>& rectangles);
  
 private:
  std::unique_ptr<RTreeNode> root_;
  int size_;
  int max_entries_;
  int min_entries_;
  int next_data_id_;  // For auto-generating IDs if not provided
  
  // Insert helper
  bool InsertHelper(RTreeNode* node, const BoundingBox& bounds, int data_id, 
                   int depth, bool& split);
  
  // Split helper
  std::unique_ptr<RTreeNode> SplitNode(RTreeNode* node);
  
  // Adjust tree after insertion/split
  void AdjustTree(RTreeNode* node, std::unique_ptr<RTreeNode> new_node);
  
  // Query helpers
  void RangeQueryHelper(const RTreeNode* node, const BoundingBox& range,
                       std::vector<int>& result) const;
  
  void PointQueryHelper(const RTreeNode* node, const Point2D& point,
                       std::vector<int>& result) const;
  
  // Find leaf node containing data ID
  RTreeNode* FindLeaf(RTreeNode* node, int data_id, 
                     std::vector<RTreeNode*>& path);
  
  // Condense tree after deletion
  void CondenseTree(std::vector<RTreeNode*>& path);
  
  // Depth helper
  int GetDepthHelper(const RTreeNode* node) const;
  
  // Node count helper
  int GetNodeCountHelper(const RTreeNode* node) const;
  
  // Collect all rectangles
  void CollectRectangles(const RTreeNode* node,
                        std::vector<std::pair<BoundingBox, int>>& result) const;
};

}  // namespace geometry
