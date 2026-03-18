#include "kdtree.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace geometry {

// ============================================================================
// KDTreeNode Implementation
// ============================================================================

KDTreeNode::KDTreeNode(const Point2D& point, SplitAxis axis)
    : point_(point), split_axis_(axis) {
}

bool KDTreeNode::Insert(const Point2D& point) {
  // Check if point already exists
  if (point == point_) {
    return false;
  }
  
  // Determine which subtree to insert into
  if (IsInLeftSubtree(point)) {
    if (!left_) {
      left_ = std::make_unique<KDTreeNode>(point, GetNextAxis());
      return true;
    }
    return left_->Insert(point);
  } else {
    if (!right_) {
      right_ = std::make_unique<KDTreeNode>(point, GetNextAxis());
      return true;
    }
    return right_->Insert(point);
  }
}

bool KDTreeNode::Contains(const Point2D& point) const {
  // Check current node
  if (point == point_) {
    return true;
  }
  
  // Search in appropriate subtree
  if (IsInLeftSubtree(point)) {
    return left_ && left_->Contains(point);
  } else {
    return right_ && right_->Contains(point);
  }
}

void KDTreeNode::RangeQuery(const BoundingBox& range, 
                           std::vector<Point2D>& result) const {
  // Check if current point is in range
  if (range.Contains(point_)) {
    result.push_back(point_);
  }
  
  // Recursively search subtrees that might contain points in range
  double split_value = GetSplitValue();
  
  // Check left subtree
  if (left_) {
    bool search_left = false;
    if (split_axis_ == X_AXIS) {
      search_left = range.min_x <= split_value;
    } else {
      search_left = range.min_y <= split_value;
    }
    
    if (search_left) {
      left_->RangeQuery(range, result);
    }
  }
  
  // Check right subtree
  if (right_) {
    bool search_right = false;
    if (split_axis_ == X_AXIS) {
      search_right = range.max_x >= split_value;
    } else {
      search_right = range.max_y >= split_value;
    }
    
    if (search_right) {
      right_->RangeQuery(range, result);
    }
  }
}

void KDTreeNode::RadiusQuery(const Point2D& center, double radius,
                            std::vector<Point2D>& result) const {
  // Check if current point is within radius
  if (center.DistanceSquaredTo(point_) <= radius * radius) {
    result.push_back(point_);
  }
  
  // Recursively search subtrees that might contain points within radius
  double split_value = GetSplitValue();
  double query_value = (split_axis_ == X_AXIS) ? center.x : center.y;
  
  // Check left subtree
  if (left_ && query_value - radius <= split_value) {
    left_->RadiusQuery(center, radius, result);
  }
  
  // Check right subtree
  if (right_ && query_value + radius >= split_value) {
    right_->RadiusQuery(center, radius, result);
  }
}

bool KDTreeNode::NearestNeighbor(const Point2D& query, Point2D& nearest,
                                 double& min_dist) const {
  // Check current node
  double dist = query.DistanceSquaredTo(point_);
  if (dist < min_dist) {
    min_dist = dist;
    nearest = point_;
  }
  
  // Determine which subtree to search first
  double query_value = (split_axis_ == X_AXIS) ? query.x : query.y;
  double split_value = GetSplitValue();
  
  KDTreeNode* first = (query_value < split_value) ? left_.get() : right_.get();
  KDTreeNode* second = (query_value < split_value) ? right_.get() : left_.get();
  
  // Search first subtree
  if (first) {
    first->NearestNeighbor(query, nearest, min_dist);
  }
  
  // Check if we need to search second subtree
  double dist_to_split = query_value - split_value;
  if (second && dist_to_split * dist_to_split < min_dist) {
    second->NearestNeighbor(query, nearest, min_dist);
  }
  
  return min_dist < std::numeric_limits<double>::max();
}

