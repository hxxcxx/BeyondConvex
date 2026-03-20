#include "bsptree.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <queue>

namespace geometry {

// ============================================================================
// BSPNode Implementation
// ============================================================================

BSPNode::BSPNode(const BoundingBox& bounds)
    : bounds_(bounds) {
}

bool BSPNode::Insert(const Point2D& point, int capacity) {
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
  
  // Insert into appropriate child based on split plane
  auto classification = split_plane_.Classify(point);
  
  if (classification == Plane::FRONT) {
    if (front_child_) {
      return front_child_->Insert(point, capacity);
    }
  } else if (classification == Plane::BACK) {
    if (back_child_) {
      return back_child_->Insert(point, capacity);
    }
  } else {
    // Coplanar points go to both children (or just front)
    if (front_child_) {
      front_child_->Insert(point, capacity);
    }
    if (back_child_) {
      back_child_->Insert(point, capacity);
    }
    return true;
  }
  
  return false;
}

bool BSPNode::Remove(const Point2D& point) {
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
  auto classification = split_plane_.Classify(point);
  
  if (classification == Plane::FRONT && front_child_) {
    return front_child_->Remove(point);
  } else if (classification == Plane::BACK && back_child_) {
    return back_child_->Remove(point);
  } else {
    // Check both children for coplanar points
    bool found = false;
    if (front_child_) {
      found = front_child_->Remove(point);
    }
    if (back_child_ && !found) {
      found = back_child_->Remove(point);
    }
    return found;
  }
}

void BSPNode::RangeQuery(const BoundingBox& range, 
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
    if (front_child_) {
      front_child_->RangeQuery(range, result);
    }
    if (back_child_) {
      back_child_->RangeQuery(range, result);
    }
  }
}

void BSPNode::RadiusQuery(const Point2D& center, double radius,
                          std::vector<Point2D>& result) const {
  // If this node doesn't intersect the circle, return
  double radius_sq = radius * radius;
  
  // Find closest point on bounds to center
  double closest_x = std::max(bounds_.min_x, std::min(center.x, bounds_.max_x));
  double closest_y = std::max(bounds_.min_y, std::min(center.y, bounds_.max_y));
  
  double dx = center.x - closest_x;
  double dy = center.y - closest_y;
  double dist_sq = dx * dx + dy * dy;
  
  if (dist_sq > radius_sq) {
    return;
  }
  
  // Check points in this node
  for (const auto& pt : points_) {
    if (center.DistanceSquaredTo(pt) <= radius_sq) {
      result.push_back(pt);
    }
  }
  
  // Recursively search children
  if (!IsLeaf()) {
    if (front_child_) {
      front_child_->RadiusQuery(center, radius, result);
    }
    if (back_child_) {
      back_child_->RadiusQuery(center, radius, result);
    }
  }
}

bool BSPNode::NearestNeighbor(const Point2D& query, Point2D& nearest,
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
  
  // Determine which side to search first
  auto classification = split_plane_.Classify(query);
  
  BSPNode* first = (classification == Plane::BACK) ? 
                   back_child_.get() : front_child_.get();
  BSPNode* second = (classification == Plane::BACK) ? 
                    back_child_.get() : front_child_.get();
  
  // Swap if classification is FRONT
  if (classification == Plane::FRONT) {
    first = front_child_.get();
    second = back_child_.get();
  }
  
  // Search first side
  if (first) {
    Point2D child_nearest;
    double child_dist = min_dist;
    if (first->NearestNeighbor(query, child_nearest, child_dist)) {
      if (child_dist < min_dist) {
        min_dist = child_dist;
        nearest = child_nearest;
        found = true;
      }
    }
  }
  
  // Check if we need to search second side
  double dist_to_plane = std::abs(split_plane_.SignedDistance(query));
  if (second && dist_to_plane * dist_to_plane < min_dist) {
    Point2D child_nearest;
    double child_dist = min_dist;
    if (second->NearestNeighbor(query, child_nearest, child_dist)) {
      if (child_dist < min_dist) {
        min_dist = child_dist;
        nearest = child_nearest;
        found = true;
      }
    }
  }
  
  return found;
}

