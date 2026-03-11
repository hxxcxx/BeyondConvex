#include "dcel_test_scene.h"
#include <imgui.h>
#include <random>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace geometry {

DCELTestScene::DCELTestScene()
    : current_mode_(Mode::PointLocation),
      highlighted_face_(nullptr),
      show_circumcircles_(false),
      drawing_line_(false),
      line_complete_(false),
      canvas_min_x_(0.0),
      canvas_min_y_(0.0),
      canvas_max_x_(800.0),
      canvas_max_y_(600.0),
      num_random_points_(20),
      point_margin_(50.0),
      key_1_pressed_(false),
      key_2_pressed_(false),
      key_3_pressed_(false),
      key_r_pressed_(false) {
  
  InitializeDCEL();
}

std::string DCELTestScene::Name() const {
  return "DCEL Test";
}

std::string DCELTestScene::Description() const {
  return "Test DCEL data structure with triangulation. Press 1/2/3 to switch modes, R to regenerate.";
}

void DCELTestScene::Reset() {
  InitializeDCEL();
  highlighted_face_ = nullptr;
  drawing_line_ = false;
  line_complete_ = false;
  swept_faces_.clear();
  show_circumcircles_ = false;
}

void DCELTestScene::Update(float delta_time) {
  // Check for key presses
  if (ImGui::IsKeyPressed(ImGuiKey_1)) {
    current_mode_ = Mode::PointLocation;
    highlighted_face_ = nullptr;
  }
  if (ImGui::IsKeyPressed(ImGuiKey_2)) {
    current_mode_ = Mode::Circumcircle;
    show_circumcircles_ = !show_circumcircles_;
  }
  if (ImGui::IsKeyPressed(ImGuiKey_3)) {
    current_mode_ = Mode::LineSweep;
    drawing_line_ = false;
    line_complete_ = false;
    swept_faces_.clear();
  }
  if (ImGui::IsKeyPressed(ImGuiKey_R)) {
    InitializeDCEL();
    highlighted_face_ = nullptr;
    drawing_line_ = false;
    line_complete_ = false;
    swept_faces_.clear();
  }
}

bool DCELTestScene::OnMouseClicked(double x, double y) {
  Point2D mouse_point(x, y);
  
  switch (current_mode_) {
    case Mode::PointLocation:
      HandlePointLocation(mouse_point);
      break;
      
    case Mode::LineSweep:
      if (!drawing_line_) {
        line_start_ = mouse_point;
        line_end_ = mouse_point;
        drawing_line_ = true;
        line_complete_ = false;
        swept_faces_.clear();
      } else {
        line_end_ = mouse_point;
        drawing_line_ = false;
        line_complete_ = true;
        HandleLineSweep();
      }
      break;
      
    default:
      break;
  }
  
  return true;
}

void DCELTestScene::Render(float canvas_x, float canvas_y, 
                           float canvas_width, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Update canvas bounds
  canvas_min_x_ = canvas_x;
  canvas_min_y_ = canvas_y;
  canvas_max_x_ = canvas_x + canvas_width;
  canvas_max_y_ = canvas_y + canvas_height;
  
  // Clear background (draw a rectangle)
  draw_list->AddRectFilled(
    ImVec2(canvas_x, canvas_y),
    ImVec2(canvas_x + canvas_width, canvas_y + canvas_height),
    IM_COL32(26, 26, 38, 255)
  );
  
  // Render DCEL structure
  RenderDCEL(draw_list, canvas_x, canvas_y, canvas_height);
  
  // Render based on current mode
  switch (current_mode_) {
    case Mode::PointLocation:
      if (highlighted_face_ != nullptr) {
        RenderFace(draw_list, highlighted_face_, canvas_x, canvas_y, canvas_height, 
                   IM_COL32(0, 255, 0, 128));
      }
      break;
      
    case Mode::Circumcircle:
      if (show_circumcircles_) {
        for (auto* face : triangular_faces_) {
          DrawCircumcircle(draw_list, face, canvas_x, canvas_y, canvas_height);
        }
      }
      break;
      
    case Mode::LineSweep:
      // Draw line being drawn
      if (drawing_line_) {
        float x1 = canvas_x + line_start_.x;
        float y1 = canvas_y + canvas_height - line_start_.y;
        float x2 = canvas_x + line_end_.x;
        float y2 = canvas_y + canvas_height - line_end_.y;
        draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(255, 255, 0, 255), 2.0f);
      }
      // Draw completed line
      if (line_complete_) {
        float x1 = canvas_x + line_start_.x;
        float y1 = canvas_y + canvas_height - line_start_.y;
        float x2 = canvas_x + line_end_.x;
        float y2 = canvas_y + canvas_height - line_end_.y;
        draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), IM_COL32(255, 255, 0, 255), 3.0f);
      }
      // Draw swept faces
      for (auto* face : swept_faces_) {
        RenderFace(draw_list, face, canvas_x, canvas_y, canvas_height,
                   IM_COL32(255, 128, 0, 153));
      }
      break;
  }
}

