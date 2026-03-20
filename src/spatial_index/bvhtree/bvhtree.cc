#include "bvhtree.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <limits>
#include <set>
#include <functional>

namespace geometry {

// ============================================================================
// BVHNode Implementation
// ============================================================================

BVHNode::BVHNode(const BoundingBox& bounds)
    : bounds_(bounds) {}

BVHNode::BVHNode(const BoundingBox& bounds, const std::vector<Point2D>& points)
    : bounds_(bounds), points_(points) {}

bool BVHNode::Insert(const Point2D& point, int capacity) {
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
  
  // Insert into appropriate child based on bounds
  if (left_ && left_->GetBounds().Contains(point)) {
    return left_->Insert(point, capacity);
  } else if (right_ && right_->GetBounds().Contains(point)) {
    return right_->Insert(point, capacity);
  }
  
  // If point is on the boundary, insert into the closer child
  double left_dist = left_ ? bounds_.Center().DistanceSquaredTo(left_->GetBounds().Center()) : std::numeric_limits<double>::max();
  double right_dist = right_ ? bounds_.Center().DistanceSquaredTo(right_->GetBounds().Center()) : std::numeric_limits<double>::max();
  
  if (left_dist < right_dist && left_) {
    return left_->Insert(point, capacity);
  } else if (right_) {
    return right_->Insert(point, capacity);
  }
  
  return false;
}

bool BVHNode::Remove(const Point2D& point) {
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
  bool removed = false;
  if (left_ && left_->GetBounds().Contains(point)) {
    removed = left_->Remove(point);
  }
  if (!removed && right_ && right_->GetBounds().Contains(point)) {
    removed = right_->Remove(point);
  }
  
  return removed;
}

void BVHNode::RangeQuery(const BoundingBox& range, 
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
    if (left_) {
      left_->RangeQuery(range, result);
    }
    if (right_) {
      right_->RangeQuery(range, result);
    }
  }
}

void BVHNode::RadiusQuery(const Point2D& center, double radius, 
                         std::vector<Point2D>& result) const {
  double radius_sq = radius * radius;
  
  // Check if this node's bounds are too far
  double min_dist_sq = 0.0;
  if (center.x < bounds_.min_x) {
    double dx = bounds_.min_x - center.x;
    min_dist_sq += dx * dx;
  } else if (center.x > bounds_.max_x) {
    double dx = center.x - bounds_.max_x;
    min_dist_sq += dx * dx;
  }
  
  if (center.y < bounds_.min_y) {
    double dy = bounds_.min_y - center.y;
    min_dist_sq += dy * dy;
  } else if (center.y > bounds_.max_y) {
    double dy = center.y - bounds_.max_y;
    min_dist_sq += dy * dy;
  }
  
  if (min_dist_sq > radius_sq) {
    return;  // Node is too far
  }
  
  // Check points in this node
  for (const auto& pt : points_) {
    double dist_sq = center.DistanceSquaredTo(pt);
    if (dist_sq <= radius_sq) {
      result.push_back(pt);
    }
  }
  
  // Recursively search children
  if (!IsLeaf()) {
    if (left_) {
      left_->RadiusQuery(center, radius, result);
    }
    if (right_) {
      right_->RadiusQuery(center, radius, result);
    }
  }
}

