#include "convex_polygon_intersection_scene.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <imgui.h>

namespace geometry {

ConvexPolygonIntersectionScene::ConvexPolygonIntersectionScene()
    : selected_algorithm_(0),
      animation_state_(AnimationState::kIdle),
      animation_timer_(0.0f),
      canvas_x_(0), canvas_y_(0), canvas_width_(1920), canvas_height_(1080) {
  
  // Initialize colors
  convex1_color_[0] = 0.3f;  // R
  convex1_color_[1] = 0.6f;  // G
  convex1_color_[2] = 0.9f;  // B (Light blue)
  
  convex2_color_[0] = 0.9f;  // R
  convex2_color_[1] = 0.5f;  // G
  convex2_color_[2] = 0.3f;  // B (Light orange)
  
  intersection_color_[0] = 0.4f;  // R
  intersection_color_[1] = 0.9f;  // G
  intersection_color_[2] = 0.4f;  // B (Light green)
  
  // Get supported algorithms
  algorithms_ = ConvexPolygonIntersection::GetSupportedAlgorithms();
}

void ConvexPolygonIntersectionScene::Initialize() {
  GenerateRandomConvexPolygonsInternal();
  ResetAnimationInternal();
}

void ConvexPolygonIntersectionScene::Reset() {
  ResetAnimationInternal();
}

bool ConvexPolygonIntersectionScene::OnMouseClicked(double x, double y) {
  // Not used in this scene
  return false;
}

void ConvexPolygonIntersectionScene::Update(float delta_time) {
  animation_timer_ += delta_time;
  
  switch (animation_state_) {
    case AnimationState::kIdle:
      if (animation_timer_ > 0.5f) {
        animation_state_ = AnimationState::kShowingConvex1;
        animation_timer_ = 0.0f;
      }
      break;
      
    case AnimationState::kShowingConvex1:
      if (animation_timer_ > 1.0f) {
        animation_state_ = AnimationState::kShowingConvex2;
        animation_timer_ = 0.0f;
      }
      break;
      
    case AnimationState::kShowingConvex2:
      if (animation_timer_ > 1.0f) {
        animation_state_ = AnimationState::kComputing;
        animation_timer_ = 0.0f;
      }
      break;
      
    case AnimationState::kComputing:
      ComputeIntersection();
      animation_state_ = AnimationState::kShowingResult;
      animation_timer_ = 0.0f;
      break;
      
    case AnimationState::kShowingResult:
      if (animation_timer_ > 2.0f) {
        animation_state_ = AnimationState::kComplete;
      }
      break;
      
    case AnimationState::kComplete:
      // Stay in complete state
      break;
  }
}

void ConvexPolygonIntersectionScene::Render(
    float canvas_x, float canvas_y, float canvas_width, float canvas_height) {
  
  canvas_x_ = canvas_x;
  canvas_y_ = canvas_y;
  canvas_width_ = canvas_width;
  canvas_height_ = canvas_height;
  
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Draw convex polygon 1
  if (animation_state_ >= AnimationState::kShowingConvex1) {
    const auto& vertices1 = convex1_.GetVertices();
    if (!vertices1.empty()) {
      // Draw filled polygon with transparency
      DrawFilledPolygon(vertices1, convex1_color_[0], convex1_color_[1], 
                       convex1_color_[2], 0.3f);
      
      // Draw edges
      for (size_t i = 0; i < vertices1.size(); ++i) {
        size_t next = (i + 1) % vertices1.size();
        DrawLine(vertices1[i], vertices1[next], 2.0f,
                convex1_color_[0], convex1_color_[1], convex1_color_[2]);
      }
      
      // Draw vertices
      for (const auto& v : vertices1) {
        DrawPoint(v, 6.0f, convex1_color_[0], convex1_color_[1], convex1_color_[2]);
      }
      
      // Draw label
      DrawText("Polygon 1", vertices1[0].x + 10, vertices1[0].y - 20, 14.0f,
              convex1_color_[0], convex1_color_[1], convex1_color_[2]);
    }
  }
  
  // Draw convex polygon 2
  if (animation_state_ >= AnimationState::kShowingConvex2) {
    const auto& vertices2 = convex2_.GetVertices();
    if (!vertices2.empty()) {
      // Draw filled polygon with transparency
      DrawFilledPolygon(vertices2, convex2_color_[0], convex2_color_[1],
                       convex2_color_[2], 0.3f);
      
      // Draw edges
      for (size_t i = 0; i < vertices2.size(); ++i) {
        size_t next = (i + 1) % vertices2.size();
        DrawLine(vertices2[i], vertices2[next], 2.0f,
                convex2_color_[0], convex2_color_[1], convex2_color_[2]);
      }
      
      // Draw vertices
      for (const auto& v : vertices2) {
        DrawPoint(v, 6.0f, convex2_color_[0], convex2_color_[1], convex2_color_[2]);
      }
      
      // Draw label
      DrawText("Polygon 2", vertices2[0].x + 10, vertices2[0].y - 20, 14.0f,
              convex2_color_[0], convex2_color_[1], convex2_color_[2]);
    }
  }
  
  // Draw intersection result
  if (animation_state_ >= AnimationState::kShowingResult && !intersection_result_.is_empty) {
    if (!intersection_result_.vertices.empty()) {
      // Draw filled intersection
      DrawFilledPolygon(intersection_result_.vertices, 
                       intersection_color_[0], intersection_color_[1], 
                       intersection_color_[2], 0.5f);
      
      // Draw edges with thicker lines
      for (size_t i = 0; i < intersection_result_.vertices.size(); ++i) {
        size_t next = (i + 1) % intersection_result_.vertices.size();
        DrawLine(intersection_result_.vertices[i], 
                intersection_result_.vertices[next], 3.0f,
                intersection_color_[0], intersection_color_[1], intersection_color_[2]);
      }
      
      // Draw vertices
      for (const auto& v : intersection_result_.vertices) {
        DrawPoint(v, 8.0f, intersection_color_[0], intersection_color_[1], intersection_color_[2]);
      }
      
      // Draw label
      if (!intersection_result_.vertices.empty()) {
        DrawText("Intersection", intersection_result_.vertices[0].x + 10,
                intersection_result_.vertices[0].y - 20, 14.0f,
                intersection_color_[0], intersection_color_[1], intersection_color_[2]);
      }
    }
  }
}

void ConvexPolygonIntersectionScene::RenderUI() {
  // Algorithm selection combo box
  ImGui::Text("Algorithm:");
  if (ImGui::BeginCombo("##Algorithm", 
      ConvexPolygonIntersection::GetAlgorithmName(algorithms_[selected_algorithm_]).c_str())) {
    for (size_t i = 0; i < algorithms_.size(); ++i) {
      bool is_selected = (static_cast<int>(i) == selected_algorithm_);
      if (ImGui::Selectable(ConvexPolygonIntersection::GetAlgorithmName(algorithms_[i]).c_str(), is_selected)) {
        selected_algorithm_ = static_cast<int>(i);
        ResetAnimationInternal();
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  
  ImGui::Text("Complexity: %s", 
              ConvexPolygonIntersection::GetAlgorithmComplexity(algorithms_[selected_algorithm_]).c_str());
  ImGui::Separator();
  
  ImGui::Text("Polygon 1: %zu vertices", convex1_.GetVertices().size());
  ImGui::Text("Polygon 2: %zu vertices", convex2_.GetVertices().size());
  ImGui::Separator();
  
  if (animation_state_ >= AnimationState::kShowingResult) {
    if (intersection_result_.is_empty) {
      ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Result: No Intersection");
    } else if (intersection_result_.is_point) {
      ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), 
                        "Result: Point (%.1f, %.1f)",
                        intersection_result_.vertices[0].x,
                        intersection_result_.vertices[0].y);
    } else if (intersection_result_.is_segment) {
      double length = (intersection_result_.vertices[1] - 
                      intersection_result_.vertices[0]).Length();
      ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), 
                        "Result: Segment, length = %.2f", length);
    } else {
      ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), 
                        "Result: Polygon with %zu vertices", 
                        intersection_result_.vertices.size());
      auto hull = intersection_result_.ToConvexHull();
      ImGui::Text("Area: %.2f", hull.Area());
    }
  } else {
    ImGui::Text("Computing...");
  }
  
  ImGui::Separator();
  
  // Control buttons
  if (ImGui::Button("Generate New Polygons")) {
    GenerateRandomConvexPolygonsInternal();
    ResetAnimationInternal();
  }
  
  if (ImGui::Button("Switch Algorithm")) {
    SwitchAlgorithm();
  }
}