void DCELTestScene::RenderUI() {
  ImGui::Text("DCEL Test Scene - Press keys to switch modes:");
  ImGui::TextColored(ImVec4(0, 1, 0, 1), "[1] Point Location - Click to find triangle");
  ImGui::TextColored(ImVec4(0, 1, 1, 1), "[2] Circumcircle - Toggle circumcircle display");
  ImGui::TextColored(ImVec4(1, 1, 0, 1), "[3] Line Sweep - Click twice to draw line");
  ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "[R] Regenerate triangulation");
  
  ImGui::Separator();
  
  std::string mode_str = "Current Mode: ";
  switch (current_mode_) {
    case Mode::PointLocation:
      mode_str += "Point Location";
      break;
    case Mode::Circumcircle:
      mode_str += "Circumcircle";
      break;
    case Mode::LineSweep:
      mode_str += "Line Sweep";
      break;
  }
  ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", mode_str.c_str());
  
  ImGui::Text("Triangles: %zu, Vertices: %zu", 
              triangular_faces_.size(), dcel_->GetVertexCount());
}

// Initialization

void DCELTestScene::InitializeDCEL() {
  dcel_ = std::make_unique<DCEL>();
  GenerateRandomTriangulation();
}

void DCELTestScene::GenerateRandomTriangulation() {
  // Generate random points
  std::vector<Point2D> points;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dis_x(point_margin_, canvas_max_x_ - point_margin_);
  std::uniform_real_distribution<double> dis_y(point_margin_, canvas_max_y_ - point_margin_);
  
  for (int i = 0; i < num_random_points_; ++i) {
    points.emplace_back(dis_x(gen), dis_y(gen));
  }
  
  // Simple triangulation: fan from first point
  std::vector<std::array<int, 3>> triangles;
  
  if (points.size() >= 3) {
    // Sort points by x-coordinate for better triangulation
    std::vector<size_t> indices(points.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&points](size_t a, size_t b) {
      return points[a].x < points[b].x;
    });
    
    // Create triangles using a simple fan approach
    for (size_t i = 0; i + 2 < indices.size(); ++i) {
      triangles.push_back({{static_cast<int>(indices[0]), static_cast<int>(indices[i + 1]), static_cast<int>(indices[i + 2])}});
    }
  }
  
  // Build DCEL from triangulation
  triangular_faces_ = DCELBuilder::BuildTriangulation(points, triangles, dcel_.get());
}

// Rendering helpers

void DCELTestScene::RenderDCEL(ImDrawList* draw_list, float canvas_x, float canvas_y, float canvas_height) {
  // Render all faces with light color
  for (auto* face : triangular_faces_) {
    RenderFace(draw_list, face, canvas_x, canvas_y, canvas_height, IM_COL32(77, 102, 128, 77));
  }
  
  // Render all edges
  for (size_t i = 0; i < dcel_->GetHalfEdgeCount(); ++i) {
    auto* edge = dcel_->GetHalfEdge(i);
    if (edge != nullptr && edge->GetOrigin() != nullptr && edge->GetTwin() != nullptr) {
      auto* dest = edge->GetDestination();
      if (dest != nullptr) {
        // Only render each edge once (use ID to avoid duplicates)
        if (edge->GetId() < edge->GetTwin()->GetId()) {
          RenderHalfEdge(draw_list, edge, canvas_x, canvas_y, canvas_height, IM_COL32(204, 204, 204, 255));
        }
      }
    }
  }
  
  // Render all vertices
  for (size_t i = 0; i < dcel_->GetVertexCount(); ++i) {
    auto* vertex = dcel_->GetVertex(i);
    if (vertex != nullptr) {
      RenderVertex(draw_list, vertex, canvas_x, canvas_y, canvas_height, IM_COL32(255, 0, 0, 255));
    }
  }
}

