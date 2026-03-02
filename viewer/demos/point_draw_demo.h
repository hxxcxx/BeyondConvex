#ifndef VIEWER_DEMOS_POINT_DRAW_DEMO_H_
#define VIEWER_DEMOS_POINT_DRAW_DEMO_H_

#include "demo_base.h"
#include <vector>

namespace geometry {

// Simple point drawing demo
class PointDrawDemo : public DemoBase {
 public:
  std::string Name() const override {
    return "Point Draw";
  }
  
  std::string Description() const override {
    return "Click to add points on the canvas";
  }
  
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  
 private:
  std::vector<Point> points_;
};

}  // namespace geometry

#endif  // VIEWER_DEMOS_POINT_DRAW_DEMO_H_
