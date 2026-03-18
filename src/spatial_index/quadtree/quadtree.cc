#include "quadtree.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <limits>

namespace geometry {

// ============================================================================
// QuadTreeNode Implementation
// ============================================================================

QuadTreeNode::QuadTreeNode(const BoundingBox& bounds)
    : bounds_(bounds), children_(4) {}

bool QuadTreeNode::Insert(const Point2D& point, int capacity) {
  // Check if point is within bounds
  if (!bounds_.Contains(point)) {
    return false;
  }
  
  // If this is a leaf and has capacity, add the point
  if (IsLeaf()) {
    if (points_.size() < static_cast<size_t>(capacity)) {
      points_.push_back(point);
      return true;
    }
    
    // Subdivide if capacity exceeded
    Subdivide();
  }
  
  // Insert into appropriate child
  int child_idx = GetChildIndex(point);
  if (children_[child_idx]) {
    return children_[child_idx]->Insert(point, capacity);
  }
  
  return false;
}

bool QuadTreeNode::Remove(const Point2D& point) {
  // Check if point is in this node
  auto it = std::find(points_.begin(), points_.end(), point);
  if (it != points_.end()) {
    points_.erase(it);
    return true;
  }
  
  // If leaf, point not found
  if (IsLeaf()) {
    return false;
  }
  
  // Search in children
  int child_idx = GetChildIndex(point);
  if (children_[child_idx]) {
    return children_[child_idx]->Remove(point);
  }
  
  return false;
}

void QuadTreeNode::RangeQuery(const BoundingBox& range, 
                             std::vector<Point2D>& result) const {
  // If this node doesn't intersect the range, return
  if (!bounds_.Intersects(range)) {
    return;
  }
  
  // Check points in this node
  for (const auto& pt : points_) {
    if (range.Contains(pt)) {
      result.push_back(pt);
    }
  }
  
  // Recursively search children
  if (!IsLeaf()) {
    for (const auto& child : children_) {
      if (child) {
        child->RangeQuery(range, result);
      }
    }
  }
}

bool QuadTreeNode::NearestNeighbor(const Point2D& query, Point2D& nearest,
                                   double& min_dist) const {
  bool found = false;
  
  // Check points in this node
  for (const auto& pt : points_) {
    double dist = query.DistanceSquaredTo(pt);
    if (dist < min_dist) {
      min_dist = dist;
      nearest = pt;
      found = true;
    }
  }
  
  // If leaf, we're done
  if (IsLeaf()) {
    return found;
  }
  
  // Search children, starting with the closest one
  std::vector<std::pair<double, int>> child_order;
  for (int i = 0; i < 4; ++i) {
    if (children_[i]) {
      double dist = query.DistanceSquaredTo(children_[i]->bounds_.Center());
      child_order.push_back({dist, i});
    }
  }
  
  std::sort(child_order.begin(), child_order.end());
  
  for (const auto& item : child_order) {
    int idx = item.second;
    
    // Prune if this child's bounds are too far
    double min_possible_dist = 0.0;
    if (query.x < children_[idx]->bounds_.min_x) {
      double dx = children_[idx]->bounds_.min_x - query.x;
      min_possible_dist += dx * dx;
    } else if (query.x > children_[idx]->bounds_.max_x) {
      double dx = query.x - children_[idx]->bounds_.max_x;
      min_possible_dist += dx * dx;
    }
    
    if (query.y < children_[idx]->bounds_.min_y) {
      double dy = children_[idx]->bounds_.min_y - query.y;
      min_possible_dist += dy * dy;
    } else if (query.y > children_[idx]->bounds_.max_y) {
      double dy = query.y - children_[idx]->bounds_.max_y;
      min_possible_dist += dy * dy;
    }
    
    if (min_possible_dist >= min_dist) {
      continue;  // Prune this branch
    }
    
    Point2D child_nearest;
    double child_dist = min_dist;
    if (children_[idx]->NearestNeighbor(query, child_nearest, child_dist)) {
      if (child_dist < min_dist) {
        min_dist = child_dist;
        nearest = child_nearest;
        found = true;
      }
    }
  }
  
  return found;
}

bool QuadTreeNode::Contains(const Point2D& point) const {
  // Check points in this node
  if (std::find(points_.begin(), points_.end(), point) != points_.end()) {
    return true;
  }
  
  // If leaf, point not found
  if (IsLeaf()) {
    return false;
  }
  
  // Search in appropriate child
  int child_idx = GetChildIndex(point);
  return children_[child_idx] && children_[child_idx]->Contains(point);
}

std::vector<Point2D> QuadTreeNode::GetAllPoints() const {
  std::vector<Point2D> result;
  result.insert(result.end(), points_.begin(), points_.end());
  
  if (!IsLeaf()) {
    for (const auto& child : children_) {
      if (child) {
        auto child_points = child->GetAllPoints();
        result.insert(result.end(), child_points.begin(), child_points.end());
      }
    }
  }
  
  return result;
}

int QuadTreeNode::GetDepth() const {
  return GetDepthHelper(0);
}

int QuadTreeNode::GetNodeCount() const {
  int count = 1;
  GetNodeCountHelper(count);
  return count;
}

int QuadTreeNode::GetPointCount() const {
  int count = static_cast<int>(points_.size());
  
  if (!IsLeaf()) {
    for (const auto& child : children_) {
      if (child) {
        count += child->GetPointCount();
      }
    }
  }
  
  return count;
}

void QuadTreeNode::Clear() {
  points_.clear();
  for (auto& child : children_) {
    child.reset();
  }
}

void QuadTreeNode::Subdivide() {
  children_[0] = std::make_unique<QuadTreeNode>(bounds_.GetNW());
  children_[1] = std::make_unique<QuadTreeNode>(bounds_.GetNE());
  children_[2] = std::make_unique<QuadTreeNode>(bounds_.GetSW());
  children_[3] = std::make_unique<QuadTreeNode>(bounds_.GetSE());
  
  // Redistribute existing points to children
  for (const auto& pt : points_) {
    int child_idx = GetChildIndex(pt);
    if (children_[child_idx]) {
      children_[child_idx]->points_.push_back(pt);
    }
  }
  points_.clear();
}

int QuadTreeNode::GetChildIndex(const Point2D& point) const {
  Point2D center = bounds_.Center();
  
  if (point.y >= center.y) {
    return point.x < center.x ? NW : NE;
  } else {
    return point.x < center.x ? SW : SE;
  }
}

int QuadTreeNode::GetDepthHelper(int current_depth) const {
  if (IsLeaf()) {
    return current_depth;
  }
  
  int max_depth = current_depth;
  for (const auto& child : children_) {
    if (child) {
      int child_depth = child->GetDepthHelper(current_depth + 1);
      max_depth = std::max(max_depth, child_depth);
    }
  }
  
  return max_depth;
}

void QuadTreeNode::GetNodeCountHelper(int& count) const {
  if (!IsLeaf()) {
    for (const auto& child : children_) {
      if (child) {
        count++;
        child->GetNodeCountHelper(count);
      }
    }
  }
}

// ============================================================================
// Quadtree Implementation
// ============================================================================

Quadtree::Quadtree(const BoundingBox& bounds, int capacity)
    : bounds_(bounds), capacity_(capacity), auto_bounds_(false) {
  if (bounds_.Width() > 0 && bounds_.Height() > 0) {
    root_ = std::make_unique<QuadTreeNode>(bounds_);
  }
}

bool Quadtree::Insert(const Point2D& point) {
  // If no root, create one with auto-calculated bounds
  if (!root_) {
    if (auto_bounds_) {
      bounds_ = BoundingBox(point.x - 100, point.y - 100,
                           point.x + 100, point.y + 100);
      root_ = std::make_unique<QuadTreeNode>(bounds_);
    } else {
      return false;
    }
  }
  
  // Expand bounds if needed (auto-bounds mode)
  if (auto_bounds_ && !bounds_.Contains(point)) {
    ExpandBounds(point);
    Rebuild();
    return true;
  }
  
  return root_->Insert(point, capacity_);
}

void Quadtree::Insert(const std::vector<Point2D>& points) {
  for (const auto& pt : points) {
    Insert(pt);
  }
}

bool Quadtree::Remove(const Point2D& point) {
  return root_ && root_->Remove(point);
}

std::vector<Point2D> Quadtree::RangeQuery(const BoundingBox& range) const {
  std::vector<Point2D> result;
  if (root_) {
    root_->RangeQuery(range, result);
  }
  return result;
}

std::vector<Point2D> Quadtree::RadiusQuery(const Point2D& center, 
                                           double radius) const {
  BoundingBox range(center.x - radius, center.y - radius,
                   center.x + radius, center.y + radius);
  
  auto points = RangeQuery(range);
  
  // Filter by actual circular radius
  std::vector<Point2D> result;
  double radius_sq = radius * radius;
  
  for (const auto& pt : points) {
    if (center.DistanceSquaredTo(pt) <= radius_sq) {
      result.push_back(pt);
    }
  }
  
  return result;
}

bool Quadtree::NearestNeighbor(const Point2D& query, Point2D& nearest) const {
  if (!root_) {
    return false;
  }
  
  double min_dist = std::numeric_limits<double>::max();
  return root_->NearestNeighbor(query, nearest, min_dist);
}

std::vector<Point2D> Quadtree::KNearestNeighbors(const Point2D& query, 
                                                 int k) const {
  std::vector<Point2D> result;
  std::vector<double> distances;
  
  if (root_) {
    KNearestNeighborsHelper(root_.get(), query, k, result, distances);
  }
  
  return result;
}

bool Quadtree::Contains(const Point2D& point) const {
  return root_ && root_->Contains(point);
}

std::vector<Point2D> Quadtree::GetAllPoints() const {
  return root_ ? root_->GetAllPoints() : std::vector<Point2D>();
}

void Quadtree::Clear() {
  if (root_) {
    root_->Clear();
  }
}

void Quadtree::Rebuild() {
  auto all_points = GetAllPoints();
  root_ = std::make_unique<QuadTreeNode>(bounds_);
  Insert(all_points);
}

int Quadtree::GetDepth() const {
  return root_ ? root_->GetDepth() : 0;
}

int Quadtree::GetNodeCount() const {
  return root_ ? root_->GetNodeCount() : 0;
}

int Quadtree::GetPointCount() const {
  return root_ ? root_->GetPointCount() : 0;
}

BoundingBox Quadtree::CalculateBounds(const std::vector<Point2D>& points) {
  if (points.empty()) {
    return BoundingBox();
  }
  
  double min_x = points[0].x, max_x = points[0].x;
  double min_y = points[0].y, max_y = points[0].y;
  
  for (const auto& pt : points) {
    min_x = std::min(min_x, pt.x);
    max_x = std::max(max_x, pt.x);
    min_y = std::min(min_y, pt.y);
    max_y = std::max(max_y, pt.y);
  }
  
  // Add padding
  double padding = 10.0;
  return BoundingBox(min_x - padding, min_y - padding,
                    max_x + padding, max_y + padding);
}

void Quadtree::ExpandBounds(const Point2D& point) {
  if (bounds_.Width() == 0 || bounds_.Height() == 0) {
    bounds_ = BoundingBox(point.x - 100, point.y - 100,
                         point.x + 100, point.y + 100);
    return;
  }
  
  double padding = 50.0;
  bounds_.min_x = std::min(bounds_.min_x, point.x - padding);
  bounds_.max_x = std::max(bounds_.max_x, point.x + padding);
  bounds_.min_y = std::min(bounds_.min_y, point.y - padding);
  bounds_.max_y = std::max(bounds_.max_y, point.y + padding);
}

void Quadtree::KNearestNeighborsHelper(const QuadTreeNode* node,
                                       const Point2D& query,
                                       int k,
                                       std::vector<Point2D>& result,
                                       std::vector<double>& distances) const {
  if (!node) return;
  
  // Check points in this node
  for (const auto& pt : node->GetPoints()) {
    double dist = query.DistanceSquaredTo(pt);
    
    if (result.size() < static_cast<size_t>(k)) {
      result.push_back(pt);
      distances.push_back(dist);
      
      // Maintain heap property
      size_t idx = result.size() - 1;
      while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (distances[parent] >= distances[idx]) break;
        std::swap(result[parent], result[idx]);
        std::swap(distances[parent], distances[idx]);
        idx = parent;
      }
    } else if (dist < distances[0]) {
      // Replace max element
      result[0] = pt;
      distances[0] = dist;
      
      // Heapify down
      size_t idx = 0;
      while (true) {
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        size_t largest = idx;
        
        if (left < result.size() && distances[left] > distances[largest]) {
          largest = left;
        }
        if (right < result.size() && distances[right] > distances[largest]) {
          largest = right;
        }
        
        if (largest == idx) break;
        
        std::swap(result[idx], result[largest]);
        std::swap(distances[idx], distances[largest]);
        idx = largest;
      }
    }
  }
  
  // Recursively search children
  if (!node->IsLeaf()) {
    for (const auto& child : node->GetChildren()) {
      if (child) {
        KNearestNeighborsHelper(child.get(), query, k, result, distances);
      }
    }
  }
}

}  // namespace geometry
