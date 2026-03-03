#include "convex_hull_demo_scene.h"
#include <imgui.h>

namespace geometry {

void ConvexHullDemoScene::Reset() {
  points_.clear();
  hull_ = ConvexHull();
  initialized_ = false;
}

void ConvexHullDemoScene::RebuildHull() {
  if (points_.size() >= 3) {
    try {
      hull_ = ConvexHullFactory::Create(current_algorithm_, points_);
    } catch (const std::runtime_error& e) {
      // Algorithm not implemented, handle gracefully
      hull_ = ConvexHull();
    }
  }
}

bool ConvexHullDemoScene::OnMouseClicked(double x, double y) {
  points_.push_back(Point2D(x, y));
  initialized_ = true;
  RebuildHull();
  return true;
}

void ConvexHullDemoScene::Render(float canvas_x, float canvas_y,
                                 float canvas_width, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Draw all points
  for (const auto& pt : points_) {
    ImVec2 pos(canvas_x + pt.x, canvas_y + canvas_height - pt.y);
    draw_list->AddCircleFilled(pos, 5.0f, IM_COL32(150, 150, 150, 255));
  }
  
  // Draw convex hull
  if (hull_.Size() >= 3) {
    std::vector<Edge2D> edges = hull_.GetEdges();
    for (const auto& edge : edges) {
      float x1 = canvas_x + edge.p1.x;
      float y1 = canvas_y + canvas_height - edge.p1.y;
      float x2 = canvas_x + edge.p2.x;
      float y2 = canvas_y + canvas_height - edge.p2.y;
      draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(0, 255, 0, 255), 3.0f);
    }
  }
}

void ConvexHullDemoScene::RenderUI() {
  ImGui::Text("Points: %zu", points_.size());
  ImGui::Text("Hull Size: %zu", hull_.Size());
  
  ImGui::Separator();
  
  // Algorithm selection
  ImGui::Text("Algorithm:");
  if (ImGui::RadioButton("Jarvis March", 
                        current_algorithm_ == ConvexHullAlgorithm::JarvisMarch)) {
    SetAlgorithm(ConvexHullAlgorithm::JarvisMarch);
  }
  
  // Future algorithms can be added here
  // if (ImGui::RadioButton("Graham Scan", 
  //                       current_algorithm_ == ConvexHullAlgorithm::GrahamScan)) {
  //   SetAlgorithm(ConvexHullAlgorithm::GrahamScan);
  // }
  
  ImGui::Separator();
  
  // Display algorithm info
  ImGui::Text("Name: %s", 
              GetAlgorithmName(current_algorithm_).c_str());
  ImGui::Text("Complexity: %s", 
              GetAlgorithmComplexity(current_algorithm_).c_str());
  
  if (hull_.Size() >= 3) {
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "Area: %.1f", hull_.Area());
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "Perimeter: %.1f", hull_.Perimeter());
  }
  
  ImGui::Separator();
  if (ImGui::Button("Reset")) {
    Reset();
  }
}

}  // namespace geometry
