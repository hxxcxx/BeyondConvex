
#pragma once
#include <string>
#include <vector>
#include "core/point2d.h"

namespace geometry {

// Base class for all geometry scenes
class GeometryScene {
 public:
  virtual ~GeometryScene() = default;
  
  // Scene name for display
  virtual std::string Name() const = 0;
  
  // Scene description
  virtual std::string Description() const = 0;
  
  // Initialize scene
  virtual void Initialize() {}
  
  // Reset scene state
  virtual void Reset() = 0;
  
  // Handle mouse click on canvas
  // Returns true if the click was handled
  virtual bool OnMouseClicked(double x, double y) = 0;
  
  // Render the scene on the canvas
  // canvas_p0: top-left corner of canvas in screen coordinates
  // canvas_sz: size of canvas
  virtual void Render(float canvas_x, float canvas_y, 
                     float canvas_width, float canvas_height) = 0;
  
  // Render UI controls (optional)
  virtual void RenderUI() {}
  
  // Update scene with delta time (optional)
  virtual void Update(float delta_time) {}
  
  // Check if scene is active
  bool IsActive() const { return is_active_; }
  void SetActive(bool active) { is_active_ = active; }
  
 protected:
  bool is_active_ = true;
};

}  // namespace geometry

