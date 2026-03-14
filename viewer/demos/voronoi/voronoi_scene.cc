#include "voronoi_scene.h"
#include <imgui.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace geometry {

VoronoiScene::VoronoiScene()
    : show_voronoi_(true),
      show_sites_(true),
      show_edges_(true),
      bounds_min_x_(0.0),
      bounds_min_y_(0.0),
      bounds_max_x_(800.0),
      bounds_max_y_(600.0),
      site_radius_(5.0f),
      current_algorithm_index_(0) {
  
  site_color_[0] = 1.0f;
  site_color_[1] = 0.0f;
  site_color_[2] = 0.0f;
  
  edge_color_[0] = 0.0f;
  edge_color_[1] = 0.8f;
  edge_color_[2] = 1.0f;
  
  InitializeAlgorithms();
}

void VoronoiScene::Initialize() {
  InitializeAlgorithms();
}

void VoronoiScene::InitializeAlgorithms() {
  available_algorithms_ = Voronoi::GetAvailableAlgorithms();
  current_algorithm_index_ = 0;
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
  // Algorithm selection combo box (like ConvexPolygonIntersectionScene)
  ImGui::Text("Algorithm:");
  if (ImGui::BeginCombo("##Algorithm", 
      Voronoi::GetAlgorithmInfo(available_algorithms_[current_algorithm_index_])->Name().c_str())) {
    for (size_t i = 0; i < available_algorithms_.size(); ++i) {
      bool is_selected = (static_cast<int>(i) == current_algorithm_index_);
      auto algo = Voronoi::GetAlgorithmInfo(available_algorithms_[i]);
      
      if (ImGui::Selectable(algo->Name().c_str(), is_selected)) {
        if (current_algorithm_index_ != static_cast<int>(i)) {
          current_algorithm_index_ = static_cast<int>(i);
          
          // If switching to Divide & Conquer, auto-generate random points
          if (available_algorithms_[i] == VoronoiAlgorithmType::kDivideConquer) {
            GenerateRandomSites(8);  // Generate 8 random sites
            GenerateVoronoi();
          } else if (!sites_.empty()) {
            GenerateVoronoi();  // Regenerate with new algorithm
          }
        }
      }
      
      // Show tooltip with description on hover
      if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Complexity: %s", algo->Complexity().c_str());
        ImGui::TextWrapped("Description: %s", algo->Description().c_str());
        ImGui::EndTooltip();
      }
      
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  
  ImGui::Text("Complexity: %s", 
              Voronoi::GetAlgorithmInfo(available_algorithms_[current_algorithm_index_])->Complexity().c_str());
  
  ImGui::Separator();
  
  // Scene Info - Statistics
  ImGui::Text("Sites: %zu", sites_.size());
  ImGui::Text("Cells: %zu", voronoi_result_.CellCount());
  ImGui::Text("Edges: %zu", voronoi_result_.EdgeCount());
  
  ImGui::Separator();
  
  // Display options
  ImGui::Checkbox("Show Sites", &show_sites_);
  ImGui::Checkbox("Show Edges", &show_edges_);
  
  ImGui::Separator();
  
  // Actions
  if (ImGui::Button("Clear All")) {
    Clear();
  }
  
  ImGui::Separator();
  
  // Instructions
  ImGui::Text("Instructions:");
  ImGui::BulletText("Left click: Add site");
  ImGui::BulletText("Switch algorithm to compare");
  ImGui::BulletText("Canvas size = World bounds");
}

void VoronoiScene::GenerateRandomSites(int count) {
  sites_.clear();
  
  // Generate random sites within bounds (with some padding)
  double padding = 50.0;
  double min_x = bounds_min_x_ + padding;
  double max_x = bounds_max_x_ - padding;
  double min_y = bounds_min_y_ + padding;
  double max_y = bounds_max_y_ - padding;
  
  for (int i = 0; i < count; ++i) {
    double x = min_x + (double)rand() / RAND_MAX * (max_x - min_x);
    double y = min_y + (double)rand() / RAND_MAX * (max_y - min_y);
    sites_.push_back(Point2D(x, y));
  }
  
  // Sort by x-coordinate for better divide & conquer performance
  std::sort(sites_.begin(), sites_.end(),
      [](const Point2D& a, const Point2D& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
      });
}

void VoronoiScene::GenerateVoronoi() {
  if (available_algorithms_.empty()) {
    return;
  }
  
  // Get current algorithm
  VoronoiAlgorithmType current_algo = available_algorithms_[current_algorithm_index_];
  
  // Create bounds
  VoronoiBounds bounds(bounds_min_x_, bounds_min_y_, bounds_max_x_, bounds_max_y_);
  
  // Generate Voronoi diagram with selected algorithm
  voronoi_result_ = Voronoi::Generate(sites_, current_algo, bounds);
}

void VoronoiScene::Clear() {
  sites_.clear();
  voronoi_result_ = VoronoiDiagramResult();
}

}  // namespace geometry
