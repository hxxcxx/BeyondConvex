#include "voronoi_scene.h"
#include <imgui.h>
#include <cmath>

namespace geometry {

VoronoiScene::VoronoiScene()
    : show_voronoi_(true),
      show_sites_(true),
      show_edges_(true),
      bounds_min_x_(0.0),
      bounds_min_y_(0.0),
      bounds_max_x_(800.0),
      bounds_max_y_(600.0),
      site_radius_(5.0f) {
  
  site_color_[0] = 1.0f;
  site_color_[1] = 0.0f;
  site_color_[2] = 0.0f;
  
  edge_color_[0] = 0.0f;
  edge_color_[1] = 0.8f;
  edge_color_[2] = 1.0f;
}

void VoronoiScene::Initialize() {
}

void VoronoiScene::Reset() {
  Clear();
}

void VoronoiScene::Update(float delta_time) {
  (void)delta_time;
}

bool VoronoiScene::OnMouseClicked(double x, double y) {
  sites_.emplace_back(x, y);
  GenerateVoronoi();
  return true;
}

void VoronoiScene::Render(float canvas_x, float canvas_y, 
                         float canvas_width, float canvas_height) {
  // Update bounds to match canvas size
  bounds_max_x_ = canvas_width;
  bounds_max_y_ = canvas_height;
  
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImVec2 canvas_p0 = ImVec2(canvas_x, canvas_y);
  ImVec2 canvas_sz = ImVec2(canvas_width, canvas_height);
  
  if (show_voronoi_) {
    // Draw Voronoi edges
    if (show_edges_) {
      for (const auto& edge : voronoi_result_.edges) {
        // Direct mapping: world coordinates = screen coordinates
        float sx1 = canvas_p0.x + (float)edge.p1.x;
        float sy1 = canvas_p0.y + canvas_sz.y - (float)edge.p1.y;  // Flip Y
        float sx2 = canvas_p0.x + (float)edge.p2.x;
        float sy2 = canvas_p0.y + canvas_sz.y - (float)edge.p2.y;  // Flip Y
        
        draw_list->AddLine(ImVec2(sx1, sy1), ImVec2(sx2, sy2),
          IM_COL32((int)(edge_color_[0] * 255), (int)(edge_color_[1] * 255), 
                   (int)(edge_color_[2] * 255), 255), 2.0f);
      }
    }
  }
  
  // Draw sites
  if (show_sites_) {
    for (const auto& site : sites_) {
      float sx = canvas_p0.x + (float)site.x;
      float sy = canvas_p0.y + canvas_sz.y - (float)site.y;  // Flip Y
      
      draw_list->AddCircleFilled(ImVec2(sx, sy), site_radius_,
        IM_COL32((int)(site_color_[0] * 255), (int)(site_color_[1] * 255), 
                 (int)(site_color_[2] * 255), 255));
    }
  }
}

void VoronoiScene::RenderUI() {
  ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(250, 200), ImGuiCond_FirstUseEver);
  
  ImGui::Begin("Voronoi Diagram");
  
  ImGui::Text("Sites: %zu", sites_.size());
  ImGui::Text("Cells: %zu", voronoi_result_.CellCount());
  
  ImGui::Separator();
  
  if (ImGui::Button("Clear All")) {
    Clear();
  }
  
  ImGui::Separator();
  
  ImGui::Checkbox("Show Sites", &show_sites_);
  ImGui::Checkbox("Show Edges", &show_edges_);
  
  ImGui::Separator();
  
  ImGui::Text("Instructions:");
  ImGui::BulletText("Left click: Add site");
  ImGui::BulletText("Canvas size = World bounds");
  
  ImGui::End();
}

void VoronoiScene::GenerateVoronoi() {
  voronoi_result_ = VoronoiDiagram::Generate(
      sites_,
      bounds_min_x_,
      bounds_min_y_,
      bounds_max_x_,
      bounds_max_y_);
}

void VoronoiScene::Clear() {
  sites_.clear();
  voronoi_result_ = VoronoiDiagramResult();
}

}  // namespace geometry
