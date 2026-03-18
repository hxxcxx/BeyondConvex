
#pragma once
#include "../scene_base.h"
#include "../../../src/spatial_index/kdtree/kdtree.h"
#include <vector>
#include <memory>
#include <imgui.h>

namespace geometry {

enum class KDTreeDemoMode {
  InsertPoints,      // Click to insert points
  NearestNeighbor,   // Click to find nearest neighbor
  KNearestNeighbors, // Click to find k nearest neighbors
  RangeQuery,        // Drag to select range
  RadiusQuery        // Drag to define radius
};

class KDTreeScene : public GeometryScene {
 public:
  KDTreeScene();
  
  std::string Name() const override {
    return "KD-Tree Demo";
  }
  
  std::string Description() const override {
    return "KD-Tree visualization - optimized for nearest neighbor search. Click to add points and test queries.";
  }
  
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  
 private:
  std::unique_ptr<KDTree> kdtree_;
  std::vector<Point2D> points_;
  
  // Demo mode
  KDTreeDemoMode mode_;
  
  // Query visualization
  Point2D query_point_;
  BoundingBox query_range_;
  double query_radius_;
  int k_neighbors_;
  std::vector<Point2D> query_results_;
  bool show_query_;
  
  // Settings
  bool show_points_;
  bool show_tree_;
  bool show_query_results_;
  
  // Mouse state for drag operations
  bool is_dragging_;
  Point2D drag_start_;
  Point2D drag_end_;
  Point2D current_mouse_pos_;
  
  // Statistics
  int last_query_count_;
  double last_query_time_ms_;
  
  // Canvas size tracking
  float canvas_width_;
  float canvas_height_;
  bool bounds_initialized_;
  
  void InitializeKDTree();
  void InitializeBounds(float canvas_width, float canvas_height);
  void RebuildKDTree();
  
  // Query operations
  void PerformNearestNeighbor(const Point2D& query);
  void PerformKNearestNeighbors(const Point2D& query, int k);
  void PerformRangeQuery(const BoundingBox& range);
  void PerformRadiusQuery(const Point2D& center, double radius);
  
  // Test helpers
  void GenerateRandomPoints(int count);
  
  // Rendering helpers
  void RenderKDTree(const KDTreeNode* node, 
                   float canvas_x, float canvas_y,
                   float canvas_height, int depth,
                   const BoundingBox& bounds);
  void RenderPoints(float canvas_x, float canvas_y, float canvas_height);
  void RenderQuery(float canvas_x, float canvas_y, float canvas_height);
  
  // Color helpers
  ImU32 GetDepthColor(int depth, int max_depth);
  
  // Mouse update
  void UpdateMousePosition(double x, double y);
};

}  // namespace geometry