void BSPNode::KNearestNeighbors(const Point2D& query, int k,
                                std::vector<Point2D>& result,
                                std::vector<double>& distances) const {
  // Use a max-heap to maintain k nearest neighbors
  auto cmp = [](const std::pair<double, Point2D>& a, 
                const std::pair<double, Point2D>& b) {
    return a.first < b.first;  // Max-heap (smallest at bottom)
  };
  std::vector<std::pair<double, Point2D>> heap;
  
  // Helper function using lambda recursion
  std::function<void(const BSPNode*)> search = 
      [&](const BSPNode* node) {
    if (!node) return;
    
    // Check current node
    for (const auto& pt : node->points_) {
      double dist = query.DistanceSquaredTo(pt);
      if (heap.size() < static_cast<size_t>(k)) {
        heap.push_back({dist, pt});
        std::push_heap(heap.begin(), heap.end(), cmp);
      } else if (dist < heap[0].first) {
        std::pop_heap(heap.begin(), heap.end(), cmp);
        heap.pop_back();
        heap.push_back({dist, pt});
        std::push_heap(heap.begin(), heap.end(), cmp);
      }
    }
    
    // Recursively search children
    if (!node->IsLeaf()) {
      // Determine which side to search first
      auto classification = node->split_plane_.Classify(query);
      
      const BSPNode* first = (classification == Plane::BACK) ? 
                             node->back_child_.get() : node->front_child_.get();
      const BSPNode* second = (classification == Plane::BACK) ? 
                             node->front_child_.get() : node->back_child_.get();
      
      if (classification == Plane::FRONT) {
        first = node->front_child_.get();
        second = node->back_child_.get();
      }
      
      // Search first side
      search(first);
      
      // Check if we need to search second side
      double max_dist = heap.empty() ? std::numeric_limits<double>::max() : heap[0].first;
      double dist_to_plane = std::abs(node->split_plane_.SignedDistance(query));
      if (second && dist_to_plane * dist_to_plane < max_dist) {
        search(second);
      }
    }
  };
  
  search(this);
  
  // Extract results
  result.clear();
  distances.clear();
  for (const auto& item : heap) {
    result.push_back(item.second);
    distances.push_back(std::sqrt(item.first));
  }
}

bool BSPNode::Contains(const Point2D& point) const {
  // Check points in this node
  if (std::find(points_.begin(), points_.end(), point) != points_.end()) {
    return true;
  }
  
  // If leaf, point not found
  if (IsLeaf()) {
    return false;
  }
  
  // Search in appropriate child
  auto classification = split_plane_.Classify(point);
  
  if (classification == Plane::FRONT && front_child_) {
    return front_child_->Contains(point);
  } else if (classification == Plane::BACK && back_child_) {
    return back_child_->Contains(point);
  } else {
    // Check both children for coplanar points
    if (front_child_ && front_child_->Contains(point)) {
      return true;
    }
    if (back_child_ && back_child_->Contains(point)) {
      return true;
    }
    return false;
  }
}

std::vector<Point2D> BSPNode::GetAllPoints() const {
  std::vector<Point2D> result;
  result.insert(result.end(), points_.begin(), points_.end());
  
  if (!IsLeaf()) {
    if (front_child_) {
      auto front_points = front_child_->GetAllPoints();
      result.insert(result.end(), front_points.begin(), front_points.end());
    }
    if (back_child_) {
      auto back_points = back_child_->GetAllPoints();
      result.insert(result.end(), back_points.begin(), back_points.end());
    }
  }
  
  return result;
}

int BSPNode::GetDepth() const {
  return GetDepthHelper(0);
}

int BSPNode::GetNodeCount() const {
  int count = 1;
  GetNodeCountHelper(count);
  return count;
}

int BSPNode::GetPointCount() const {
  int count = static_cast<int>(points_.size());
  
  if (!IsLeaf()) {
    if (front_child_) {
      count += front_child_->GetPointCount();
    }
    if (back_child_) {
      count += back_child_->GetPointCount();
    }
  }
  
  return count;
}

void BSPNode::Clear() {
  points_.clear();
  front_child_.reset();
  back_child_.reset();
}

void BSPNode::Subdivide() {
  // Calculate optimal split plane
  split_plane_ = CalculateOptimalSplitPlane();
  
  // Split the bounding box
  auto [front_bounds, back_bounds] = SplitBounds(split_plane_);
  
  // Create child nodes
  front_child_ = std::make_unique<BSPNode>(front_bounds);
  back_child_ = std::make_unique<BSPNode>(back_bounds);
  
  // Redistribute existing points to children
  for (const auto& pt : points_) {
    auto classification = split_plane_.Classify(pt);
    
    if (classification == Plane::FRONT) {
      front_child_->points_.push_back(pt);
    } else if (classification == Plane::BACK) {
      back_child_->points_.push_back(pt);
    } else {
      // Coplanar points go to front child
      front_child_->points_.push_back(pt);
    }
  }
  points_.clear();
}

Plane BSPNode::CalculateOptimalSplitPlane() const {
  if (points_.empty()) {
    // Default: split through center with X-axis normal
    Point2D center = bounds_.Center();
    return Plane(center, Vector2D(1.0, 0.0));
  }
  
  // Calculate centroid
  double sum_x = 0, sum_y = 0;
  for (const auto& pt : points_) {
    sum_x += pt.x;
    sum_y += pt.y;
  }
  Point2D centroid(sum_x / points_.size(), sum_y / points_.size());
  
  // Calculate principal axis (using variance)
  double var_x = 0, var_y = 0;
  for (const auto& pt : points_) {
    double dx = pt.x - centroid.x;
    double dy = pt.y - centroid.y;
    var_x += dx * dx;
    var_y += dy * dy;
  }
  
  // Split along axis with larger variance
  Vector2D normal;
  if (var_x > var_y) {
    normal = Vector2D(1.0, 0.0);  // X-axis
  } else {
    normal = Vector2D(0.0, 1.0);  // Y-axis
  }
  
  return Plane(centroid, normal);
}

