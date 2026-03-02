#include "to_left_test_demo.h"
#include "core/geometry_utils.h"
#include <imgui.h>
#include <cmath>

namespace geometry {

void ToLeftTestDemo::Reset() {
  points_.clear();
  test_complete_ = false;
}

bool ToLeftTestDemo::OnMouseClicked(double x, double y) {
  // If test is complete (3 points), reset and start new test
  if (test_complete_) {
    points_.clear();
    test_complete_ = false;
  }
  
  // Add new point
  points_.push_back(Point2D(x, y));
  
  // Mark test as complete when we have 3 points
  if (points_.size() >= 3) {
    test_complete_ = true;
  }
  
  return true;
}

void ToLeftTestDemo::Render(float canvas_x, float canvas_y, 
                           float canvas_width, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Draw existing points
  for (size_t i = 0; i < points_.size(); ++i) {
    ImVec2 pos(canvas_x + points_[i].x, canvas_y + canvas_height - points_[i].y);
    draw_list->AddCircleFilled(pos, 8.0f, IM_COL32(100, 150, 255, 255));
    
    const char* label = (i == 0) ? "P" : (i == 1) ? "Q" : "S";
    draw_list->AddText(ImVec2(pos.x + 12, pos.y - 12), 
                      IM_COL32(255, 255, 255, 255), label);
  }
  
  // Draw test visualization when we have 3 points
  if (points_.size() >= 3) {
    const Point2D& p = points_[0];
    const Point2D& q = points_[1];
    const Point2D& s = points_[2];
    
    ImVec2 p_pos(canvas_x + p.x, canvas_y + canvas_height - p.y);
    ImVec2 q_pos(canvas_x + q.x, canvas_y + canvas_height - q.y);
    ImVec2 s_pos(canvas_x + s.x, canvas_y + canvas_height - s.y);
    
    // Test orientation using ToLeftTest
    bool is_left = GeometryUtils::ToLeftTest(p, q, s);
    
    // Calculate cross product value for display
    double pqx = q.x - p.x;
    double pqy = q.y - p.y;
    double psx = s.x - p.x;
    double psy = s.y - p.y;
    double cross_value = pqx * psy - pqy * psx;
    
    // Draw reference line P->Q (white, dashed effect with thinner line)
    draw_list->AddLine(p_pos, q_pos, IM_COL32(255, 255, 255, 255), 3.0f);
    
    // Draw arrow head on P->Q
    DrawArrow(p_pos.x, p_pos.y, q_pos.x, q_pos.y, 
             15.0f, IM_COL32(255, 255, 255, 255));
    
    // Draw line from P to S (to show the relationship)
    // Color based on ToLeftTest result
    ImU32 ps_color;
    if (is_left) {
      ps_color = IM_COL32(100, 255, 100, 255);  // Green for left
    } else {
      ps_color = IM_COL32(255, 255, 100, 255);  // Yellow for not left
    }
    draw_list->AddLine(p_pos, s_pos, ps_color, 2.0f);
    
    // Draw points
    draw_list->AddCircleFilled(p_pos, 10.0f, IM_COL32(100, 150, 255, 255));  // P (larger)
    draw_list->AddCircleFilled(q_pos, 8.0f, IM_COL32(200, 200, 200, 255));   // Q (gray)
    draw_list->AddCircleFilled(s_pos, 8.0f, ps_color);  // S (color by result)
    
    // Draw point outlines
    draw_list->AddCircle(p_pos, 10.0f, IM_COL32(255, 255, 255, 255), 2.0f);
    draw_list->AddCircle(q_pos, 8.0f, IM_COL32(255, 255, 255, 255), 1.5f);
    draw_list->AddCircle(s_pos, 8.0f, IM_COL32(255, 255, 255, 255), 1.5f);
    
    // Draw labels
    draw_list->AddText(ImVec2(p_pos.x + 15, p_pos.y - 15), 
                      IM_COL32(255, 255, 255, 255), "P (origin)");
    draw_list->AddText(ImVec2(q_pos.x + 12, q_pos.y - 12), 
                      IM_COL32(200, 200, 200, 255), "Q");
    draw_list->AddText(ImVec2(s_pos.x + 12, s_pos.y - 12), 
                      IM_COL32(255, 255, 255, 255), "S");
    
    // Display result
    const char* result_text;
    ImU32 result_color;
    if (is_left) {
      result_text = "S is LEFT of directed line P->Q";
      result_color = IM_COL32(100, 255, 100, 255);
    } else {
      result_text = "S is NOT LEFT of directed line P->Q";
      result_color = IM_COL32(255, 255, 100, 255);
    }
    
    draw_list->AddText(ImVec2(canvas_x + 10, canvas_y + 10), 
                      result_color, result_text);
    
    // Display cross product value
    char cross_text[64];
    snprintf(cross_text, sizeof(cross_text), "Cross Product: %.2f", cross_value);
    draw_list->AddText(ImVec2(canvas_x + 10, canvas_y + 30), 
                      IM_COL32(200, 200, 200, 255), cross_text);
    
    // Display formula
    draw_list->AddText(ImVec2(canvas_x + 10, canvas_y + 50), 
                      IM_COL32(150, 150, 150, 255), 
                      "Cross = (Qx-Px)(Sy-Py) - (Qy-Py)(Sx-Px)");
  }
}

void ToLeftTestDemo::RenderUI() {
  ImGui::Text("Points: %zu / 3", points_.size());
  ImGui::Separator();
  
  if (test_complete_) {
    ImGui::TextWrapped("Test complete! Click anywhere to start a new test.");
    ImGui::Separator();
    
    const Point2D& p = points_[0];
    const Point2D& q = points_[1];
    const Point2D& s = points_[2];
    
    bool is_left = GeometryUtils::ToLeftTest(p, q, s);
    const char* result;
    if (is_left) {
      result = "Result: S is LEFT of P->Q";
    } else {
      result = "Result: S is NOT LEFT of P->Q";
    }
    
    ImGui::TextWrapped("%s", result);
    
    // Calculate cross product for display
    double pqx = q.x - p.x;
    double pqy = q.y - p.y;
    double psx = s.x - p.x;
    double psy = s.y - p.y;
    double cross = pqx * psy - pqy * psx;
    ImGui::TextWrapped("Cross Product: %.2f", cross);
  } else {
    ImGui::TextWrapped("Click to add points P, Q, S (in order).");
    ImGui::TextWrapped("Tests if S is to the left of directed line P->Q.");
  }
}

void ToLeftTestDemo::DrawArrow(float x1, float y1, float x2, float y2, 
                               float arrow_size, ImU32 color) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Draw line
  draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, 2.0f);
  
  // Calculate arrow head
  float dx = x2 - x1;
  float dy = y2 - y1;
  float len = std::sqrt(dx * dx + dy * dy);
  
  if (len > 0) {
    float arrow_angle = 0.5f;
    
    ImVec2 arrow1(
      x2 - arrow_size * (dx / len * std::cos(arrow_angle) + dy / len * std::sin(arrow_angle)),
      y2 - arrow_size * (dy / len * std::cos(arrow_angle) - dx / len * std::sin(arrow_angle))
    );
    ImVec2 arrow2(
      x2 - arrow_size * (dx / len * std::cos(-arrow_angle) + dy / len * std::sin(-arrow_angle)),
      y2 - arrow_size * (dy / len * std::cos(-arrow_angle) - dx / len * std::sin(-arrow_angle))
    );
    
    draw_list->AddTriangle(ImVec2(x2, y2), arrow1, arrow2, color);
  }
}

}  // namespace geometry
