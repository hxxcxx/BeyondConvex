#include "line_segment_intersection_scene.h"
#include <imgui.h>
#include <random>
#include <algorithm>

namespace geometry {

LineSegmentIntersectionScene::LineSegmentIntersectionScene()
    : has_temp_point_(false), use_brute_force_(false),
      animation_state_(AnimationState::Idle), animation_progress_(0.0f), animation_speed_(1.0f) {
  ClearSegments();
}

void LineSegmentIntersectionScene::Reset() {
  ClearSegments();
}

void LineSegmentIntersectionScene::ClearSegments() {
  segments_.clear();
  intersections_.clear();
  temp_point_ = Point2D();
  has_temp_point_ = false;
  animation_state_ = AnimationState::Idle;
  animation_events_ = std::queue<AnimationEvent>();
  current_event_ = AnimationEvent();
  animation_progress_ = 0.0f;
}

void LineSegmentIntersectionScene::GenerateRandomSegments() {
  ClearSegments();
  
  // Random number generator
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> x_dist(50, 750);
  std::uniform_real_distribution<> y_dist(50, 550);
  
  // Generate 10-15 random segments
  int num_segments = 10 + (gen() % 6);
  
  for (int i = 0; i < num_segments; ++i) {
    Point2D p1(x_dist(gen), y_dist(gen));
    Point2D p2(x_dist(gen), y_dist(gen));
    
    // Ensure segment is not too short
    while (p1.DistanceTo(p2) < 30) {
      p2 = Point2D(x_dist(gen), y_dist(gen));
    }
    
    segments_.emplace_back(p1, p2);
  }
  
  // Start animation
  FindIntersectionsWithAnimation();
}

void LineSegmentIntersectionScene::FindIntersectionsWithAnimation() {
  // Clear previous animation
  animation_events_ = std::queue<AnimationEvent>();
  intersections_.clear();
  
  if (segments_.empty()) {
    return;
  }
  
  // Create animation events showing sweep line progress
  // Collect all unique y-coordinates from segment endpoints
  std::vector<double> y_coords;
  for (const auto& seg : segments_) {
    y_coords.push_back(seg.p1.y);
    y_coords.push_back(seg.p2.y);
  }
  
  // Sort in descending order (from top to bottom)
  std::sort(y_coords.begin(), y_coords.end(), std::greater<double>());
  
  // Remove duplicates
  auto last = std::unique(y_coords.begin(), y_coords.end());
  y_coords.erase(last, y_coords.end());
  
  // Generate animation events
  for (size_t i = 0; i < y_coords.size(); ++i) {
    AnimationEvent event;
    event.position = Point2D(400, y_coords[i]);  // Center x, current y
    
    // Find segments that intersect with this horizontal line
    std::vector<int> active_segs;
    for (size_t j = 0; j < segments_.size(); ++j) {
      const Edge2D& seg = segments_[j];
      double min_y = std::min(seg.p1.y, seg.p2.y);
      double max_y = std::max(seg.p1.y, seg.p2.y);
      
      if (y_coords[i] >= min_y && y_coords[i] <= max_y) {
        active_segs.push_back(static_cast<int>(j));
      }
    }
    event.active_segments = active_segs;
    
    // Check for intersections at this level
    for (size_t j = 0; j < active_segs.size(); ++j) {
      for (size_t k = j + 1; k < active_segs.size(); ++k) {
        Point2D intersection;
        if (LineSegmentIntersection::SegmentsIntersect(
                segments_[active_segs[j]], segments_[active_segs[k]], intersection)) {
          
          // Check if this intersection is already recorded
          bool exists = false;
          for (const auto& existing : intersections_) {
            if (existing.point.DistanceTo(intersection) < 1.0) {
              exists = true;
              break;
            }
          }
          
          if (!exists) {
            event.found_intersections.emplace_back(intersection, active_segs[j], active_segs[k]);
            intersections_.push_back(event.found_intersections.back());
          }
        }
      }
    }
    
    // Set description
    if (i == 0) {
      event.description = "Starting sweep line";
    } else if (i == y_coords.size() - 1) {
      event.description = "Sweep line complete";
    } else {
      event.description = "Sweep line at y = " + std::to_string(static_cast<int>(y_coords[i]));
    }
    
    animation_events_.push(event);
  }
  
  // Start animation
  animation_state_ = AnimationState::Running;
  animation_progress_ = 0.0f;
  
  if (!animation_events_.empty()) {
    current_event_ = animation_events_.front();
  }
}

void LineSegmentIntersectionScene::UpdateAnimation(float delta_time) {
  if (animation_state_ != AnimationState::Running) {
    return;
  }
  
  animation_progress_ += delta_time * animation_speed_;
  
  if (animation_progress_ >= 1.0f) {
    // Move to next event
    animation_progress_ = 0.0f;
    animation_events_.pop();
    
    if (animation_events_.empty()) {
      animation_state_ = AnimationState::Finished;
    } else {
      current_event_ = animation_events_.front();
    }
  }
}

bool LineSegmentIntersectionScene::OnMouseClicked(double x, double y) {
  Point2D clicked_point(x, y);
  
  if (!has_temp_point_) {
    // First click: store temporary point
    temp_point_ = clicked_point;
    has_temp_point_ = true;
  } else {
    // Second click: create segment
    segments_.emplace_back(temp_point_, clicked_point);
    has_temp_point_ = false;
    FindIntersections();
  }
  
  return true;
}

void LineSegmentIntersectionScene::Update(float delta_time) {
  UpdateAnimation(delta_time);
}

void LineSegmentIntersectionScene::FindIntersections() {
  if (use_brute_force_) {
    intersections_ = LineSegmentIntersection::FindAllIntersectionsBruteForce(segments_);
  } else {
    intersections_ = LineSegmentIntersection::FindAllIntersections(segments_);
  }
}

void LineSegmentIntersectionScene::Render(float canvas_x, float canvas_y,
                                         float canvas_width, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Draw all segments
  for (const auto& seg : segments_) {
    float x1 = canvas_x + seg.p1.x;
    float y1 = canvas_y + canvas_height - seg.p1.y;
    float x2 = canvas_x + seg.p2.x;
    float y2 = canvas_y + canvas_height - seg.p2.y;
    
    draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(100, 150, 255, 255), 2.0f);
    
    // Draw endpoints
    draw_list->AddCircleFilled(ImVec2(x1, y1), 4.0f, IM_COL32(100, 150, 255, 255));
    draw_list->AddCircleFilled(ImVec2(x2, y2), 4.0f, IM_COL32(100, 150, 255, 255));
  }
  