bool BVHNode::NearestNeighbor(const Point2D& query, Point2D& nearest,
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
  
  // Determine order to search children (closest first)
  std::pair<BVHNode*, BVHNode*> children_order;
  if (left_ && right_) {
    double left_dist = query.DistanceSquaredTo(left_->GetBounds().Center());
    double right_dist = query.DistanceSquaredTo(right_->GetBounds().Center());
    
    if (left_dist < right_dist) {
      children_order = {left_.get(), right_.get()};
    } else {
      children_order = {right_.get(), left_.get()};
    }
  } else if (left_) {
    children_order = {left_.get(), nullptr};
  } else if (right_) {
    children_order = {right_.get(), nullptr};
  } else {
    return found;
  }
  
  // Search first child
  if (children_order.first) {
    double dist_to_child = 0.0;
    const BoundingBox& child_bounds = children_order.first->GetBounds();
    
    if (query.x < child_bounds.min_x) {
      double dx = child_bounds.min_x - query.x;
      dist_to_child += dx * dx;
    } else if (query.x > child_bounds.max_x) {
      double dx = query.x - child_bounds.max_x;
      dist_to_child += dx * dx;
    }
    
    if (query.y < child_bounds.min_y) {
      double dy = child_bounds.min_y - query.y;
      dist_to_child += dy * dy;
    } else if (query.y > child_bounds.max_y) {
      double dy = query.y - child_bounds.max_y;
      dist_to_child += dy * dy;
    }
    
    if (dist_to_child < min_dist) {
      Point2D child_nearest;
      double child_dist = min_dist;
      if (children_order.first->NearestNeighbor(query, child_nearest, child_dist)) {
        if (child_dist < min_dist) {
          min_dist = child_dist;
          nearest = child_nearest;
          found = true;
        }
      }
    }
  }
  
  // Search second child if needed
  if (children_order.second) {
    double dist_to_child = 0.0;
    const BoundingBox& child_bounds = children_order.second->GetBounds();
    
    if (query.x < child_bounds.min_x) {
      double dx = child_bounds.min_x - query.x;
      dist_to_child += dx * dx;
    } else if (query.x > child_bounds.max_x) {
      double dx = query.x - child_bounds.max_x;
      dist_to_child += dx * dx;
    }
    
    if (query.y < child_bounds.min_y) {
      double dy = child_bounds.min_y - query.y;
      dist_to_child += dy * dy;
    } else if (query.y > child_bounds.max_y) {
      double dy = query.y - child_bounds.max_y;
      dist_to_child += dy * dy;
    }
    
    if (dist_to_child < min_dist) {
      Point2D child_nearest;
      double child_dist = min_dist;
      if (children_order.second->NearestNeighbor(query, child_nearest, child_dist)) {
        if (child_dist < min_dist) {
          min_dist = child_dist;
          nearest = child_nearest;
          found = true;
        }
      }
    }
  }
  
  return found;
}

void BVHNode::KNearestNeighbors(const Point2D& query, int k,
                               std::vector<Point2D>& result,
                               std::vector<double>& distances) const {
  auto cmp = [](const std::pair<double, Point2D>& a, 
                const std::pair<double, Point2D>& b) {
    return a.first < b.first;
  };
  
  std::vector<std::pair<double, Point2D>> heap;
  
  std::function<void(const BVHNode*)> search = [&](const BVHNode* node) {
    if (!node) return;
    
    // Check points in this node
    for (const auto& pt : node->GetPoints()) {
      double dist = query.DistanceSquaredTo(pt);
      
      if (heap.size() < static_cast<size_t>(k)) {
        heap.push_back({dist, pt});
        std::push_heap(heap.begin(), heap.end(), cmp);
      } else if (dist < heap[0].first) {
        std::pop_heap(heap.begin(), heap.end(), cmp);
        heap.back() = {dist, pt};
        std::push_heap(heap.begin(), heap.end(), cmp);
      }
    }
    
    // Recursively search children
    if (!node->IsLeaf()) {
      if (node->GetLeft()) {
        search(node->GetLeft());
      }
      if (node->GetRight()) {
        search(node->GetRight());
      }
    }
  };
  
  search(this);
  
  // Extract results
  result.clear();
  distances.clear();
  
  while (!heap.empty()) {
    result.push_back(heap[0].second);
    distances.push_back(heap[0].first);
    std::pop_heap(heap.begin(), heap.end(), cmp);
    heap.pop_back();
  }
  
  std::reverse(result.begin(), result.end());
  std::reverse(distances.begin(), distances.end());
}

bool BVHNode::Contains(const Point2D& point) const {
  // Check points in this node
  if (std::find(points_.begin(), points_.end(), point) != points_.end()) {
    return true;
  }
  
  // If leaf, point not found
  if (IsLeaf()) {
    return false;
  }
  
  // Search in children
  if (left_ && left_->GetBounds().Contains(point)) {
    return left_->Contains(point);
  }
  if (right_ && right_->GetBounds().Contains(point)) {
    return right_->Contains(point);
  }
  
  return false;
}

std::vector<Point2D> BVHNode::GetAllPoints() const {
  std::vector<Point2D> result;
  result.insert(result.end(), points_.begin(), points_.end());
  
  if (!IsLeaf()) {
    if (left_) {
      auto left_points = left_->GetAllPoints();
      result.insert(result.end(), left_points.begin(), left_points.end());
    }
    if (right_) {
      auto right_points = right_->GetAllPoints();
      result.insert(result.end(), right_points.begin(), right_points.end());
    }
  }
  
  return result;
}

int BVHNode::GetDepth() const {
  return GetDepthHelper(0);
}

int BVHNode::GetNodeCount() const {
  int count = 1;
  GetNodeCountHelper(count);
  return count;
}

