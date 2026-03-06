#include "triangulation_scene.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <imgui.h>

namespace geometry {

TriangulationScene::TriangulationScene()
    : selected_algorithm_(0),
      animation_state_(AnimationState::kIdle),
      animation_timer_(0.0f),
      sweep_line_y_(0.0f),
      canvas_x_(0), canvas_y_(0), canvas_width_(1920), canvas_height_(1080),
      current_color_index_(0),
      show_sweep_line_(true) {
  
  // Initialize polygon color
  polygon_color_[0] = 0.3f;  // R
  polygon_color_[1] = 0.6f;  // G
  polygon_color_[2] = 0.9f;  // B (Light blue)
  
  // Initialize triangle colors (rainbow colors)
  triangle_colors_[0][0] = 1.0f; triangle_colors_[0][1] = 0.3f; triangle_colors_[0][2] = 0.3f;  // Red
  triangle_colors_[1][0] = 0.3f; triangle_colors_[1][1] = 1.0f; triangle_colors_[1][2] = 0.3f;  // Green
  triangle_colors_[2][0] = 0.3f; triangle_colors_[2][1] = 0.3f; triangle_colors_[2][2] = 1.0f;  // Blue
  
  // Get supported algorithms
  algorithms_ = Triangulation::GetSupportedAlgorithms();
}

void TriangulationScene::Initialize() {
  GenerateRandomPolygon();
  Reset();
}

void TriangulationScene::Reset() {
  animation_state_ = AnimationState::kIdle;
  animation_timer_ = 0.0f;
  sweep_line_y_ = 0.0f;
  triangulation_result_ = TriangulationResult();
  current_color_index_ = 0;
}

bool TriangulationScene::OnMouseClicked(double x, double y) {
  if (animation_state_ == AnimationState::kDrawingPolygon) {
    polygon_points_.push_back(Point2D(x, y));
    return true;
  }
  return false;
}

void TriangulationScene::Update(float delta_time) {
  animation_timer_ += delta_time;
  
  switch (animation_state_) {
    case AnimationState::kIdle:
      if (animation_timer_ > 0.5f) {
        animation_state_ = AnimationState::kDrawingPolygon;
        animation_timer_ = 0.0f;
      }
      break;
      
    case AnimationState::kDrawingPolygon:
      // Wait for user to complete polygon
      break;
      
    case AnimationState::kTriangulating:
      if (show_sweep_line_ && selected_algorithm_ == 0) {
        // Animate sweep line from top to bottom
        double min_y = polygon_points_[0].y;
        double max_y = polygon_points_[0].y;
        for (const auto& v : polygon_points_) {
          min_y = std::min(min_y, v.y);
          max_y = std::max(max_y, v.y);
        }
        
        double sweep_duration = 2.0f;  // 2 seconds to sweep
        double progress = animation_timer_ / sweep_duration;
        sweep_line_y_ = max_y - progress * (max_y - min_y);
        
        if (progress >= 1.0f) {
          ComputeTriangulation();
          animation_state_ = AnimationState::kShowingResult;
          animation_timer_ = 0.0f;
        }
      } else {
        ComputeTriangulation();
        animation_state_ = AnimationState::kShowingResult;
        animation_timer_ = 0.0f;
      }
      break;
      
    case AnimationState::kShowingResult:
      if (animation_timer_ > 3.0f) {
        animation_state_ = AnimationState::kComplete;
      }
      break;
      
    case AnimationState::kComplete:
      // Stay in complete state
      break;
  }
}

void TriangulationScene::Render(
    float canvas_x, float canvas_y, float canvas_width, float canvas_height) {
  
  canvas_x_ = canvas_x;
  canvas_y_ = canvas_y;
  canvas_width_ = canvas_width;
  canvas_height_ = canvas_height;
  
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Draw polygon
  if (!polygon_points_.empty()) {
    // Draw filled polygon
    DrawFilledPolygon(polygon_points_, polygon_color_[0], polygon_color_[1], 
                     polygon_color_[2], 0.3f);
    
    // Draw edges
    for (size_t i = 0; i < polygon_points_.size(); ++i) {
      size_t next = (i + 1) % polygon_points_.size();
      DrawLine(polygon_points_[i], polygon_points_[next], 2.0f,
              polygon_color_[0], polygon_color_[1], polygon_color_[2]);
    }
    
    // Draw vertices
    for (const auto& v : polygon_points_) {
      DrawPoint(v, 6.0f, polygon_color_[0], polygon_color_[1], polygon_color_[2]);
    }
  }
  
  // Draw triangulation result
  if (animation_state_ >= AnimationState::kShowingResult && 
      triangulation_result_.IsValid()) {
    
    for (size_t i = 0; i < triangulation_result_.triangles.size(); ++i) {
      const Triangle& tri = triangulation_result_.triangles[i];
      
      // Use different colors for triangles
      int color_idx = i % 3;
      float r = triangle_colors_[color_idx][0];
      float g = triangle_colors_[color_idx][1];
      float b = triangle_colors_[color_idx][2];
      
      DrawTriangle(tri, r, g, b, 0.6f);
    }
    
    // Draw triangulation edges
    for (const auto& edge : triangulation_result_.edges) {
      DrawLine(edge.p1, edge.p2, 1.5f, 0.5f, 0.5f, 0.5f);
    }
  }
  
  // Draw sweep line (for sweep line algorithm)
  if (animation_state_ == AnimationState::kTriangulating && 
      show_sweep_line_ && selected_algorithm_ == 0) {
    DrawSweepLine(sweep_line_y_);
  }
}

void TriangulationScene::RenderUI() {
  // Algorithm selection combo box
  ImGui::Text("Algorithm:");
  if (ImGui::BeginCombo("##Algorithm", 
      Triangulation::GetAlgorithmName(algorithms_[selected_algorithm_]).c_str())) {
    for (size_t i = 0; i < algorithms_.size(); ++i) {
      bool is_selected = (static_cast<int>(i) == selected_algorithm_);
      if (ImGui::Selectable(Triangulation::GetAlgorithmName(algorithms_[i]).c_str(), is_selected)) {
        selected_algorithm_ = static_cast<int>(i);
        Reset();
        ComputeTriangulation();
      }
      if (is_selected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
  
  ImGui::Text("Complexity: %s", 
              Triangulation::GetAlgorithmComplexity(algorithms_[selected_algorithm_]).c_str());
  ImGui::Separator();
  
  // Polygon info
  if (animation_state_ >= AnimationState::kDrawingPolygon) {
    ImGui::Text("Polygon: %zu vertices", polygon_points_.size());
  }
  
  // Triangulation info
  if (animation_state_ >= AnimationState::kShowingResult && 
      triangulation_result_.IsValid()) {
    ImGui::Separator();
    ImGui::Text("Triangles: %zu", triangulation_result_.TriangleCount());
    ImGui::Text("Total Area: %.2f", triangulation_result_.TotalArea());
  }
  
  ImGui::Separator();
  
  // Control buttons
  if (ImGui::Button("Generate Random Polygon")) {
    GenerateRandomPolygon();
    Reset();
  }
  
  if (polygon_points_.size() >= 3 && animation_state_ == AnimationState::kDrawingPolygon) {
    if (ImGui::Button("Triangulate")) {
      animation_state_ = AnimationState::kTriangulating;
      animation_timer_ = 0.0f;
    }
  }
  
  if (ImGui::Button("Switch Algorithm")) {
    SwitchAlgorithm();
  }
  
  // Toggle sweep line animation
  ImGui::Checkbox("Show Sweep Line", &show_sweep_line_);
  
  // Instructions
  ImGui::Separator();
  ImGui::Text("Instructions:");
  ImGui::BulletText("Click on canvas to add points");
  ImGui::BulletText("Press 'Triangulate' when done");
  ImGui::BulletText("Or use 'Generate Random Polygon'");
}

void TriangulationScene::GenerateRandomPolygon() {
  std::random_device rd;
  std::mt19937 gen(rd());
  
  polygon_points_.clear();
  
  // Generate random points
  std::uniform_real_distribution<double> x_dist(200, 1400);
  std::uniform_real_distribution<double> y_dist(200, 700);
  
  int num_points = 5 + std::uniform_int_distribution<>(2, 4)(gen);
  for (int i = 0; i < num_points; ++i) {
    polygon_points_.push_back(Point2D(x_dist(gen), y_dist(gen)));
  }
  
  // Sort by angle to create a simple polygon
  Point2D centroid(0, 0);
  for (const auto& p : polygon_points_) {
    centroid.x += p.x;
    centroid.y += p.y;
  }
  centroid.x /= polygon_points_.size();
  centroid.y /= polygon_points_.size();
  
  std::sort(polygon_points_.begin(), polygon_points_.end(),
      [&centroid](const Point2D& a, const Point2D& b) {
        double angle_a = std::atan2(a.y - centroid.y, a.x - centroid.x);
        double angle_b = std::atan2(b.y - centroid.y, b.x - centroid.x);
        return angle_a < angle_b;
      });
}

void TriangulationScene::ComputeTriangulation() {
  if (polygon_points_.size() < 3) {
    return;
  }
  
  triangulation_result_ = Triangulation::Triangulate(
      polygon_points_, algorithms_[selected_algorithm_]);
}

void TriangulationScene::SwitchAlgorithm() {
  selected_algorithm_ = (selected_algorithm_ + 1) % algorithms_.size();
  Reset();
  ComputeTriangulation();
}

// Helper rendering functions
void TriangulationScene::DrawPolygon(
    const std::vector<Point2D>& vertices, float r, float g, float b, float alpha) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  for (size_t i = 0; i < vertices.size(); ++i) {
    size_t next = (i + 1) % vertices.size();
    ImVec2 p1(canvas_x_ + vertices[i].x, canvas_y_ + vertices[i].y);
    ImVec2 p2(canvas_x_ + vertices[next].x, canvas_y_ + vertices[next].y);
    draw_list->AddLine(p1, p2, IM_COL32(r*255, g*255, b*255, alpha*255), 2.0f);
  }
}

void TriangulationScene::DrawFilledPolygon(
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

void TriangulationScene::DrawTriangle(
    const Triangle& tri, float r, float g, float b, float alpha) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImVec2 v0(canvas_x_ + tri.v0.x, canvas_y_ + tri.v0.y);
  ImVec2 v1(canvas_x_ + tri.v1.x, canvas_y_ + tri.v1.y);
  ImVec2 v2(canvas_x_ + tri.v2.x, canvas_y_ + tri.v2.y);
  
  ImVec2 points[3] = {v0, v1, v2};
  draw_list->AddTriangleFilled(points[0], points[1], points[2],
                              IM_COL32(r*255, g*255, b*255, alpha*255));
  
  // Draw triangle edges
  draw_list->AddLine(v0, v1, IM_COL32(r*255, g*255, b*255, 255), 1.5f);
  draw_list->AddLine(v1, v2, IM_COL32(r*255, g*255, b*255, 255), 1.5f);
  draw_list->AddLine(v2, v0, IM_COL32(r*255, g*255, b*255, 255), 1.5f);
}

void TriangulationScene::DrawLine(
    const Point2D& p1, const Point2D& p2, float width, float r, float g, float b) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImVec2 sp1(canvas_x_ + p1.x, canvas_y_ + p1.y);
  ImVec2 sp2(canvas_x_ + p2.x, canvas_y_ + p2.y);
  draw_list->AddLine(sp1, sp2, IM_COL32(r*255, g*255, b*255, 255), width);
}

void TriangulationScene::DrawPoint(
    const Point2D& p, float size, float r, float g, float b) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImVec2 center(canvas_x_ + p.x, canvas_y_ + p.y);
  draw_list->AddCircleFilled(center, size, IM_COL32(r*255, g*255, b*255, 255));
}

void TriangulationScene::DrawText(
    const std::string& text, float x, float y, float size, float r, float g, float b) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImVec2 pos(canvas_x_ + x, canvas_y_ + y);
  draw_list->AddText(pos, IM_COL32(r*255, g*255, b*255, 255), text.c_str());
}

void TriangulationScene::DrawSweepLine(double y) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Draw horizontal sweep line
  ImVec2 p1(canvas_x_, canvas_y_ + y);
  ImVec2 p2(canvas_x_ + canvas_width_, canvas_y_ + y);
  draw_list->AddLine(p1, p2, IM_COL32(255, 255, 0, 200), 3.0f);
  
  // Draw label
  std::ostringstream oss;
  oss << "Sweep Line: y = " << std::fixed << std::setprecision(1) << y;
  draw_list->AddText(ImVec2(canvas_x_ + 10, canvas_y_ + y - 20),
                    IM_COL32(255, 255, 0, 255), oss.str().c_str());
}

}  // namespace geometry