  // Draw temporary point and line if exists
  if (has_temp_point_) {
    float tx = canvas_x + temp_point_.x;
    float ty = canvas_y + canvas_height - temp_point_.y;
    draw_list->AddCircleFilled(ImVec2(tx, ty), 5.0f, IM_COL32(255, 200, 100, 255));
  }
  
  // Draw intersections
  for (const auto& intersection : intersections_) {
    float ix = canvas_x + intersection.point.x;
    float iy = canvas_y + canvas_height - intersection.point.y;
    
    // Draw intersection point as a red circle
    draw_list->AddCircleFilled(ImVec2(ix, iy), 8.0f, IM_COL32(255, 50, 50, 255));
    draw_list->AddCircle(ImVec2(ix, iy), 8.0f, IM_COL32(255, 255, 255, 255), 2.0f);
  }
  
  // Draw animation sweep line
  if (animation_state_ == AnimationState::Running || animation_state_ == AnimationState::Finished) {
    float sweep_y = canvas_y + canvas_height - current_event_.position.y;
    
    // Draw sweep line
    draw_list->AddLine(ImVec2(canvas_x, sweep_y), 
                      ImVec2(canvas_x + canvas_width, sweep_y),
                      IM_COL32(255, 255, 0, 150), 2.0f);
    
    // Highlight active segments
    for (int seg_idx : current_event_.active_segments) {
      const Edge2D& seg = segments_[seg_idx];
      float x1 = canvas_x + seg.p1.x;
      float y1 = canvas_y + canvas_height - seg.p1.y;
      float x2 = canvas_x + seg.p2.x;
      float y2 = canvas_y + canvas_height - seg.p2.y;
      
      draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(255, 255, 0, 255), 4.0f);
    }
    
    // Draw newly found intersections
    for (const auto& intersection : current_event_.found_intersections) {
      float ix = canvas_x + intersection.point.x;
      float iy = canvas_y + canvas_height - intersection.point.y;
      
      // Pulsing effect
      float pulse = (std::sin(animation_progress_ * 3.14159f * 2) + 1.0f) * 0.5f;
      float radius = 8.0f + pulse * 4.0f;
      
      draw_list->AddCircleFilled(ImVec2(ix, iy), radius, IM_COL32(255, 100, 100, 200));
    }
  }
}

void LineSegmentIntersectionScene::RenderUI() {
  ImGui::Text("Segments: %zu", segments_.size());
  ImGui::Text("Intersections: %zu", intersections_.size());
  
  ImGui::Separator();
  
  // Algorithm selection
  ImGui::Text("Algorithm:");
  if (ImGui::RadioButton("Bentley-Ottmann", !use_brute_force_)) {
    use_brute_force_ = false;
    FindIntersections();
  }
  if (ImGui::RadioButton("Brute Force (O(n²))", use_brute_force_)) {
    use_brute_force_ = true;
    FindIntersections();
  }
  
  ImGui::Separator();
  
  // Random generation and animation
  if (ImGui::Button("Generate Random")) {
    GenerateRandomSegments();
  }
  
  if (ImGui::Button("Run Animation")) {
    if (!segments_.empty()) {
      FindIntersectionsWithAnimation();
    }
  }
  
  // Animation speed control
  ImGui::SliderFloat("Speed", &animation_speed_, 0.1f, 3.0f, "%.1fx");
  
  // Animation status
  if (animation_state_ == AnimationState::Running) {
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "Animation Running...");
    ImGui::Text("Progress: %d/%zu", 
                static_cast<int>(animation_events_.size()) - 
                static_cast<int>(animation_events_.size()),
                static_cast<size_t>(animation_events_.size()) + 
                static_cast<size_t>(animation_events_.size()));
    ImGui::Text("Active Segments: %zu", current_event_.active_segments.size());
    if (!current_event_.description.empty()) {
      ImGui::Text("Status: %s", current_event_.description.c_str());
    }
  } else if (animation_state_ == AnimationState::Finished) {
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "Animation Complete!");
    if (ImGui::Button("Replay")) {
      FindIntersectionsWithAnimation();
    }
  }
  
  ImGui::Separator();
  
  // Instructions
  ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Instructions:");
  ImGui::BulletText("Click 'Generate Random' for random segments");
  ImGui::BulletText("Or click twice to create custom segment");
  ImGui::BulletText("Click 'Run Animation' to visualize");
  ImGui::BulletText("Intersections shown in red");
  
  ImGui::Separator();
  
  // Statistics
  if (!intersections_.empty()) {
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "Found %zu intersection(s)", intersections_.size());
  }
  
  ImGui::Separator();
  
  if (ImGui::Button("Clear All")) {
    ClearSegments();
  }
  
  if (ImGui::Button("Undo Last")) {
    if (!segments_.empty()) {
      segments_.pop_back();
      has_temp_point_ = false;
      FindIntersections();
    }
  }
}

}  // namespace geometry