int BVHNode::GetPointCount() const {
  int count = static_cast<int>(points_.size());
  
  if (!IsLeaf()) {
    if (left_) {
      count += left_->GetPointCount();
    }
    if (right_) {
      count += right_->GetPointCount();
    }
  }
  
  return count;
}

void BVHNode::Clear() {
  points_.clear();
  left_.reset();
  right_.reset();
}

void BVHNode::Subdivide() {
  SplitInfo split_info = CalculateOptimalSplit();
  
  if (split_info.axis == -1) {
    // No valid split found, split by midpoint
    split_info.axis = (bounds_.Width() > bounds_.Height()) ? 0 : 1;
    split_info.position = (split_info.axis == 0) ? 
      bounds_.min_x + bounds_.Width() / 2.0 :
      bounds_.min_y + bounds_.Height() / 2.0;
  }
  
  auto [left_points, right_points] = SplitPoints(split_info);
  
  // Create bounding boxes for children
  BoundingBox left_bounds, right_bounds;
  
  if (split_info.axis == 0) {  // Split along x-axis
    left_bounds = BoundingBox(bounds_.min_x, bounds_.min_y, 
                             split_info.position, bounds_.max_y);
    right_bounds = BoundingBox(split_info.position, bounds_.min_y, 
                              bounds_.max_x, bounds_.max_y);
  } else {  // Split along y-axis
    left_bounds = BoundingBox(bounds_.min_x, bounds_.min_y, 
                             bounds_.max_x, split_info.position);
    right_bounds = BoundingBox(bounds_.min_x, split_info.position, 
                              bounds_.max_x, bounds_.max_y);
  }
  
  // Create child nodes
  left_ = std::make_unique<BVHNode>(left_bounds, left_points);
  right_ = std::make_unique<BVHNode>(right_bounds, right_points);
  
  // Clear points in this node (they're now in children)
  points_.clear();
}

BVHNode::SplitInfo BVHNode::CalculateOptimalSplit() const {
  if (points_.size() < 2) {
    return {-1, 0.0, std::numeric_limits<double>::max()};
  }
  
  SplitInfo best_split;
  best_split.axis = -1;
  best_split.cost = std::numeric_limits<double>::max();
  
  // Try splitting on both axes
  for (int axis = 0; axis < 2; ++axis) {
    // Sort points by axis
    std::vector<Point2D> sorted_points = points_;
    if (axis == 0) {
      std::sort(sorted_points.begin(), sorted_points.end(), 
               [](const Point2D& a, const Point2D& b) { return a.x < b.x; });
    } else {
      std::sort(sorted_points.begin(), sorted_points.end(), 
               [](const Point2D& a, const Point2D& b) { return a.y < b.y; });
    }
    
    // Try splitting at each point position
    for (size_t i = 1; i < sorted_points.size(); ++i) {
      double split_pos = (axis == 0) ? 
        (sorted_points[i].x + sorted_points[i-1].x) / 2.0 :
        (sorted_points[i].y + sorted_points[i-1].y) / 2.0;
      
      // Calculate bounds for children
      BoundingBox left_bounds = bounds_;
      BoundingBox right_bounds = bounds_;
      
      if (axis == 0) {
        left_bounds.max_x = split_pos;
        right_bounds.min_x = split_pos;
      } else {
        left_bounds.max_y = split_pos;
        right_bounds.min_y = split_pos;
      }
      
      // Calculate SAH cost
      double cost = BVHTree::CalculateSAHCost(bounds_, left_bounds, right_bounds, 
                                    i, sorted_points.size() - i);
      
      if (cost < best_split.cost) {
        best_split.axis = axis;
        best_split.position = split_pos;
        best_split.cost = cost;
      }
    }
  }
  
  return best_split;
}

std::pair<std::vector<Point2D>, std::vector<Point2D>> 
BVHNode::SplitPoints(const SplitInfo& split_info) const {
  std::vector<Point2D> left_points;
  std::vector<Point2D> right_points;
  
  for (const auto& pt : points_) {
    double value = (split_info.axis == 0) ? pt.x : pt.y;
    
    if (value < split_info.position) {
      left_points.push_back(pt);
    } else {
      right_points.push_back(pt);
    }
  }
  
  return {left_points, right_points};
}

BoundingBox BVHNode::CalculateBounds(const std::vector<Point2D>& points) {
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
  
  return BoundingBox(min_x, min_y, max_x, max_y);
}

