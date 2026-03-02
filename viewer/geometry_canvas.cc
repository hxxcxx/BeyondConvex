#include "geometry_canvas.h"
#include <imgui.h>

namespace geometry {

GeometryCanvas::GeometryCanvas(DemoManager* manager)
    : demo_manager_(manager) {
}

void GeometryCanvas::Render() {
  // Get main window size
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  float window_width = viewport->Size.x;
  float window_height = viewport->Size.y;
  
  // Calculate canvas size (right side, full height minus menu bar)
  float canvas_x = 310.0f;  // Left margin for control panels
  float canvas_y = 20.0f;   // Menu bar height
  float canvas_width = window_width - canvas_x - 10.0f;  // Right margin
  float canvas_height = window_height - canvas_y - 10.0f;  // Bottom margin
  
  // Set canvas window position and size
  ImGui::SetNextWindowPos(ImVec2(canvas_x, canvas_y), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(canvas_width, canvas_height), ImGuiCond_Always);
  
  ImGui::Begin(GetTitle(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
  
  // Get canvas position and size
  ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
  ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
  
  // Ensure minimum size
  if (canvas_sz.x < 400.0f) canvas_sz.x = 400.0f;
  if (canvas_sz.y < 300.0f) canvas_sz.y = 300.0f;
  
  ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
  
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Draw canvas background
  draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(
    (int)(clear_color_[0] * 255),
    (int)(clear_color_[1] * 255),
    (int)(clear_color_[2] * 255),
    (int)(clear_color_[3] * 255)
  ));
  
  // Draw canvas border
  draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
  
  // Handle mouse input for current demo
  if (ImGui::IsMouseHoveringRect(canvas_p0, canvas_p1)) {
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      ImVec2 mouse_pos = ImGui::GetMousePos();
      double x = mouse_pos.x - canvas_p0.x;
      double y = canvas_sz.y - (mouse_pos.y - canvas_p0.y);  // Flip Y to math coordinates
      
      if (auto* demo = demo_manager_->GetCurrentDemo()) {
        demo->OnMouseClicked(x, y);
      }
    }
  }
  
  // Render current demo
  if (auto* demo = demo_manager_->GetCurrentDemo()) {
    demo->Render(canvas_p0.x, canvas_p0.y, canvas_sz.x, canvas_sz.y);
  }
  
  ImGui::End();
}

}  // namespace geometry