void DCELTestScene::RenderFace(ImDrawList* draw_list, Face* face, 
                               float canvas_x, float canvas_y, float canvas_height, ImU32 color) {
  if (face == nullptr) {
    return;
  }
  
  auto vertices = face->GetOuterBoundaryVertices();
  
  if (vertices.size() < 3) {
    return;
  }
  
  // Draw filled polygon
  std::vector<ImVec2> points;
  for (auto* vertex : vertices) {
    float x = canvas_x + vertex->GetCoordinates().x;
    float y = canvas_y + canvas_height - vertex->GetCoordinates().y;
    points.push_back(ImVec2(x, y));
  }
  
  draw_list->AddConvexPolyFilled(points.data(), points.size(), color);
}

void DCELTestScene::RenderVertex(ImDrawList* draw_list, Vertex* vertex,
                                 float canvas_x, float canvas_y, float canvas_height, ImU32 color) {
  const auto& pos = vertex->GetCoordinates();
  float x = canvas_x + pos.x;
  float y = canvas_y + canvas_height - pos.y;
  draw_list->AddCircleFilled(ImVec2(x, y), 6.0f, color);
}

void DCELTestScene::RenderHalfEdge(ImDrawList* draw_list, HalfEdge* edge,
                                   float canvas_x, float canvas_y, float canvas_height, ImU32 color) {
  auto* origin = edge->GetOrigin();
  auto* dest = edge->GetDestination();
  
  if (origin != nullptr && dest != nullptr) {
    const auto& p1 = origin->GetCoordinates();
    const auto& p2 = dest->GetCoordinates();
    float x1 = canvas_x + p1.x;
    float y1 = canvas_y + canvas_height - p1.y;
    float x2 = canvas_x + p2.x;
    float y2 = canvas_y + canvas_height - p2.y;
    draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, 1.5f);
  }
}

void DCELTestScene::RenderCircle(ImDrawList* draw_list, const Point2D& center, double radius,
                                 float canvas_x, float canvas_y, float canvas_height, ImU32 color) {
  // Draw circle as polygon approximation
  const int num_segments = 32;
  std::vector<ImVec2> points;
  
  for (int i = 0; i <= num_segments; ++i) {
    double angle = 2.0 * M_PI * i / num_segments;
    float x = canvas_x + center.x + radius * std::cos(angle);
    float y = canvas_y + canvas_height - (center.y + radius * std::sin(angle));
    points.push_back(ImVec2(x, y));
  }
  
  draw_list->AddPolyline(points.data(), points.size(), color, ImDrawFlags_None, 2.0f);
}

// Feature 1: Point Location

void DCELTestScene::HandlePointLocation(const Point2D& point) {
  highlighted_face_ = FindFaceContainingPoint(point);
}

Face* DCELTestScene::FindFaceContainingPoint(const Point2D& point) {
  for (auto* face : triangular_faces_) {
    auto vertices = face->GetOuterBoundaryVertices();
    if (PointInTriangle(point, vertices)) {
      return face;
    }
  }
  return nullptr;
}

bool DCELTestScene::PointInTriangle(const Point2D& point, const std::vector<Vertex*>& vertices) {
  if (vertices.size() != 3) {
    return false;
  }
  
  const auto& p0 = vertices[0]->GetCoordinates();
  const auto& p1 = vertices[1]->GetCoordinates();
  const auto& p2 = vertices[2]->GetCoordinates();
  
  // Barycentric coordinate method
  double denominator = ((p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y));
  
  double a = ((p1.y - p2.y) * (point.x - p2.x) + (p2.x - p1.x) * (point.y - p2.y)) / denominator;
  double b = ((p2.y - p0.y) * (point.x - p2.x) + (p0.x - p2.x) * (point.y - p2.y)) / denominator;
  double c = 1 - a - b;
  
  return (a >= 0 && a <= 1) && (b >= 0 && b <= 1) && (c >= 0 && c <= 1);
}

// Feature 2: Circumcircle

