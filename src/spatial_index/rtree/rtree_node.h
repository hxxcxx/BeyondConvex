#pragma once

#include "../spatial_index_common.h"
#include <vector>
#include <memory>
#include <algorithm>

namespace geometry {

// Forward declaration
class RTreeNode;

// Entry in R-tree node (either child node or data rectangle)
struct RTreeEntry {
  BoundingBox mbr;  // Minimum bounding rectangle
  std::unique_ptr<RTreeNode> child;  // Child node (null for leaf entries)
  int data_id;  // Data ID (only used in leaf entries)
  
  RTreeEntry() : data_id(-1) {}
  RTreeEntry(const BoundingBox& bounds) : mbr(bounds), data_id(-1) {}
  
  bool IsLeaf() const { return child == nullptr; }
};

// R-tree node
class RTreeNode {
 public:
  // Constructor
  explicit RTreeNode(bool is_leaf = false);
  
  // Getters
  bool IsLeaf() const { return is_leaf_; }
  int GetEntryCount() const { return entries_.size(); }
  const std::vector<RTreeEntry>& GetEntries() const { return entries_; }
  const BoundingBox& GetMBR() const { return mbr_; }
  
  // Entry management
  void AddEntry(RTreeEntry entry);
  void RemoveEntry(int index);
  const RTreeEntry& GetEntry(int index) const { return entries_[index]; }
  RTreeEntry& GetEntry(int index) { return entries_[index]; }
  
  // Update MBR from all entries
  void UpdateMBR();
  
  // Split node (for R-tree insertion)
  std::unique_ptr<RTreeNode> Split();
  
  // Check if node is full
  bool IsFull(int max_entries) const { return entries_.size() >= max_entries; }
  
  // Find best entry to insert a new rectangle (using area enlargement)
  int FindBestEntry(const BoundingBox& bounds) const;
  
  // Clear all entries
  void Clear() { 
    entries_.clear(); 
    mbr_ = BoundingBox();
  }
  
 private:
  bool is_leaf_;
  std::vector<RTreeEntry> entries_;
  BoundingBox mbr_;
  
  // Helper for quadratic split algorithm
  void PickSeeds(int& seed1, int& seed2) const;
  void PickNext(std::vector<RTreeEntry>& entries, 
               const BoundingBox& group1_mbr, 
               const BoundingBox& group2_mbr,
               int& next_index, bool& pick_group1) const;
};

}  // namespace geometry
