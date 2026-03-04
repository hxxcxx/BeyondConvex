#include "line_segment_intersection_scene.h"
#include <imgui.h>
#include <random>
#include <algorithm>
#include <sstream>

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
  
  // Create animation events showing sweep line progress (LEFT TO RIGHT)
  // Collect all unique x-coordinates from segment endpoints
  std::vector<double> x_coords;
  for (const auto& seg : segments_) {
    x_coords.push_back(seg.p1.x);
    x_coords.push_back(seg.p2.x);
  }
  
  // Sort in ascending order (from left to right)
  std::sort(x_coords.begin(), x_coords.end());
  
  // Remove duplicates
  auto last = std::unique(x_coords.begin(), x_coords.end());
  x_coords.erase(last, x_coords.end());
  
  // Generate animation events
  for (size_t i = 0; i < x_coords.size(); ++i) {
    AnimationEvent event;
    event.position = Point2D(x_coords[i], 300);  // Current x, center y
    
    // Find segments that intersect with this vertical line
    std::vector<int> active_segs;
    for (size_t j = 0; j < segments_.size(); ++j) {
      const Edge2D& seg = segments_[j];
      double min_x = std::min(seg.p1.x, seg.p2.x);
      double max_x = std::max(seg.p1.x, seg.p2.x);
      
      if (x_coords[i] >= min_x && x_coords[i] <= max_x) {
        active_segs.push_back(static_cast<int>(j));
      }
    }
    event.active_segments = active_segs;
    
    // Sort active segments by y-coordinate at current x (sweep line order)
    event.sweep_line_order = active_segs;
    std::sort(event.sweep_line_order.begin(), event.sweep_line_order.end(),
      [this, x_coords_i = x_coords[i]](int a, int b) {
        double y_a = LineSegmentIntersection::GetYAtX(segments_[a], x_coords_i);
        double y_b = LineSegmentIntersection::GetYAtX(segments_[b], x_coords_i);
        return y_a < y_b;
      });
    
    // Build event queue order (segments sorted by left endpoint x)
    std::vector<int> event_queue_order;
    for (size_t j = 0; j < segments_.size(); ++j) {
      double left_x = std::min(segments_[j].p1.x, segments_[j].p2.x);
      if (left_x >= x_coords[i]) {
        event_queue_order.push_back(static_cast<int>(j));
      }
    }
    std::sort(event_queue_order.begin(), event_queue_order.end(),
      [this](int a, int b) {
        double left_x_a = std::min(segments_[a].p1.x, segments_[a].p2.x);
        double left_x_b = std::min(segments_[b].p1.x, segments_[b].p2.x);
        return left_x_a < left_x_b;
      });
    event.event_queue_order = event_queue_order;
    
    // Check for intersections at this x position
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
      event.description = "Starting sweep line (left to right)";
    } else if (i == x_coords.size() - 1) {
      event.description = "Sweep line complete";
    } else {
      event.description = "Sweep line at x = " + std::to_string(static_cast<int>(x_coords[i]));
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
  
  // Draw all segments with consistent color
  for (size_t i = 0; i < segments_.size(); ++i) {
    const Edge2D& seg = segments_[i];
    float x1 = canvas_x + seg.p1.x;
    float y1 = canvas_y + canvas_height - seg.p1.y;
    float x2 = canvas_x + seg.p2.x;
    float y2 = canvas_y + canvas_height - seg.p2.y;
    
    // Draw segment with consistent color (white/light gray)
    draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(200, 200, 200, 255), 2.0f);
    
    // Draw start point number (green)
    std::string start_label = std::to_string(i + 1);
    const char* start_text = start_label.c_str();
    ImVec2 start_text_size = ImGui::CalcTextSize(start_text);
    draw_list->AddText(ImVec2(x1 - start_text_size.x / 2, y1 - 20), 
                       IM_COL32(0, 255, 0, 255), start_text);
    
    // Draw end point number (red)
    std::string end_label = std::to_string(i + 1);
    const char* end_text = end_label.c_str();
    ImVec2 end_text_size = ImGui::CalcTextSize(end_text);
    draw_list->AddText(ImVec2(x2 - end_text_size.x / 2, y2 - 20), 
                       IM_COL32(255, 0, 0, 255), end_text);
  }
  
  // Draw temporary point and line if exists
  if (has_temp_point_) {
    float tx = canvas_x + temp_point_.x;
    float ty = canvas_y + canvas_height - temp_point_.y;
    draw_list->AddCircleFilled(ImVec2(tx, ty), 5.0f, IM_COL32(255, 200, 100, 255));
  }
  
  // Draw all intersections as triangles
  for (const auto& intersection : intersections_) {
    float ix = canvas_x + intersection.point.x;
    float iy = canvas_y + canvas_height - intersection.point.y;
    
    // Draw intersection point as a triangle
    float triangle_size = 10.0f;
    ImVec2 p1(ix, iy - triangle_size);
    ImVec2 p2(ix - triangle_size * 0.866f, iy + triangle_size * 0.5f);
    ImVec2 p3(ix + triangle_size * 0.866f, iy + triangle_size * 0.5f);
    
    draw_list->AddTriangleFilled(p1, p2, p3, IM_COL32(255, 50, 50, 255));
    draw_list->AddTriangle(p1, p2, p3, IM_COL32(255, 255, 255, 255), 2.0f);
  }
  
  // Draw animation sweep line and algorithm state
  if (animation_state_ == AnimationState::Running || animation_state_ == AnimationState::Finished) {
    float sweep_x = canvas_x + current_event_.position.x;
    
    // Draw vertical sweep line
    draw_list->AddLine(ImVec2(sweep_x, canvas_y), 
                      ImVec2(sweep_x, canvas_y + canvas_height),
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
    
    // Draw newly found intersections as triangles
    for (const auto& intersection : current_event_.found_intersections) {
      float ix = canvas_x + intersection.point.x;
      float iy = canvas_y + canvas_height - intersection.point.y;
      
      // Pulsing effect
      float pulse = (std::sin(animation_progress_ * 3.14159f * 2) + 1.0f) * 0.5f;
      float triangle_size = 10.0f + pulse * 4.0f;
      
      ImVec2 p1(ix, iy - triangle_size);
      ImVec2 p2(ix - triangle_size * 0.866f, iy + triangle_size * 0.5f);
      ImVec2 p3(ix + triangle_size * 0.866f, iy + triangle_size * 0.5f);
      
      draw_list->AddTriangleFilled(p1, p2, p3, IM_COL32(255, 100, 100, 200));
    }
    
    // Draw algorithm state visualization (bottom right corner)
    float panel_x = canvas_x + canvas_width - 250;
    float panel_y = canvas_y + canvas_height - 200;
    
    // Draw panel background
    draw_list->AddRectFilled(
      ImVec2(panel_x, panel_y),
      ImVec2(panel_x + 240, panel_y + 190),
      IM_COL32(30, 30, 40, 230)
    );
    draw_list->AddRect(
      ImVec2(panel_x, panel_y),
      ImVec2(panel_x + 240, panel_y + 190),
      IM_COL32(100, 100, 100, 255), 1.5f
    );
    
    // Draw title
    draw_list->AddText(ImVec2(panel_x + 10, panel_y + 10), IM_COL32(255, 255, 0, 255), "Algorithm State");
    
    // Draw sweep line status
    std::stringstream ss;
    ss << "Sweep Line X: " << static_cast<int>(current_event_.position.x);
    draw_list->AddText(ImVec2(panel_x + 10, panel_y + 35), IM_COL32(200, 200, 200, 255), ss.str().c_str());
    
    // Draw active segments in sweep line
    draw_list->AddText(ImVec2(panel_x + 10, panel_y + 55), IM_COL32(150, 255, 150, 255), "Active Segments:");
    float y_offset = 75;
    for (size_t i = 0; i < current_event_.active_segments.size() && i < 5; ++i) {
      int seg_idx = current_event_.active_segments[i];
      std::string seg_text = "  Seg " + std::to_string(seg_idx + 1);
      draw_list->AddText(ImVec2(panel_x + 10, panel_y + y_offset), IM_COL32(200, 200, 200, 255), seg_text.c_str());
      y_offset += 18;
    }
    
    if (current_event_.active_segments.size() > 5) {
      draw_list->AddText(ImVec2(panel_x + 10, panel_y + y_offset), 
                         IM_COL32(150, 150, 150, 255), 
                         (std::string("  ... +") + std::to_string(current_event_.active_segments.size() - 5) + " more").c_str());
    }
    
    // Draw found intersections
    if (!current_event_.found_intersections.empty()) {
      y_offset += 25;
      draw_list->AddText(ImVec2(panel_x + 10, panel_y + y_offset), IM_COL32(255, 100, 100, 255), "New Intersections:");
      y_offset += 20;
      for (size_t i = 0; i < current_event_.found_intersections.size() && i < 3; ++i) {
        const auto& inter = current_event_.found_intersections[i];
        std::string inter_text = "  Seg " + std::to_string(inter.segment1_index + 1) + 
                                " x Seg " + std::to_string(inter.segment2_index + 1);
        draw_list->AddText(ImVec2(panel_x + 10, panel_y + y_offset), 
                           IM_COL32(255, 150, 150, 255), inter_text.c_str());
        y_offset += 18;
      }
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
