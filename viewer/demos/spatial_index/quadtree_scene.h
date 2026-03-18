#ifndef VIEWER_DEMOS_SPATIAL_INDEX_QUADTREE_SCENE_H_
#define VIEWER_DEMOS_SPATIAL_INDEX_QUADTREE_SCENE_H_

#include "../scene_base.h"
#include "../../../src/spatial_index/quadtree/quadtree.h"
#include <vector>
#include <memory>
#include <imgui.h>

namespace geometry {

enum class QuadtreeDemoMode {
  InsertPoints,      // Click to insert points
  RangeQuery,        // Drag to select range
  NearestNeighbor,   // Click to find nearest neighbor
  RadiusQuery        // Click to find points within radius
};

class QuadtreeScene : public GeometryScene {
 public:
  QuadtreeScene();
  
  std::string Name() const override {
    return "Quadtree Demo";
  }
  
  std::string Description() const override {
    return "Interactive Quadtree visualization. Click to add points, right-click to query.";
  }
  
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  
  // Handle mouse move for drag visualization
  void UpdateMousePosition(double x, double y);
  
 private:
  std::unique_ptr<Quadtree> quadtree_;
  std::vector<Point2D> points_;
  
  // Demo mode
  QuadtreeDemoMode mode_;
  
  // Query visualization
  Point2D query_point_;
  BoundingBox query_range_;
  double query_radius_;  // Store the actual query radius
  std::vector<Point2D> query_results_;
  bool show_query_;
  
  // Settings
  int capacity_;
  bool auto_bounds_;
  bool show_points_;
  bool show_bounds_;
  bool show_query_results_;
  
  // Mouse state for range/radius query
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
  
  void InitializeQuadtree();
  void InitializeBounds(float canvas_width, float canvas_height);
  void RebuildQuadtree();
  
  // Query operations
  void PerformRangeQuery(const BoundingBox& range);
  void PerformNearestNeighbor(const Point2D& query);
  void PerformRadiusQuery(const Point2D& center, double radius);
  
  // Test helpers
  void GenerateRandomPoints(int count);
  
  // Rendering helpers
  void RenderQuadtree(const QuadTreeNode* node, 
                     float canvas_x, float canvas_y,
                     float canvas_height, int depth);
  void RenderPoints(float canvas_x, float canvas_y, float canvas_height);
  void RenderQuery(float canvas_x, float canvas_y, float canvas_height);
  
  // Color helpers
  ImU32 GetDepthColor(int depth, int max_depth);
};

}  // namespace geometry

#endif  // VIEWER_DEMOS_SPATIAL_INDEX_QUADTREE_SCENE_H_
