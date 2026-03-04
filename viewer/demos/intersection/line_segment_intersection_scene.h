#ifndef VIEWER_DEMOS_INTERSECTION_LINE_SEGMENT_INTERSECTION_SCENE_H_
#define VIEWER_DEMOS_INTERSECTION_LINE_SEGMENT_INTERSECTION_SCENE_H_

#include "../scene_base.h"
#include "../../../src/core/point2d.h"
#include "../../../src/core/edge2d.h"
#include "../../../src/intersection/line_segment_intersection.h"
#include <vector>
#include <queue>

namespace geometry {

// Animation state for sweep line
enum class AnimationState {
  Idle,           // Not animating
  Running,        // Animation in progress
  Finished        // Animation complete
};

// Animation event for visualization
struct AnimationEvent {
  Point2D position;
  std::vector<int> active_segments;
  std::vector<IntersectionPoint> found_intersections;
  std::string description;
};

class LineSegmentIntersectionScene : public GeometryScene {
 public:
  LineSegmentIntersectionScene();
  
  std::string Name() const override {
    return "Line Segment Intersection";
  }
  
  std::string Description() const override {
    return "Bentley-Ottmann sweep line algorithm (left to right). Click to add line segments.";
  }
  
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y,
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  void Update(float delta_time) override;
  
 private:
  std::vector<Edge2D> segments_;
  std::vector<IntersectionPoint> intersections_;
  Point2D temp_point_;  // Temporary point for creating segments
  bool has_temp_point_;
  bool use_brute_force_;  // Use brute force instead of Bentley-Ottmann
  
  // Animation
  AnimationState animation_state_;
  std::queue<AnimationEvent> animation_events_;
  AnimationEvent current_event_;
  float animation_progress_;
  float animation_speed_;
  
  void FindIntersections();
  void FindIntersectionsWithAnimation();
  void ClearSegments();
  void GenerateRandomSegments();
  void UpdateAnimation(float delta_time);
};

}  // namespace geometry

#endif  // VIEWER_DEMOS_INTERSECTION_LINE_SEGMENT_INTERSECTION_SCENE_H_