void DCELTestScene::DrawCircumcircle(ImDrawList* draw_list, Face* face,
                                     float canvas_x, float canvas_y, float canvas_height) {
  auto vertices = face->GetOuterBoundaryVertices();
  if (vertices.size() != 3) {
    return;
  }
  
  Point2D center = ComputeCircumcenter(vertices);
  double radius = ComputeCircumradius(center, vertices);
  
  RenderCircle(draw_list, center, radius, canvas_x, canvas_y, canvas_height, IM_COL32(0, 255, 255, 204));
  
  // Draw center point
  float cx = canvas_x + center.x;
  float cy = canvas_y + canvas_height - center.y;
  draw_list->AddCircleFilled(ImVec2(cx, cy), 5.0f, IM_COL32(255, 0, 255, 255));
}

Point2D DCELTestScene::ComputeCircumcenter(const std::vector<Vertex*>& vertices) {
  if (vertices.size() != 3) {
    return Point2D(0, 0);
  }
  
  const auto& p0 = vertices[0]->GetCoordinates();
  const auto& p1 = vertices[1]->GetCoordinates();
  const auto& p2 = vertices[2]->GetCoordinates();
  
  double ax = p0.x, ay = p0.y;
  double bx = p1.x, by = p1.y;
  double cx = p2.x, cy = p2.y;
  
  double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
  
  if (std::abs(d) < 1e-10) {
    // Degenerate case, return centroid
    return Point2D((ax + bx + cx) / 3.0, (ay + by + cy) / 3.0);
  }
  
  double ux = ((ax * ax + ay * ay) * (by - cy) + 
               (bx * bx + by * by) * (cy - ay) + 
               (cx * cx + cy * cy) * (ay - by)) / d;
  
  double uy = ((ax * ax + ay * ay) * (cx - bx) + 
               (bx * bx + by * by) * (ax - cx) + 
               (cx * cx + cy * cy) * (bx - ax)) / d;
  
  return Point2D(ux, uy);
}

double DCELTestScene::ComputeCircumradius(const Point2D& center, const std::vector<Vertex*>& vertices) {
  if (vertices.empty()) {
    return 0.0;
  }
  
  const auto& p0 = vertices[0]->GetCoordinates();
  return std::sqrt((p0.x - center.x) * (p0.x - center.x) + 
                   (p0.y - center.y) * (p0.y - center.y));
}

// Feature 3: Line Sweep

void DCELTestScene::HandleLineSweep() {
  if (!line_complete_) {
    return;
  }
  
  Edge2D line(line_start_, line_end_);
  swept_faces_ = FindIntersectedFaces(line);
}

std::vector<Face*> DCELTestScene::FindIntersectedFaces(const Edge2D& line) {
  std::vector<Face*> intersected;
  
  for (auto* face : triangular_faces_) {
    auto vertices = face->GetOuterBoundaryVertices();
    if (LineIntersectsTriangle(line, vertices)) {
      intersected.push_back(face);
    }
  }
  
  return intersected;
}

bool DCELTestScene::LineIntersectsTriangle(const Edge2D& line, const std::vector<Vertex*>& vertices) {
  if (vertices.size() != 3) {
    return false;
  }
  
  const auto& p0 = vertices[0]->GetCoordinates();
  const auto& p1 = vertices[1]->GetCoordinates();
  const auto& p2 = vertices[2]->GetCoordinates();
  
  // Check if line intersects any edge of the triangle
  bool intersects_edge1 = LineIntersectsSegment(line.p1, line.p2, p0, p1);
  bool intersects_edge2 = LineIntersectsSegment(line.p1, line.p2, p1, p2);
  bool intersects_edge3 = LineIntersectsSegment(line.p1, line.p2, p2, p0);
  
  // Also check if line is completely inside triangle
  bool start_inside = PointInTriangle(line.p1, vertices);
  bool end_inside = PointInTriangle(line.p2, vertices);
  
  return intersects_edge1 || intersects_edge2 || intersects_edge3 || (start_inside && end_inside);
}

bool DCELTestScene::LineIntersectsSegment(const Point2D& line_start, const Point2D& line_end,
                                          const Point2D& seg_start, const Point2D& seg_end) {
  // Line-line intersection test using cross products
  auto cross = [](const Point2D& a, const Point2D& b, const Point2D& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  };
  
  double d1 = cross(seg_start, seg_end, line_start);
  double d2 = cross(seg_start, seg_end, line_end);
  double d3 = cross(line_start, line_end, seg_start);
  double d4 = cross(line_start, line_end, seg_end);
  
  if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
      ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))) {
    return true;
  }
  
  return false;
}

}  // namespace geometry
