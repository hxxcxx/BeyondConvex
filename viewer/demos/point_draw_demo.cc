#include "point_draw_demo.h"
#include <imgui.h>

namespace geometry {

void PointDrawDemo::Reset() {
  points_.clear();
}

bool PointDrawDemo::OnMouseClicked(double x, double y) {
  points_.push_back(Point(x, y));
  return true;
}

void PointDrawDemo::Render(float canvas_x, float canvas_y, 
                           float canvas_width, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Draw all points
  for (const auto& pt : points_) {
    ImVec2 pos(canvas_x + pt.X(), canvas_y + pt.Y());
    draw_list->AddCircleFilled(pos, 5.0f, IM_COL32(255, 100, 100, 255));
  }
}

void PointDrawDemo::RenderUI() {
  ImGui::Text("Points: %zu", points_.size());
  ImGui::Separator();
  ImGui::TextWrapped("Click on canvas to add points.");
}

}  // namespace geometry
