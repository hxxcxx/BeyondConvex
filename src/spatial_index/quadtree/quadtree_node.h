#ifndef GEOMETRY_SPATIAL_INDEX_QUADTREE_QUADTREE_NODE_H_
#define GEOMETRY_SPATIAL_INDEX_QUADTREE_QUADTREE_NODE_H_

#include "../../core/point2d.h"
#include <vector>
#include <memory>

namespace geometry {

// Axis-aligned bounding box for quadtree regions
struct BoundingBox {
  double min_x, min_y;
  double max_x, max_y;
  
  BoundingBox() : min_x(0), min_y(0), max_x(0), max_y(0) {}
  BoundingBox(double min_x, double min_y, double max_x, double max_y)
      : min_x(min_x), min_y(min_y), max_x(max_x), max_y(max_y) {}
  
  double Width() const { return max_x - min_x; }
  double Height() const { return max_y - min_y; }
  double Area() const { return Width() * Height(); }
  
  Point2D Center() const {
    return Point2D((min_x + max_x) / 2.0, (min_y + max_y) / 2.0);
  }
  
  bool Contains(const Point2D& point) const {
    return point.x >= min_x && point.x <= max_x &&
           point.y >= min_y && point.y <= max_y;
  }
  
  bool Intersects(const BoundingBox& other) const {
    return !(other.max_x < min_x || other.min_x > max_x ||
             other.max_y < min_y || other.min_y > max_y);
  }
  
  // Get the four quadrants
  BoundingBox GetNW() const {
    return BoundingBox(min_x, Center().y, Center().x, max_y);
  }
  BoundingBox GetNE() const {
    return BoundingBox(Center().x, Center().y, max_x, max_y);
  }
  BoundingBox GetSW() const {
    return BoundingBox(min_x, min_y, Center().x, Center().y);
  }
  BoundingBox GetSE() const {
    return BoundingBox(Center().x, min_y, max_x, Center().y);
  }
};

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

#endif  // GEOMETRY_SPATIAL_INDEX_QUADTREE_QUADTREE_NODE_H_