void KDTreeNode::KNearestNeighbors(const Point2D& query, int k,
                                   std::vector<Point2D>& result,
                                   std::vector<double>& distances) const {
  // Use a max-heap to maintain k nearest neighbors
  auto cmp = [](const std::pair<double, Point2D>& a, 
                const std::pair<double, Point2D>& b) {
    return a.first < b.first;  // Max-heap (smallest at bottom)
  };
  std::vector<std::pair<double, Point2D>> heap;
  
  // Helper function using lambda recursion
  std::function<void(const KDTreeNode*)> search = 
      [&](const KDTreeNode* node) {
    if (!node) return;
    
    // Check current node
    double dist = query.DistanceSquaredTo(node->point_);
    if (heap.size() < static_cast<size_t>(k)) {
      heap.push_back({dist, node->point_});
      std::push_heap(heap.begin(), heap.end(), cmp);
    } else if (dist < heap[0].first) {
      std::pop_heap(heap.begin(), heap.end(), cmp);
      heap.pop_back();
      heap.push_back({dist, node->point_});
      std::push_heap(heap.begin(), heap.end(), cmp);
    }
    
    // Determine which subtree to search first
    double query_value = (node->split_axis_ == X_AXIS) ? query.x : query.y;
    double split_value = node->GetSplitValue();
    
    const KDTreeNode* first = (query_value < split_value) ? 
                              node->left_.get() : node->right_.get();
    const KDTreeNode* second = (query_value < split_value) ? 
                               node->right_.get() : node->left_.get();
    
    // Search first subtree
    search(first);
    
    // Check if we need to search second subtree
    double max_dist = heap.empty() ? std::numeric_limits<double>::max() : heap[0].first;
    double dist_to_split = query_value - split_value;
    if (second && dist_to_split * dist_to_split < max_dist) {
      search(second);
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

std::vector<Point2D> KDTreeNode::GetAllPoints() const {
  std::vector<Point2D> result;
  result.push_back(point_);
  
  if (left_) {
    auto left_points = left_->GetAllPoints();
    result.insert(result.end(), left_points.begin(), left_points.end());
  }
  
  if (right_) {
    auto right_points = right_->GetAllPoints();
    result.insert(result.end(), right_points.begin(), right_points.end());
  }
  
  return result;
}

int KDTreeNode::GetDepth() const {
  return GetDepthHelper(0);
}

int KDTreeNode::GetNodeCount() const {
  int count = 1;
  if (left_) count += left_->GetNodeCount();
  if (right_) count += right_->GetNodeCount();
  return count;
}

double KDTreeNode::GetSplitValue() const {
  return (split_axis_ == X_AXIS) ? point_.x : point_.y;
}

bool KDTreeNode::IsInLeftSubtree(const Point2D& point) const {
  double point_value = (split_axis_ == X_AXIS) ? point.x : point.y;
  double split_value = GetSplitValue();
  return point_value < split_value;
}

int KDTreeNode::GetDepthHelper(int current_depth) const {
  int left_depth = left_ ? left_->GetDepthHelper(current_depth + 1) : current_depth;
  int right_depth = right_ ? right_->GetDepthHelper(current_depth + 1) : current_depth;
  return std::max(left_depth, right_depth);
}

// ============================================================================
// KDTree Implementation
// ============================================================================

KDTree::KDTree() : size_(0) {
}

bool KDTree::Insert(const Point2D& point) {
  if (!root_) {
    root_ = std::make_unique<KDTreeNode>(point, KDTreeNode::X_AXIS);
    size_++;
    return true;
  }
  
  if (root_->Contains(point)) {
    return false;
  }
  
  root_->Insert(point);
  size_++;
  return true;
}

void KDTree::Insert(const std::vector<Point2D>& points) {
  for (const auto& pt : points) {
    Insert(pt);
  }
}

void KDTree::Build(const std::vector<Point2D>& points) {
  if (points.empty()) {
    Clear();
    return;
  }
  
  // Remove duplicates
  std::vector<Point2D> unique_points = points;
  std::sort(unique_points.begin(), unique_points.end());
  unique_points.erase(
      std::unique(unique_points.begin(), unique_points.end()),
      unique_points.end());
  
  size_ = static_cast<int>(unique_points.size());
  root_ = BuildHelper(unique_points, 0, static_cast<int>(unique_points.size()), 
                      KDTreeNode::X_AXIS);
}

bool KDTree::Contains(const Point2D& point) const {
  return root_ && root_->Contains(point);
}

std::vector<Point2D> KDTree::RangeQuery(const BoundingBox& range) const {
  std::vector<Point2D> result;
  if (root_) {
    root_->RangeQuery(range, result);
  }
  return result;
}

std::vector<Point2D> KDTree::RadiusQuery(const Point2D& center, 
                                         double radius) const {
  std::vector<Point2D> result;
  if (root_) {
    root_->RadiusQuery(center, radius, result);
  }
  return result;
}

bool KDTree::NearestNeighbor(const Point2D& query, Point2D& nearest) const {
  if (!root_) {
    return false;
  }
  
  double min_dist = std::numeric_limits<double>::max();
  return root_->NearestNeighbor(query, nearest, min_dist);
}

std::vector<Point2D> KDTree::KNearestNeighbors(const Point2D& query, int k) const {
  std::vector<Point2D> result;
  std::vector<double> distances;
  
  if (root_) {
    root_->KNearestNeighbors(query, k, result, distances);
  }
  
  return result;
}

std::vector<Point2D> KDTree::GetAllPoints() const {
  return root_ ? root_->GetAllPoints() : std::vector<Point2D>();
}

void KDTree::Clear() {
  root_.reset();
  size_ = 0;
}

int KDTree::GetDepth() const {
  return root_ ? root_->GetDepth() : 0;
}

int KDTree::GetNodeCount() const {
  return root_ ? root_->GetNodeCount() : 0;
}

std::unique_ptr<KDTreeNode> KDTree::BuildHelper(
    std::vector<Point2D>& points, int start, int end,
    KDTreeNode::SplitAxis axis) {
  if (start >= end) {
    return nullptr;
  }
  
  // Sort by current axis
  SortByAxis(points, start, end, axis);
  
  // Find median
  int mid = start + (end - start) / 2;
  
  // Create node with median point
  auto node = std::make_unique<KDTreeNode>(points[mid], axis);
  
  // Recursively build subtrees
  node->SetLeft(BuildHelper(points, start, mid, node->GetNextAxis()));
  node->SetRight(BuildHelper(points, mid + 1, end, node->GetNextAxis()));
  
  return node;
}

void KDTree::SortByAxis(std::vector<Point2D>& points, int start, int end,
                       KDTreeNode::SplitAxis axis) {
  auto cmp = [axis](const Point2D& a, const Point2D& b) {
    return (axis == KDTreeNode::X_AXIS) ? (a.x < b.x) : (a.y < b.y);
  };
  
  std::sort(points.begin() + start, points.begin() + end, cmp);
}

}  // namespace geometry