std::pair<BoundingBox, BoundingBox> BSPNode::SplitBounds(const Plane& plane) const {
  // For axis-aligned splits, this is straightforward
  // For arbitrary splits, we need to clip the bounding box
  
  BoundingBox front_bounds = bounds_;
  BoundingBox back_bounds = bounds_;
  
  // Check if plane is axis-aligned
  bool is_x_aligned = std::abs(plane.normal.x) > 0.9;
  bool is_y_aligned = std::abs(plane.normal.y) > 0.9;
  
  if (is_x_aligned) {
    // Vertical split
    double split_x = plane.point.x;
    front_bounds.min_x = split_x;
    back_bounds.max_x = split_x;
  } else if (is_y_aligned) {
    // Horizontal split
    double split_y = plane.point.y;
    front_bounds.min_y = split_y;
    back_bounds.max_y = split_y;
  } else {
    // Arbitrary split - use centroid for simplicity
    Point2D center = bounds_.Center();
    front_bounds.min_x = center.x;
    back_bounds.max_x = center.x;
  }
  
  return {front_bounds, back_bounds};
}

int BSPNode::GetDepthHelper(int current_depth) const {
  if (IsLeaf()) {
    return current_depth;
  }
  
  int max_depth = current_depth;
  if (front_child_) {
    int front_depth = front_child_->GetDepthHelper(current_depth + 1);
    max_depth = std::max(max_depth, front_depth);
  }
  if (back_child_) {
    int back_depth = back_child_->GetDepthHelper(current_depth + 1);
    max_depth = std::max(max_depth, back_depth);
  }
  
  return max_depth;
}

void BSPNode::GetNodeCountHelper(int& count) const {
  if (!IsLeaf()) {
    if (front_child_) {
      count++;
      front_child_->GetNodeCountHelper(count);
    }
    if (back_child_) {
      count++;
      back_child_->GetNodeCountHelper(count);
    }
  }
}

// ============================================================================
// BSPTree Implementation
// ============================================================================

BSPTree::BSPTree(const BoundingBox& bounds, int capacity)
    : bounds_(bounds), capacity_(capacity) {
  if (bounds_.Width() > 0 && bounds_.Height() > 0) {
    root_ = std::make_unique<BSPNode>(bounds_);
  }
}

bool BSPTree::Insert(const Point2D& point) {
  if (!root_) {
    return false;
  }
  
  // Expand bounds if needed (optional)
  if (!bounds_.Contains(point)) {
    return false;
  }
  
  return root_->Insert(point, capacity_);
}

void BSPTree::Insert(const std::vector<Point2D>& points) {
  for (const auto& pt : points) {
    Insert(pt);
  }
}

bool BSPTree::Remove(const Point2D& point) {
  return root_ && root_->Remove(point);
}

bool BSPTree::Contains(const Point2D& point) const {
  return root_ && root_->Contains(point);
}

std::vector<Point2D> BSPTree::RangeQuery(const BoundingBox& range) const {
  std::vector<Point2D> result;
  if (root_) {
    root_->RangeQuery(range, result);
  }
  return result;
}

std::vector<Point2D> BSPTree::RadiusQuery(const Point2D& center, 
                                           double radius) const {
  std::vector<Point2D> result;
  if (root_) {
    root_->RadiusQuery(center, radius, result);
  }
  return result;
}

bool BSPTree::NearestNeighbor(const Point2D& query, Point2D& nearest) const {
  if (!root_) {
    return false;
  }
  
  double min_dist = std::numeric_limits<double>::max();
  return root_->NearestNeighbor(query, nearest, min_dist);
}

std::vector<Point2D> BSPTree::KNearestNeighbors(const Point2D& query, 
                                                 int k) const {
  std::vector<Point2D> result;
  std::vector<double> distances;
  
  if (root_) {
    root_->KNearestNeighbors(query, k, result, distances);
  }
  
  return result;
}

std::vector<Point2D> BSPTree::GetAllPoints() const {
  return root_ ? root_->GetAllPoints() : std::vector<Point2D>();
}

void BSPTree::Clear() {
  if (root_) {
    root_->Clear();
  }
}

void BSPTree::Rebuild() {
  auto all_points = GetAllPoints();
  root_ = std::make_unique<BSPNode>(bounds_);
  Insert(all_points);
}

int BSPTree::GetDepth() const {
  return root_ ? root_->GetDepth() : 0;
}

int BSPTree::GetNodeCount() const {
  return root_ ? root_->GetNodeCount() : 0;
}

int BSPTree::GetPointCount() const {
  return root_ ? root_->GetPointCount() : 0;
}

void BSPTree::KNearestNeighborsHelper(const BSPNode* node,
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
    if (node->GetFrontChild()) {
      KNearestNeighborsHelper(node->GetFrontChild(), query, k, result, distances);
    }
    if (node->GetBackChild()) {
      KNearestNeighborsHelper(node->GetBackChild(), query, k, result, distances);
    }
  }
}

}  // namespace geometry
