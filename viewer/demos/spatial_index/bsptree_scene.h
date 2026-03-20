#pragma once
#include "../scene_base.h"
#include "../../../src/spatial_index/bsptree/bsptree.h"
#include <vector>
#include <memory>

struct ImDrawList;

namespace geometry {

class BSPNode;

class BSPTreeScene : public GeometryScene {
 public:
  BSPTreeScene();
  
  std::string Name() const override {
    return "BSP Tree Demo";
  }
  
  std::string Description() const override {
    return "BSP Tree visualization - Binary Space Partitioning for efficient spatial queries. Move mouse to test queries.";
  }
  
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  void Update(float delta_time) override;
  
 private:
  void DrawBSPTree(struct ImDrawList* draw_list, const class BSPNode* node, int depth);
  void GenerateRandomPoints(int count);
  void PerformQueries();
  void Render();
  void InitializeBounds(float canvas_width, float canvas_height);
  
  // BSP tree
  std::unique_ptr<BSPTree> bsp_tree_;
  BoundingBox bounds_;
  
  // Canvas size
  float canvas_width_;
  float canvas_height_;
  bool bounds_initialized_;
  
  // Points
  std::vector<Point2D> points_;
  
  // Query
  Point2D query_point_;
  float query_radius_;
  BoundingBox query_range_;
  int k_value_;
  
  // Results
  bool nearest_found_;
  Point2D nearest_point_;
  double nearest_distance_;
  std::vector<Point2D> knn_results_;
  std::vector<Point2D> radius_results_;
  std::vector<Point2D> range_results_;
  
  // Display options
  bool show_nearest_;
  bool show_knn_;
  bool show_radius_query_;
  bool show_range_query_;
  bool show_tree_structure_;
};

}  // namespace geometry
