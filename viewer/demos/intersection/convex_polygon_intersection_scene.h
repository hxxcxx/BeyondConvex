
#pragma once
#include "../scene_base.h"
#include "../../../src/intersection/convex_polygon_intersection.h"
#include "../../../src/convex_hull/convex_hull_factory.h"
#include <vector>
#include <memory>

namespace geometry {

class ConvexPolygonIntersectionScene : public GeometryScene {
 public:
  ConvexPolygonIntersectionScene();
  virtual ~ConvexPolygonIntersectionScene() = default;
  
  std::string Name() const override {
    return "Convex Polygon Intersection";
  }
  
  std::string Description() const override {
    return "Compute intersection of two convex polygons using different algorithms. "
           "Press SPACE to generate new polygons, A to switch algorithms.";
  }
  
  void Initialize() override;
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  void Update(float delta_time) override;
  
  // Public methods for keyboard input
  void GenerateRandomConvexPolygonsInternal();
  void ResetAnimationInternal();
  void SwitchAlgorithm();
  
 private:
  void ComputeIntersection();
  
  // Helper functions for rendering
  void DrawPolygon(const std::vector<Point2D>& vertices, float r, float g, float b, float alpha);
  void DrawFilledPolygon(const std::vector<Point2D>& vertices, float r, float g, float b, float alpha);
  void DrawLine(const Point2D& p1, const Point2D& p2, float width, float r, float g, float b);
  void DrawPoint(const Point2D& p, float size, float r, float g, float b);
  void DrawText(const std::string& text, float x, float y, float size, float r, float g, float b);
  
  // Two convex polygons
  ConvexHull convex1_;
  ConvexHull convex2_;
  
  // Intersection result
  ConvexIntersectionResult intersection_result_;
  
  // Algorithm selection
  int selected_algorithm_;
  std::vector<ConvexIntersectionAlgorithm> algorithms_;
  
  // Animation state
  enum class AnimationState {
    kIdle,
    kShowingConvex1,
    kShowingConvex2,
    kComputing,
    kShowingResult,
    kComplete
  };
  AnimationState animation_state_;
  float animation_timer_;
  float pulse_phase_;  // For pulsing effect on intersection
  
  // Canvas parameters (set during Render)
  float canvas_x_, canvas_y_, canvas_width_, canvas_height_;
  
  // Visualization parameters
  float convex1_color_[3];
  float convex2_color_[3];
  float intersection_color_[3];
};

}  // namespace geometry

