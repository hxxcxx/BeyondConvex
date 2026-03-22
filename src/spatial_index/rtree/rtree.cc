#include "rtree.h"
#include <algorithm>
#include <limits>
#include <cmath>

namespace geometry {

// ============================================================================
// RTreeNode Implementation
// ============================================================================

RTreeNode::RTreeNode(bool is_leaf) : is_leaf_(is_leaf) {
}

void RTreeNode::AddEntry(RTreeEntry entry) {
  entries_.push_back(std::move(entry));
  UpdateMBR();
}

void RTreeNode::RemoveEntry(int index) {
  if (index >= 0 && index < static_cast<int>(entries_.size())) {
    entries_.erase(entries_.begin() + index);
    UpdateMBR();
  }
}

void RTreeNode::UpdateMBR() {
  if (entries_.empty()) {
    mbr_ = BoundingBox();
    return;
  }
  
  mbr_ = entries_[0].mbr;
  for (size_t i = 1; i < entries_.size(); ++i) {
    mbr_.min_x = std::min(mbr_.min_x, entries_[i].mbr.min_x);
    mbr_.min_y = std::min(mbr_.min_y, entries_[i].mbr.min_y);
    mbr_.max_x = std::max(mbr_.max_x, entries_[i].mbr.max_x);
    mbr_.max_y = std::max(mbr_.max_y, entries_[i].mbr.max_y);
  }
}

int RTreeNode::FindBestEntry(const BoundingBox& bounds) const {
  if (entries_.empty()) return -1;
  
  int best_index = 0;
  double min_enlargement = std::numeric_limits<double>::max();
  double min_area = std::numeric_limits<double>::max();
  
  for (size_t i = 0; i < entries_.size(); ++i) {
    const BoundingBox& entry_mbr = entries_[i].mbr;
    
    // Calculate area enlargement needed
    double original_area = entry_mbr.Area();
    
    double new_min_x = std::min(entry_mbr.min_x, bounds.min_x);
    double new_min_y = std::min(entry_mbr.min_y, bounds.min_y);
    double new_max_x = std::max(entry_mbr.max_x, bounds.max_x);
    double new_max_y = std::max(entry_mbr.max_y, bounds.max_y);
    
    double new_area = (new_max_x - new_min_x) * (new_max_y - new_min_y);
    double enlargement = new_area - original_area;
    
    // Choose entry with minimum enlargement, then minimum area
    if (enlargement < min_enlargement || 
        (enlargement == min_enlargement && original_area < min_area)) {
      min_enlargement = enlargement;
      min_area = original_area;
      best_index = static_cast<int>(i);
    }
  }
  
  return best_index;
}

std::unique_ptr<RTreeNode> RTreeNode::Split() {
  // Use quadratic split algorithm (R*-tree variant)
  int seed1, seed2;
  PickSeeds(seed1, seed2);
  
  // Create new node
  auto new_node = std::make_unique<RTreeNode>(is_leaf_);
  
  // Use indices to track which entries go to which group
  std::vector<int> group1_indices, group2_indices;
  group1_indices.push_back(seed1);
  group2_indices.push_back(seed2);
  
  // Calculate MBRs for both groups
  BoundingBox mbr1 = entries_[seed1].mbr;
  BoundingBox mbr2 = entries_[seed2].mbr;
  
  // Mark seeds as used
  std::vector<bool> used(entries_.size(), false);
  used[seed1] = true;
  used[seed2] = true;
  
  // Distribute remaining entries
  while (group1_indices.size() + group2_indices.size() < entries_.size()) {
    int next_index = -1;
    bool pick_group1 = true;
    double max_diff = -1;
    
    // Find next entry to assign
    for (size_t i = 0; i < entries_.size(); ++i) {
      if (used[i]) continue;
      
      const BoundingBox& mbr = entries_[i].mbr;
      
      // Calculate area enlargement for both groups
      double area1 = mbr1.Area();
      double new_min_x1 = std::min(mbr1.min_x, mbr.min_x);
      double new_min_y1 = std::min(mbr1.min_y, mbr.min_y);
      double new_max_x1 = std::max(mbr1.max_x, mbr.max_x);
      double new_max_y1 = std::max(mbr1.max_y, mbr.max_y);
      double enlargement1 = (new_max_x1 - new_min_x1) * (new_max_y1 - new_min_y1) - area1;
      
      double area2 = mbr2.Area();
      double new_min_x2 = std::min(mbr2.min_x, mbr.min_x);
      double new_min_y2 = std::min(mbr2.min_y, mbr.min_y);
      double new_max_x2 = std::max(mbr2.max_x, mbr.max_x);
      double new_max_y2 = std::max(mbr2.max_y, mbr.max_y);
      double enlargement2 = (new_max_x2 - new_min_x2) * (new_max_y2 - new_min_y2) - area2;
      
      double diff = std::abs(enlargement1 - enlargement2);
      
      if (diff > max_diff) {
        max_diff = diff;
        next_index = static_cast<int>(i);
        pick_group1 = (enlargement1 < enlargement2);
      }
    }
    
    if (next_index >= 0) {
      if (pick_group1) {
        group1_indices.push_back(next_index);
        mbr1.min_x = std::min(mbr1.min_x, entries_[next_index].mbr.min_x);
        mbr1.min_y = std::min(mbr1.min_y, entries_[next_index].mbr.min_y);
        mbr1.max_x = std::max(mbr1.max_x, entries_[next_index].mbr.max_x);
        mbr1.max_y = std::max(mbr1.max_y, entries_[next_index].mbr.max_y);
      } else {
        group2_indices.push_back(next_index);
        mbr2.min_x = std::min(mbr2.min_x, entries_[next_index].mbr.min_x);
        mbr2.min_y = std::min(mbr2.min_y, entries_[next_index].mbr.min_y);
        mbr2.max_x = std::max(mbr2.max_x, entries_[next_index].mbr.max_x);
        mbr2.max_y = std::max(mbr2.max_y, entries_[next_index].mbr.max_y);
      }
      used[next_index] = true;
    }
  }
  
  // Create new entries vectors using move semantics
  std::vector<RTreeEntry> new_entries1;
  std::vector<RTreeEntry> new_entries2;
  new_entries1.reserve(group1_indices.size());
  new_entries2.reserve(group2_indices.size());
  
  for (int idx : group1_indices) {
    new_entries1.push_back(std::move(entries_[idx]));
  }
  for (int idx : group2_indices) {
    new_entries2.push_back(std::move(entries_[idx]));
  }
  
  // Update current node
  entries_ = std::move(new_entries1);
  UpdateMBR();
  
  // Set new node
  new_node->entries_ = std::move(new_entries2);
  new_node->UpdateMBR();
  
  return new_node;
}

void RTreeNode::PickSeeds(int& seed1, int& seed2) const {
  if (entries_.size() < 2) {
    seed1 = 0;
    seed2 = 0;
    return;
  }
  
  double max_waste = -1;
  
  // Quadratic algorithm: find pair that maximizes wasted space
  for (size_t i = 0; i < entries_.size(); ++i) {
    for (size_t j = i + 1; j < entries_.size(); ++j) {
      const BoundingBox& mbr1 = entries_[i].mbr;
      const BoundingBox& mbr2 = entries_[j].mbr;
      
      // Calculate combined MBR
      double combined_min_x = std::min(mbr1.min_x, mbr2.min_x);
      double combined_min_y = std::min(mbr1.min_y, mbr2.min_y);
      double combined_max_x = std::max(mbr1.max_x, mbr2.max_x);
      double combined_max_y = std::max(mbr1.max_y, mbr2.max_y);
      
      double combined_area = (combined_max_x - combined_min_x) * 
                            (combined_max_y - combined_min_y);
      double waste = combined_area - mbr1.Area() - mbr2.Area();
      
      if (waste > max_waste) {
        max_waste = waste;
        seed1 = static_cast<int>(i);
        seed2 = static_cast<int>(j);
      }
    }
  }
}

void RTreeNode::PickNext(std::vector<RTreeEntry>& entries,
                        const BoundingBox& group1_mbr,
                        const BoundingBox& group2_mbr,
                        int& next_index, bool& pick_group1) const {
  double max_diff = -1;
  next_index = -1;
  pick_group1 = true;
  
  for (size_t i = 0; i < entries.size(); ++i) {
    // Skip if already used (we need to track this differently)
    bool used = false;
    for (size_t j = 0; j < entries_.size(); ++j) {
      if (&entries[i] == &entries_[j]) {
        // This is a simplified check - in practice we'd use a separate used array
        break;
      }
    }
    
    const BoundingBox& mbr = entries[i].mbr;
    
    // Calculate area enlargement for both groups
    double area1 = group1_mbr.Area();
    double new_min_x1 = std::min(group1_mbr.min_x, mbr.min_x);
    double new_min_y1 = std::min(group1_mbr.min_y, mbr.min_y);
    double new_max_x1 = std::max(group1_mbr.max_x, mbr.max_x);
    double new_max_y1 = std::max(group1_mbr.max_y, mbr.max_y);
    double enlargement1 = (new_max_x1 - new_min_x1) * (new_max_y1 - new_min_y1) - area1;
    
    double area2 = group2_mbr.Area();
    double new_min_x2 = std::min(group2_mbr.min_x, mbr.min_x);
    double new_min_y2 = std::min(group2_mbr.min_y, mbr.min_y);
    double new_max_x2 = std::max(group2_mbr.max_x, mbr.max_x);
    double new_max_y2 = std::max(group2_mbr.max_y, mbr.max_y);
    double enlargement2 = (new_max_x2 - new_min_x2) * (new_max_y2 - new_min_y2) - area2;
    
    double diff = std::abs(enlargement1 - enlargement2);
    
    if (diff > max_diff) {
      max_diff = diff;
      next_index = static_cast<int>(i);
      pick_group1 = (enlargement1 < enlargement2);
    }
  }
}

// ============================================================================
// RTree Implementation
// ============================================================================

RTree::RTree(int max_entries, int min_entries)
    : size_(0),
      max_entries_(max_entries),
      min_entries_(min_entries),
      next_data_id_(0) {
  if (min_entries_ > max_entries_ / 2) {
    min_entries_ = max_entries_ / 2;
  }
}

bool RTree::Insert(const BoundingBox& bounds, int data_id) {
  if (data_id < 0) {
    data_id = next_data_id_++;
  }
  
  if (!root_) {
    root_ = std::make_unique<RTreeNode>(true);
  }
  
  bool split = false;
  bool success = InsertHelper(root_.get(), bounds, data_id, 0, split);
  
  if (success) {
    size_++;
  }
  
  return success;
}

void RTree::Insert(const std::vector<std::pair<BoundingBox, int>>& rectangles) {
  for (const auto& rect : rectangles) {
    Insert(rect.first, rect.second);
  }
}

bool RTree::InsertHelper(RTreeNode* node, const BoundingBox& bounds, 
                        int data_id, int depth, bool& split) {
  split = false;
  
  if (node->IsLeaf()) {
    // Add entry to leaf
    RTreeEntry entry;
    entry.mbr = bounds;
    entry.data_id = data_id;
    node->AddEntry(std::move(entry));
    
    // Check if node needs to split
    if (node->IsFull(max_entries_)) {
      split = true;
      return true;
    }
    return true;
  } else {
    // Find best child to insert into
    int best_index = node->FindBestEntry(bounds);
    if (best_index < 0) return false;
    
    RTreeEntry& entry = node->GetEntry(best_index);
    bool child_split = false;
    bool success = InsertHelper(entry.child.get(), bounds, data_id, 
                               depth + 1, child_split);
    
    if (!success) return false;
    
    // Update MBR
    entry.mbr.min_x = std::min(entry.mbr.min_x, bounds.min_x);
    entry.mbr.min_y = std::min(entry.mbr.min_y, bounds.min_y);
    entry.mbr.max_x = std::max(entry.mbr.max_x, bounds.max_x);
    entry.mbr.max_y = std::max(entry.mbr.max_y, bounds.max_y);
    
    if (child_split) {
      // Child split, need to add new entry
      auto new_child = SplitNode(entry.child.get());
      
      RTreeEntry new_entry;
      new_entry.mbr = new_child->GetMBR();
      new_entry.child = std::move(new_child);
      
      node->AddEntry(std::move(new_entry));
      
      // Remove old child entry and add both children
      // This is simplified - in practice we'd handle this better
      
      if (node->IsFull(max_entries_)) {
        split = true;
      }
    }
    
    return true;
  }
}

std::unique_ptr<RTreeNode> RTree::SplitNode(RTreeNode* node) {
  return node->Split();
}

std::vector<int> RTree::RangeQuery(const BoundingBox& range) const {
  std::vector<int> result;
  if (root_) {
    RangeQueryHelper(root_.get(), range, result);
  }
  return result;
}

void RTree::RangeQueryHelper(const RTreeNode* node, const BoundingBox& range,
                            std::vector<int>& result) const {
  if (!node) return;
  
  for (const auto& entry : node->GetEntries()) {
    if (!entry.mbr.Intersects(range)) {
      continue;
    }
    
    if (entry.IsLeaf()) {
      result.push_back(entry.data_id);
    } else {
      RangeQueryHelper(entry.child.get(), range, result);
    }
  }
}

std::vector<int> RTree::PointQuery(const Point2D& point) const {
  std::vector<int> result;
  if (root_) {
    PointQueryHelper(root_.get(), point, result);
  }
  return result;
}

void RTree::PointQueryHelper(const RTreeNode* node, const Point2D& point,
                            std::vector<int>& result) const {
  if (!node) return;
  
  for (const auto& entry : node->GetEntries()) {
    if (!entry.mbr.Contains(point)) {
      continue;
    }
    
    if (entry.IsLeaf()) {
      result.push_back(entry.data_id);
    } else {
      PointQueryHelper(entry.child.get(), point, result);
    }
  }
}

std::vector<int> RTree::IntersectionQuery(const BoundingBox& bounds) const {
  return RangeQuery(bounds);
}

bool RTree::Contains(int data_id) {
  std::vector<RTreeNode*> path;
  return FindLeaf(root_.get(), data_id, path) != nullptr;
}

std::vector<int> RTree::GetAllData() const {
  std::vector<int> result;
  auto rectangles = GetAllRectangles();
  for (const auto& rect : rectangles) {
    result.push_back(rect.second);
  }
  return result;
}

std::vector<std::pair<BoundingBox, int>> RTree::GetAllRectangles() const {
  std::vector<std::pair<BoundingBox, int>> result;
  if (root_) {
    CollectRectangles(root_.get(), result);
  }
  return result;
}

void RTree::CollectRectangles(const RTreeNode* node,
                             std::vector<std::pair<BoundingBox, int>>& result) const {
  if (!node) return;
  
  if (node->IsLeaf()) {
    for (const auto& entry : node->GetEntries()) {
      result.push_back({entry.mbr, entry.data_id});
    }
  } else {
    for (const auto& entry : node->GetEntries()) {
      CollectRectangles(entry.child.get(), result);
    }
  }
}

void RTree::Clear() {
  root_.reset();
  size_ = 0;
  next_data_id_ = 0;
}

void RTree::Rebuild() {
  auto rectangles = GetAllRectangles();
  Clear();
  Insert(rectangles);
}

int RTree::GetDepth() const {
  return GetDepthHelper(root_.get());
}

int RTree::GetDepthHelper(const RTreeNode* node) const {
  if (!node || node->IsLeaf()) return 0;
  
  int max_depth = 0;
  for (const auto& entry : node->GetEntries()) {
    if (entry.child) {
      int depth = GetDepthHelper(entry.child.get());
      max_depth = std::max(max_depth, depth);
    }
  }
  return max_depth + 1;
}

int RTree::GetNodeCount() const {
  return GetNodeCountHelper(root_.get());
}

int RTree::GetNodeCountHelper(const RTreeNode* node) const {
  if (!node) return 0;
  
  int count = 1;
  if (!node->IsLeaf()) {
    for (const auto& entry : node->GetEntries()) {
      if (entry.child) {
        count += GetNodeCountHelper(entry.child.get());
      }
    }
  }
  return count;
}

int RTree::GetHeight() const {
  return GetDepth() + 1;
}

BoundingBox RTree::CalculateBounds(
    const std::vector<std::pair<BoundingBox, int>>& rectangles) {
  if (rectangles.empty()) {
    return BoundingBox();
  }
  
  BoundingBox bounds = rectangles[0].first;
  for (size_t i = 1; i < rectangles.size(); ++i) {
    bounds.min_x = std::min(bounds.min_x, rectangles[i].first.min_x);
    bounds.min_y = std::min(bounds.min_y, rectangles[i].first.min_y);
    bounds.max_x = std::max(bounds.max_x, rectangles[i].first.max_x);
    bounds.max_y = std::max(bounds.max_y, rectangles[i].first.max_y);
  }
  return bounds;
}

RTreeNode* RTree::FindLeaf(RTreeNode* node, int data_id,
                          std::vector<RTreeNode*>& path) {
  if (!node) return nullptr;
  
  path.push_back(node);
  
  if (node->IsLeaf()) {
    for (size_t i = 0; i < node->GetEntries().size(); ++i) {
      if (node->GetEntry(i).data_id == data_id) {
        return node;
      }
    }
    path.pop_back();
    return nullptr;
  }
  
  for (size_t i = 0; i < node->GetEntries().size(); ++i) {
    const auto& entry = node->GetEntry(i);
    if (entry.child) {
      auto* leaf = FindLeaf(entry.child.get(), data_id, path);
      if (leaf) return leaf;
    }
  }
  
  path.pop_back();
  return nullptr;
}

bool RTree::Remove(int data_id) {
  std::vector<RTreeNode*> path;
  auto* leaf = FindLeaf(root_.get(), data_id, path);
  
  if (!leaf) return false;
  
  // Remove entry from leaf
  for (size_t i = 0; i < leaf->GetEntries().size(); ++i) {
    if (leaf->GetEntry(i).data_id == data_id) {
      leaf->RemoveEntry(static_cast<int>(i));
      break;
    }
  }
  
  CondenseTree(path);
  size_--;
  return true;
}

void RTree::CondenseTree(std::vector<RTreeNode*>& path) {
  // Simplified condense - just update MBRs
  for (auto it = path.rbegin(); it != path.rend(); ++it) {
    (*it)->UpdateMBR();
  }
}

}  // namespace geometry
