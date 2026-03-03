#ifndef VIEWER_DEMOS_TO_LEFT_TEST_SCENE_H_
#define VIEWER_DEMOS_TO_LEFT_TEST_SCENE_H_

#include "../scene_base.h"
#include "core/point2d.h"
#include <vector>

// Forward declarations
struct ImVec2;
using ImU32 = unsigned int;

namespace geometry {

// To-Left Test visualization scene
class ToLeftTestScene : public GeometryScene {
 public:
  std::string Name() const override {
    return "To-Left Test";
  }
  
  std::string Description() const override {
    return "Check if point S is to the left of directed line P->Q (using P as origin)";
  }
  
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  
 private:
  std::vector<Point2D> points_;
  bool test_complete_ = false;  // Flag to indicate test is complete
  
  // Helper to draw arrow
  void DrawArrow(float x1, float y1, float x2, float y2, 
                 float arrow_size, unsigned int color);
};

}  // namespace geometry

#endif  // VIEWER_DEMOS_TO_LEFT_TEST_SCENE_H_
