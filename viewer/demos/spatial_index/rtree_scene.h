#pragma once

#include "../scene_base.h"
#include "../../../src/spatial_index/rtree/rtree.h"
#include <vector>
#include <memory>
#include <imgui.h>

namespace geometry {

enum class RTreeDemoMode {
  InsertRectangles,    // Drag to create rectangles
  RangeQuery,          // Drag to select range
  PointQuery,          // Click to query point
  IntersectionQuery    // Drag to create query rectangle
};

class RTreeScene : public GeometryScene {
 public:
  RTreeScene();
  
  std::string Name() const override {
    return "R-Tree Demo";
  }
  
  std::string Description() const override {
    return "R-Tree visualization - optimized for rectangle objects. "
           "Drag to create rectangles and test spatial queries.";
  }
  
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  
 private:
  std::unique_ptr<RTree> rtree_;
  std::vector<std::pair<BoundingBox, int>> rectangles_;
  
  // Demo mode
  RTreeDemoMode mode_;
  
  // Rectangle creation state
  bool is_creating_rect_;
  Point2D rect_start_;
  Point2D rect_end_;
  
  // Query visualization
  Point2D query_point_;
  BoundingBox query_range_;
  std::vector<int> query_results_;
  bool show_query_;
  
  // Settings
  bool show_rectangles_;
  bool show_tree_;
  bool show_query_results_;
  bool show_mbrs_;
  int max_entries_;
  int min_entries_;
  
  // Mouse state
  Point2D current_mouse_pos_;
  
  // Statistics
  int last_query_count_;
  double last_query_time_ms_;
  
  // Canvas size tracking
  float canvas_width_;
  float canvas_height_;
  
  // Test helpers
  void GenerateRandomRectangles(int count);
  void GenerateGridRectangles(int rows, int cols);
  void GenerateOverlappingRectangles(int count);
  void GenerateSpiralRectangles(int count);
  void GenerateConcentricRectangles(int rings, int per_ring);
  void GenerateCityBlocks(int rows, int cols);
  void GenerateRandomClusters(int clusters, int per_cluster);
  
  // Query operations
  void PerformRangeQuery(const BoundingBox& range);
  void PerformPointQuery(const Point2D& point);
  void PerformIntersectionQuery(const BoundingBox& bounds);
  
  // Rendering helpers
  void RenderRTree(const RTreeNode* node, 
                   float canvas_x, float canvas_y,
                   float canvas_height, int depth);
  void RenderRectangles(float canvas_x, float canvas_y, float canvas_height);
  void RenderQuery(float canvas_x, float canvas_y, float canvas_height);
  void RenderRectangle(const BoundingBox& bounds, ImU32 color, float fill_alpha,
                      float canvas_x, float canvas_y, float canvas_height);
  
  // Color helpers
  ImU32 GetDepthColor(int depth, int max_depth);
  ImU32 GetRectangleColor(int index);
  
  // Mouse update
  void UpdateMousePosition(double x, double y);
  
  // Rebuild tree
  void RebuildTree();
};

}  // namespace geometry
