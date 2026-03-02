#ifndef VIEWER_GEOMETRY_CANVAS_H_
#define VIEWER_GEOMETRY_CANVAS_H_

#include "demo_manager.h"
#include <string>

namespace geometry {

// Geometry canvas for rendering demos
class GeometryCanvas {
 public:
  GeometryCanvas(DemoManager* manager);
  
  // Render the canvas and current demo
  void Render();
  
  // Set canvas background color
  void SetBackgroundColor(float r, float g, float b, float a) {
    clear_color_[0] = r;
    clear_color_[1] = g;
    clear_color_[2] = b;
    clear_color_[3] = a;
  }
  
  // Get canvas title
  const char* GetTitle() const { return title_; }
  void SetTitle(const char* title) { title_ = title; }
  
 private:
  DemoManager* demo_manager_;
  float clear_color_[4] = {0.1f, 0.1f, 0.15f, 1.0f};
  const char* title_ = "Geometry Canvas";
};

}  // namespace geometry

#endif  // VIEWER_GEOMETRY_CANVAS_H_
