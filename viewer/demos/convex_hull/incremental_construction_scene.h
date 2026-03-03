#ifndef VIEWER_DEMOS_INCREMENTAL_CONSTRUCTION_SCENE_H_
#define VIEWER_DEMOS_INCREMENTAL_CONSTRUCTION_SCENE_H_

#include "../scene_base.h"
#include "../../../src/core/point2d.h"
#include "../../../src/convex_hull/convex_hull.h"
#include <vector>

// Forward declarations
struct ImVec2;
using ImU32 = unsigned int;

namespace geometry {

// Incremental Construction visualization scene
// Shows how to incrementally build a convex hull by adding points one by one
class IncrementalConstructionScene : public GeometryScene {
 public:
  std::string Name() const override {
    return "Incremental Construction";
  }
  
  std::string Description() const override {
    return "Incrementally construct a convex hull. Start with 6 points, then add more "
           "points to see how the convex hull updates dynamically.";
  }
  
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  
 private:
  std::vector<Point2D> points_;
  ConvexHull hull_;  // Convex hull with vertices in CCW order
  bool initialized_ = false;
  bool show_hull_edges_ = true;
  bool show_all_points_ = true;
  
  // Initialize with 6 points
  void InitializePoints();
  
  // Helper to draw arrow
  void DrawArrow(float x1, float y1, float x2, float y2, 
                 float arrow_size, ImU32 color, float thickness = 2.0f);
  
  // Helper to draw point with label
  void DrawLabeledPoint(float x, float y, const char* label, ImU32 color);
};

}  // namespace geometry

#endif  // VIEWER_DEMOS_INCREMENTAL_CONSTRUCTION_SCENE_H_
