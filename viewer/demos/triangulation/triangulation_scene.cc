#include "triangulation_scene.h"
#include <random>
#include <cmath>
#include <imgui.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace geometry {

TriangulationScene::TriangulationScene()
    : selected_algorithm_(0),
      canvas_x_(0), canvas_y_(0), canvas_width_(1920), canvas_height_(1080),
      show_triangulation_(true) {

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
  ComputeTriangulation();
}

void TriangulationScene::Reset() {
  triangulation_result_ = TriangulationResult();
}

bool TriangulationScene::OnMouseClicked(double x, double y) {
  // Not used in this simplified version
  return false;
}

void TriangulationScene::Update(float delta_time) {
  // No animation needed
}

void TriangulationScene::Render(
    float canvas_x, float canvas_y, float canvas_width, float canvas_height) {

  canvas_x_ = canvas_x;
  canvas_y_ = canvas_y;
  canvas_width_ = canvas_width;
  canvas_height_ = canvas_height;

  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  // Draw polygon edges (simply connect vertices in order)
  if (!polygon_points_.empty()) {
    // Draw edges by connecting vertices in order
    for (size_t i = 0; i < polygon_points_.size(); ++i) {
      size_t next = (i + 1) % polygon_points_.size();
      DrawLine(polygon_points_[i], polygon_points_[next], 3.0f,
              polygon_color_[0], polygon_color_[1], polygon_color_[2]);
    }

    // Draw vertices
    for (const auto& v : polygon_points_) {
      DrawPoint(v, 8.0f, polygon_color_[0], polygon_color_[1], polygon_color_[2]);
    }
  }

  // Draw triangulation result
  if (show_triangulation_ && triangulation_result_.IsValid()) {
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
  ImGui::Text("Polygon: %zu vertices", polygon_points_.size());

  // Triangulation info
  if (triangulation_result_.IsValid()) {
    ImGui::Separator();
    ImGui::Text("Triangles: %zu", triangulation_result_.TriangleCount());
    ImGui::Text("Total Area: %.2f", triangulation_result_.TotalArea());
  }

  ImGui::Separator();

  // Control buttons
  if (ImGui::Button("Generate Random Polygon")) {
    GenerateRandomPolygon();
    ComputeTriangulation();
  }

  ImGui::SameLine();
  if (ImGui::Button("Switch Algorithm")) {
    SwitchAlgorithm();
  }

  ImGui::Separator();

  // Toggle triangulation display
  ImGui::Checkbox("Show Triangulation", &show_triangulation_);
}

void TriangulationScene::GenerateRandomPolygon() {
  std::random_device rd;
  std::mt19937 gen(rd());

  polygon_points_.clear();

  // Center and radius
  double center_x = 800;
  double center_y = 450;
  double radius = 300;

  // Number of vertices
  int num_points = 5 + std::uniform_int_distribution<>(2, 5)(gen);

  // Generate random angles
  std::vector<double> angles;
  std::uniform_real_distribution<double> angle_dist(0, 2.0 * M_PI);
  for (int i = 0; i < num_points; ++i) {
    angles.push_back(angle_dist(gen));
  }

  // Sort angles to ensure CCW order (guarantees simple polygon)
  std::sort(angles.begin(), angles.end());

  // Generate points with random radius variation
  std::uniform_real_distribution<double> radius_dist(0.6, 1.0);
  for (int i = 0; i < num_points; ++i) {
    double r = radius * radius_dist(gen);
    double x = center_x + r * std::cos(angles[i]);
    double y = center_y + r * std::sin(angles[i]);
    polygon_points_.push_back(Point2D(x, y));
  }

  // Don't create concave vertices for now - keep it simple
  // The random radius variation is enough to create interesting shapes
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

}  // namespace geometry