int BVHNode::GetDepthHelper(int current_depth) const {
  if (IsLeaf()) {
    return current_depth;
  }
  
  int max_depth = current_depth;
  if (left_) {
    int left_depth = left_->GetDepthHelper(current_depth + 1);
    max_depth = std::max(max_depth, left_depth);
  }
  if (right_) {
    int right_depth = right_->GetDepthHelper(current_depth + 1);
    max_depth = std::max(max_depth, right_depth);
  }
  
  return max_depth;
}

void BVHNode::GetNodeCountHelper(int& count) const {
  if (!IsLeaf()) {
    if (left_) {
      count++;
      left_->GetNodeCountHelper(count);
    }
    if (right_) {
      count++;
      right_->GetNodeCountHelper(count);
    }
  }
}

// ============================================================================
// BVHTree Implementation
// ============================================================================

BVHTree::BVHTree(const BoundingBox& bounds, int capacity)
    : bounds_(bounds), capacity_(capacity) {
  if (bounds_.Width() > 0 && bounds_.Height() > 0) {
    root_ = std::make_unique<BVHNode>(bounds_);
  }
}

bool BVHTree::Insert(const Point2D& point) {
  if (!root_) {
    root_ = std::make_unique<BVHNode>(bounds_);
  }
  
  // If point is outside bounds, expand bounds and rebuild
  if (!bounds_.Contains(point)) {
    double padding = 10.0;
    bounds_.min_x = std::min(bounds_.min_x, point.x - padding);
    bounds_.max_x = std::max(bounds_.max_x, point.x + padding);
    bounds_.min_y = std::min(bounds_.min_y, point.y - padding);
    bounds_.max_y = std::max(bounds_.max_y, point.y + padding);
    
    Rebuild();
    return true;
  }
  
  return root_->Insert(point, capacity_);
}

void BVHTree::Insert(const std::vector<Point2D>& points) {
  for (const auto& pt : points) {
    Insert(pt);
  }
}

bool BVHTree::Remove(const Point2D& point) {
  return root_ && root_->Remove(point);
}

bool BVHTree::Contains(const Point2D& point) const {
  return root_ && root_->Contains(point);
}

std::vector<Point2D> BVHTree::RangeQuery(const BoundingBox& range) const {
  std::vector<Point2D> result;
  if (root_) {
    root_->RangeQuery(range, result);
  }
  return result;
}

std::vector<Point2D> BVHTree::RadiusQuery(const Point2D& center, 
                                         double radius) const {
  std::vector<Point2D> result;
  if (root_) {
    root_->RadiusQuery(center, radius, result);
  }
  return result;
}

bool BVHTree::NearestNeighbor(const Point2D& query, Point2D& nearest) const {
  if (!root_) {
    return false;
  }
  
  double min_dist = std::numeric_limits<double>::max();
  return root_->NearestNeighbor(query, nearest, min_dist);
}

std::vector<Point2D> BVHTree::KNearestNeighbors(const Point2D& query, 
                                                int k) const {
  std::vector<Point2D> result;
  std::vector<double> distances;
  
  if (root_) {
    root_->KNearestNeighbors(query, k, result, distances);
  }
  
  return result;
}

std::vector<Point2D> BVHTree::GetAllPoints() const {
  return root_ ? root_->GetAllPoints() : std::vector<Point2D>();
}

void BVHTree::Clear() {
  if (root_) {
    root_->Clear();
  }
}

void BVHTree::Rebuild() {
  auto all_points = GetAllPoints();
  root_.reset();
  
  if (bounds_.Width() > 0 && bounds_.Height() > 0) {
    root_ = std::make_unique<BVHNode>(bounds_);
  }
  
  if (!all_points.empty()) {
    Build(all_points);
  }
}

int BVHTree::GetDepth() const {
  return root_ ? root_->GetDepth() : 0;
}

int BVHTree::GetNodeCount() const {
  return root_ ? root_->GetNodeCount() : 0;
}

int BVHTree::GetPointCount() const {
  return root_ ? root_->GetPointCount() : 0;
}

void BVHTree::Build(const std::vector<Point2D>& points) {
  if (points.empty()) {
    root_.reset();
    return;
  }
  
  // Calculate bounds if not set
  if (bounds_.Width() == 0 || bounds_.Height() == 0) {
    bounds_ = BVHNode::CalculateBounds(points);
  }
  
  // Create a copy of points for modification
  std::vector<Point2D> points_copy = points;
  root_ = BuildHelper(points_copy, 0, points_copy.size(), bounds_, 0);
}

