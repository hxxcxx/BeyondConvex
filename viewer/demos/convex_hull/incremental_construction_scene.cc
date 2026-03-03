#include "incremental_construction_scene.h"
#include "core/geometry_utils.h"
#include <imgui.h>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace geometry {

void IncrementalConstructionScene::Reset() {
  points_.clear();
  hull_.clear();
  initialized_ = false;
}

void IncrementalConstructionScene::InitializePoints() {
  if (initialized_) return;
  
  // Create 6 points in a hexagon-like pattern
  double center_x = 400.0;
  double center_y = 300.0;
  double radius = 150.0;
  
  for (int i = 0; i < 6; ++i) {
    double angle = i * M_PI / 3.0;  // 60 degrees apart
    double x = center_x + radius * std::cos(angle);
    double y = center_y + radius * std::sin(angle);
    points_.push_back(Point2D(x, y));
  }
  
  // Add one point inside the hexagon
  points_.push_back(Point2D(center_x, center_y));
  
  initialized_ = true;
  
  // Build convex hull using Jarvis March algorithm
  hull_ = GeometryUtils::JarvisMarch(points_);
}

bool IncrementalConstructionScene::OnMouseClicked(double x, double y) {
  // Initialize if not already done
  if (!initialized_) {
    InitializePoints();
  }
  
  // Add new point
  points_.push_back(Point2D(x, y));
  
  // Rebuild convex hull using Jarvis March algorithm
  hull_ = GeometryUtils::JarvisMarch(points_);
  
  return true;
}

void IncrementalConstructionScene::Render(float canvas_x, float canvas_y,
                                          float canvas_width, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Initialize if first render
  if (!initialized_) {
    InitializePoints();
  }
  
  // Draw all points
  if (show_all_points_) {
    for (size_t i = 0; i < points_.size(); ++i) {
      const Point2D& pt = points_[i];
      ImVec2 pos(canvas_x + pt.x, canvas_y + canvas_height - pt.y);
      
      // Check if point is on hull
      bool on_hull = false;
      for (const auto& edge : hull_) {
        if ((edge.p1.x == pt.x && edge.p1.y == pt.y) ||
            (edge.p2.x == pt.x && edge.p2.y == pt.y)) {
          on_hull = true;
          break;
        }
      }
      
      ImU32 color = on_hull ? IM_COL32(0, 255, 0, 255) : IM_COL32(150, 150, 150, 255);
      float radius = on_hull ? 8.0f : 5.0f;
      
      draw_list->AddCircleFilled(pos, radius, color);
    }
  }
  
  // Draw convex hull edges
  if (show_hull_edges_ && hull_.size() >= 3) {
    for (const auto& edge : hull_) {
      const Point2D& p1 = edge.p1;
      const Point2D& p2 = edge.p2;
      
      float x1 = canvas_x + p1.x;
      float y1 = canvas_y + canvas_height - p1.y;
      float x2 = canvas_x + p2.x;
      float y2 = canvas_y + canvas_height - p2.y;
      
      draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(0, 255, 0, 255), 3.0f);
      
      // Draw arrow on edge
      float mid_x = (x1 + x2) / 2.0f;
      float mid_y = (y1 + y2) / 2.0f;
      DrawArrow(x1, y1, mid_x, mid_y, 10.0f, IM_COL32(0, 200, 0, 255), 2.0f);
    }
  }
  
  // Draw instructions
  if (!initialized_) {
    ImVec2 text_pos(canvas_x + 20, canvas_y + 20);
    draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), 
                       "Click to initialize with 6 points");
  } else {
    ImVec2 text_pos(canvas_x + 20, canvas_y + 20);
    draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), 
                       "Click to add more points");
  }
}

void IncrementalConstructionScene::RenderUI() {
  ImGui::Text("Points: %zu", points_.size());
  ImGui::Text("Hull Size: %zu", hull_.size());
  
  ImGui::Separator();
  ImGui::Checkbox("Show Hull Edges", &show_hull_edges_);
  ImGui::Checkbox("Show All Points", &show_all_points_);
  
  ImGui::Separator();
  if (ImGui::Button("Reset")) {
    Reset();
  }
  
  ImGui::Separator();
  ImGui::TextWrapped("Algorithm: Jarvis March (Gift Wrapping). "
                     "Time complexity: O(nh) where n is the number of points "
                     "and h is the number of hull points.");
  
  if (hull_.size() >= 3) {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "Convex Hull Area: %.1f", 
                       [this]() {
                         double area = 0.0;
                         for (const auto& edge : hull_) {
                           area += (edge.p1.x * edge.p2.y - edge.p2.x * edge.p1.y);
                         }
                         return std::abs(area) / 2.0;
                       }());
  }
}

void IncrementalConstructionScene::DrawArrow(float x1, float y1, float x2, float y2,
                                             float arrow_size, ImU32 color, float thickness) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Draw line
  draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, thickness);
  
  // Calculate arrow head
  float dx = x2 - x1;
  float dy = y2 - y1;
  float len = std::sqrt(dx * dx + dy * dy);
  
  if (len < 1e-5f) return;
  
  // Normalize
  dx /= len;
  dy /= len;
  
  // Arrow head points
  float arrow_x1 = x2 - arrow_size * dx + arrow_size * 0.5f * dy;
  float arrow_y1 = y2 - arrow_size * dy - arrow_size * 0.5f * dx;
  float arrow_x2 = x2 - arrow_size * dx - arrow_size * 0.5f * dy;
  float arrow_y2 = y2 - arrow_size * dy + arrow_size * 0.5f * dx;
  
  // Draw arrow head
  draw_list->AddTriangleFilled(
    ImVec2(x2, y2),
    ImVec2(arrow_x1, arrow_y1),
    ImVec2(arrow_x2, arrow_y2),
    color
  );
}

void IncrementalConstructionScene::DrawLabeledPoint(float x, float y, const char* label, ImU32 color) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  if (label[0] == '\0') return;
  
  // Draw label above point
  ImVec2 label_pos(x, y - 15);
  draw_list->AddText(label_pos, color, label);
}

}  // namespace geometry