void ConvexPolygonIntersectionScene::GenerateRandomConvexPolygonsInternal() {
  std::random_device rd;
  std::mt19937 gen(rd());
  
  // Generate random points for polygon 1 (left side)
  std::vector<Point2D> points1;
  std::uniform_real_distribution<double> x1_dist(150, 600);
  std::uniform_real_distribution<double> y1_dist(150, 700);
  
  int num_points1 = 5 + std::uniform_int_distribution<>(2, 4)(gen);
  for (int i = 0; i < num_points1; ++i) {
    points1.push_back(Point2D(x1_dist(gen), y1_dist(gen)));
  }
  
  // Generate random points for polygon 2 (right side, overlapping)
  std::vector<Point2D> points2;
  std::uniform_real_distribution<double> x2_dist(500, 1300);
  std::uniform_real_distribution<double> y2_dist(200, 650);
  
  int num_points2 = 5 + std::uniform_int_distribution<>(2, 4)(gen);
  for (int i = 0; i < num_points2; ++i) {
    points2.push_back(Point2D(x2_dist(gen), y2_dist(gen)));
  }
  
  // Build convex hulls
  convex1_ = ConvexHullFactory::Create(
      ConvexHullAlgorithm::MonotoneChain, points1);
  convex2_ = ConvexHullFactory::Create(
      ConvexHullAlgorithm::MonotoneChain, points2);
}