std::unique_ptr<BVHNode> BVHTree::BuildHelper(
    std::vector<Point2D>& points, int start, int end, 
    const BoundingBox& current_bounds, int depth) {
  int count = end - start;
  
  if (count == 0) {
    return nullptr;
  }
  
  if (count <= capacity_ || depth > 20) {
    // Create leaf node
    std::vector<Point2D> node_points;
    for (int i = start; i < end; ++i) {
      node_points.push_back(points[i]);
    }
    return std::make_unique<BVHNode>(current_bounds, node_points);
  }
  
  // Find best split using SAH
  int best_axis = -1;
  int best_split = -1;
  double best_cost = std::numeric_limits<double>::max();
  
  for (int axis = 0; axis < 2; ++axis) {
    SortByAxis(points, start, end, axis);
    
    for (int i = start + 1; i < end; ++i) {
      double split_pos = (axis == 0) ? 
        (points[i].x + points[i-1].x) / 2.0 :
        (points[i].y + points[i-1].y) / 2.0;
      
      BoundingBox left_bounds = current_bounds;
      BoundingBox right_bounds = current_bounds;
      
      if (axis == 0) {
        left_bounds.max_x = split_pos;
        right_bounds.min_x = split_pos;
      } else {
        left_bounds.max_y = split_pos;
        right_bounds.min_y = split_pos;
      }
      
      int left_count = i - start;
      int right_count = end - i;
      
      double cost = BVHTree::CalculateSAHCost(current_bounds, left_bounds, right_bounds,
                                        left_count, right_count);      if (cost < best_cost) {
        best_cost = cost;
        best_axis = axis;
        best_split = i;
      }
    }
  }
  
  // If no good split found, split at midpoint
  if (best_axis == -1) {
    best_axis = (current_bounds.Width() > current_bounds.Height()) ? 0 : 1;
    SortByAxis(points, start, end, best_axis);
    best_split = start + (end - start) / 2;
  } else {
    SortByAxis(points, start, end, best_axis);
  }
  
  // Calculate split position
  double split_pos = (best_axis == 0) ? 
    (points[best_split].x + points[best_split-1].x) / 2.0 :
    (points[best_split].y + points[best_split-1].y) / 2.0;
  
  // Create child bounds
  BoundingBox left_bounds, right_bounds;
  
  if (best_axis == 0) {
    left_bounds = BoundingBox(current_bounds.min_x, current_bounds.min_y,
                             split_pos, current_bounds.max_y);
    right_bounds = BoundingBox(split_pos, current_bounds.min_y,
                              current_bounds.max_x, current_bounds.max_y);
  } else {
    left_bounds = BoundingBox(current_bounds.min_x, current_bounds.min_y,
                             current_bounds.max_x, split_pos);
    right_bounds = BoundingBox(current_bounds.min_x, split_pos,
                              current_bounds.max_x, current_bounds.max_y);
  }
  
  // Recursively build children
  auto left_child = BuildHelper(points, start, best_split, left_bounds, depth + 1);
  auto right_child = BuildHelper(points, best_split, end, right_bounds, depth + 1);
  
  // Create internal node
  auto node = std::make_unique<BVHNode>(current_bounds);
  node->SetLeft(std::move(left_child));
  node->SetRight(std::move(right_child));
  
  return node;
}

void BVHTree::SortByAxis(std::vector<Point2D>& points, int start, int end, int axis) {
  if (axis == 0) {
    std::sort(points.begin() + start, points.begin() + end,
             [](const Point2D& a, const Point2D& b) { return a.x < b.x; });
  } else {
    std::sort(points.begin() + start, points.begin() + end,
             [](const Point2D& a, const Point2D& b) { return a.y < b.y; });
  }
}

double BVHTree::CalculateSAHCost(const BoundingBox& parent_bounds,
                                const BoundingBox& left_bounds,
                                const BoundingBox& right_bounds,
                                int left_count, int right_count) {
  double parent_surface_area = parent_bounds.Area();
  double left_surface_area = left_bounds.Area();
  double right_surface_area = right_bounds.Area();
  
  if (parent_surface_area == 0) {
    return std::numeric_limits<double>::max();
  }
  
  // SAH formula: cost = left_count * left_area / parent_area + right_count * right_area / parent_area
  double traversal_cost = 1.0;  // Cost of traversing internal node
  double left_cost = left_count * (left_surface_area / parent_surface_area);
  double right_cost = right_count * (right_surface_area / parent_surface_area);
  
  return traversal_cost + left_cost + right_cost;
}

}  // namespace geometry