void ConvexPolygonIntersectionScene::ResetAnimationInternal() {
  animation_state_ = AnimationState::kIdle;
  animation_timer_ = 0.0f;
  intersection_result_ = ConvexIntersectionResult();
}

void ConvexPolygonIntersectionScene::SwitchAlgorithm() {
  selected_algorithm_ = (selected_algorithm_ + 1) % algorithms_.size();
  ResetAnimationInternal();
}

void ConvexPolygonIntersectionScene::ComputeIntersection() {
  intersection_result_ = ConvexPolygonIntersection::Intersect(
      convex1_, convex2_, algorithms_[selected_algorithm_]);
}

// Helper rendering functions
void ConvexPolygonIntersectionScene::DrawPolygon(
    const std::vector<Point2D>& vertices, float r, float g, float b, float alpha) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  for (size_t i = 0; i < vertices.size(); ++i) {
    size_t next = (i + 1) % vertices.size();
    ImVec2 p1(canvas_x_ + vertices[i].x, canvas_y_ + vertices[i].y);
    ImVec2 p2(canvas_x_ + vertices[next].x, canvas_y_ + vertices[next].y);
    draw_list->AddLine(p1, p2, IM_COL32(r*255, g*255, b*255, alpha*255), 2.0f);
  }
}

void ConvexPolygonIntersectionScene::DrawFilledPolygon(
    const std::vector<Point2D>& vertices, float r, float g, float b, float alpha) {
  if (vertices.size() < 3) return;
  
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  std::vector<ImVec2> points;
  for (const auto& v : vertices) {
    points.push_back(ImVec2(canvas_x_ + v.x, canvas_y_ + v.y));
  }
  draw_list->AddConvexPolyFilled(points.data(), points.size(),
                                 IM_COL32(r*255, g*255, b*255, alpha*255));
}

void ConvexPolygonIntersectionScene::DrawLine(
    const Point2D& p1, const Point2D& p2, float width, float r, float g, float b) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImVec2 sp1(canvas_x_ + p1.x, canvas_y_ + p1.y);
  ImVec2 sp2(canvas_x_ + p2.x, canvas_y_ + p2.y);
  draw_list->AddLine(sp1, sp2, IM_COL32(r*255, g*255, b*255, 255), width);
}

void ConvexPolygonIntersectionScene::DrawPoint(
    const Point2D& p, float size, float r, float g, float b) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImVec2 center(canvas_x_ + p.x, canvas_y_ + p.y);
  draw_list->AddCircleFilled(center, size, IM_COL32(r*255, g*255, b*255, 255));
}

void ConvexPolygonIntersectionScene::DrawText(
    const std::string& text, float x, float y, float size, float r, float g, float b) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImVec2 pos(canvas_x_ + x, canvas_y_ + y);
  draw_list->AddText(pos, IM_COL32(r*255, g*255, b*255, 255), text.c_str());
}

}  // namespace geometry
