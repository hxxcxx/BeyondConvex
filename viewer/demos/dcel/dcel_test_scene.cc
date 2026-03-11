#include "dcel_test_scene.h"
#include "../../geometry_canvas.h"
#include <random>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace viewer {

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
  // Update logic if needed
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
  auto& canvas = GeometryCanvas::GetInstance();
  
  // Update canvas bounds
  canvas_min_x_ = canvas_x;
  canvas_min_y_ = canvas_y;
  canvas_max_x_ = canvas_x + canvas_width;
  canvas_max_y_ = canvas_y + canvas_height;
  
  // Clear background
  canvas.Clear(0.1f, 0.1f, 0.15f);
  
  // Render DCEL structure
  RenderDCEL();
  
  // Render based on current mode
  switch (current_mode_) {
    case Mode::PointLocation:
      if (highlighted_face_ != nullptr) {
        RenderFace(highlighted_face_, 0.0f, 1.0f, 0.0f, 0.5f);
      }
      break;
      
    case Mode::Circumcircle:
      if (show_circumcircles_) {
        for (auto* face : triangular_faces_) {
          DrawCircumcircle(face);
        }
      }
      break;
      
    case Mode::LineSweep:
      // Draw line being drawn
      if (drawing_line_) {
        canvas.DrawLine(line_start_.x, line_start_.y, line_end_.x, line_end_.y,
                        1.0f, 1.0f, 0.0f, 2.0f);
      }
      // Draw completed line
      if (line_complete_) {
        canvas.DrawLine(line_start_.x, line_start_.y, line_end_.x, line_end_.y,
                        1.0f, 1.0f, 0.0f, 3.0f);
      }
      // Draw swept faces
      for (auto* face : swept_faces_) {
        RenderFace(face, 1.0f, 0.5f, 0.0f, 0.6f);
      }
      break;
  }
}

void DCELTestScene::RenderUI() {
  auto& canvas = GeometryCanvas::GetInstance();
  
  // Check for key presses using ImGui
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
  
  float y = 20.0f;
  float line_height = 25.0f;
  
  canvas.DrawText(10.0f, y, 0.9f, 0.9f, 0.9f, "DCEL Test Scene - Press keys to switch modes:");
  y += line_height;
  
  canvas.DrawText(10.0f, y, 0.0f, 1.0f, 0.0f, "[1] Point Location - Click to find triangle");
  y += line_height;
  
  canvas.DrawText(10.0f, y, 0.0f, 1.0f, 1.0f, "[2] Circumcircle - Toggle circumcircle display");
  y += line_height;
  
  canvas.DrawText(10.0f, y, 1.0f, 1.0f, 0.0f, "[3] Line Sweep - Click twice to draw line");
  y += line_height;
  
  canvas.DrawText(10.0f, y, 1.0f, 0.5f, 0.0f, "[R] Regenerate triangulation");
  y += line_height;
  
  y += 10.0f;
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
  canvas.DrawText(10.0f, y, 1.0f, 1.0f, 0.0f, mode_str);
  y += line_height;
  
  canvas.DrawText(10.0f, y, 0.7f, 0.7f, 0.7f, 
                  "Triangles: " + std::to_string(triangular_faces_.size()) + 
                  ", Vertices: " + std::to_string(dcel_->GetVertexCount()));
}

// Feature 1: Point Location

void DCELTestScene::HandlePointLocation(const Point2D& point) {
  highlighted_face_ = FindFaceContainingPoint(point);
}

geometry::Face* DCELTestScene::FindFaceContainingPoint(const Point2D& point) {
  for (auto* face : triangular_faces_) {
    auto vertices = face->GetOuterBoundaryVertices();
    if (PointInTriangle(point, vertices)) {
      return face;
    }
  }
  return nullptr;
}

bool DCELTestScene::PointInTriangle(const Point2D& point, const std::vector<geometry::Vertex*>& vertices) {
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

void DCELTestScene::DrawCircumcircle(geometry::Face* face) {
  auto vertices = face->GetOuterBoundaryVertices();
  if (vertices.size() != 3) {
    return;
  }
  
  Point2D center = ComputeCircumcenter(vertices);
  double radius = ComputeCircumradius(center, vertices);
  
  RenderCircle(center, radius, 0.0f, 1.0f, 1.0f, 0.8f);
  
  // Draw center point
  auto& canvas = GeometryCanvas::GetInstance();
  canvas.DrawPoint(center.x, center.y, 1.0f, 0.0f, 1.0f, 5.0f);
}

Point2D DCELTestScene::ComputeCircumcenter(const std::vector<geometry::Vertex*>& vertices) {
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

double DCELTestScene::ComputeCircumradius(const Point2D& center, const std::vector<geometry::Vertex*>& vertices) {
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

std::vector<geometry::Face*> DCELTestScene::FindIntersectedFaces(const Edge2D& line) {
  std::vector<geometry::Face*> intersected;
  
  for (auto* face : triangular_faces_) {
    auto vertices = face->GetOuterBoundaryVertices();
    if (LineIntersectsTriangle(line, vertices)) {
      intersected.push_back(face);
    }
  }
  
  return intersected;
}

bool DCELTestScene::LineIntersectsTriangle(const Edge2D& line, const std::vector<geometry::Vertex*>& vertices) {
  if (vertices.size() != 3) {
    return false;
  }
  
  const auto& p0 = vertices[0]->GetCoordinates();
  const auto& p1 = vertices[1]->GetCoordinates();
  const auto& p2 = vertices[2]->GetCoordinates();
  
  // Check if line intersects any edge of the triangle
  bool intersects_edge1 = LineIntersectsSegment(line.start, line.end, p0, p1);
  bool intersects_edge2 = LineIntersectsSegment(line.start, line.end, p1, p2);
  bool intersects_edge3 = LineIntersectsSegment(line.start, line.end, p2, p0);
  
  // Also check if line is completely inside triangle
  bool start_inside = PointInTriangle(line.start, vertices);
  bool end_inside = PointInTriangle(line.end, vertices);
  
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

void DCELTestScene::RenderUI() {
  auto& canvas = GeometryCanvas::GetInstance();
  
  float y = 20.0f;
  float line_height = 25.0f;
  
  canvas.DrawText(10.0f, y, 0.9f, 0.9f, 0.9f, "DCEL Test Scene - Press keys to switch modes:");
  y += line_height;
  
  canvas.DrawText(10.0f, y, 0.0f, 1.0f, 0.0f, "[1] Point Location - Click to find triangle");
  y += line_height;
  
  canvas.DrawText(10.0f, y, 0.0f, 1.0f, 1.0f, "[2] Circumcircle - Toggle circumcircle display");
  y += line_height;
  
  canvas.DrawText(10.0f, y, 1.0f, 1.0f, 0.0f, "[3] Line Sweep - Click twice to draw line");
  y += line_height;
  
  canvas.DrawText(10.0f, y, 1.0f, 0.5f, 0.0f, "[R] Regenerate triangulation");
  y += line_height;
  
  y += 10.0f;
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
  canvas.DrawText(10.0f, y, 1.0f, 1.0f, 0.0f, mode_str);
  y += line_height;
  
  canvas.DrawText(10.0f, y, 0.7f, 0.7f, 0.7f, 
                  "Triangles: " + std::to_string(triangular_faces_.size()) + 
                  ", Vertices: " + std::to_string(dcel_->GetVertexCount()));
}

void DCELTestScene::InitializeDCEL() {
  dcel_ = std::make_unique<geometry::DCEL>();
  GenerateRandomTriangulation();
}

void DCELTestScene::GenerateRandomTriangulation() {
  // Generate random points
  std::vector<geometry::Point2D> points;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dis_x(point_margin_, canvas_max_x_ - point_margin_);
  std::uniform_real_distribution<double> dis_y(point_margin_, canvas_max_y_ - point_margin_);
  
  for (int i = 0; i < num_random_points_; ++i) {
    points.emplace_back(dis_x(gen), dis_y(gen));
  }
  
  // Simple triangulation: fan from first point
  // This is a naive approach - for production, use Delaunay triangulation
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
      triangles.push_back({indices[0], indices[i + 1], indices[i + 2]});
    }
  }
  
  // Build DCEL from triangulation
  triangular_faces_ = geometry::DCELBuilder::BuildTriangulation(points, triangles, dcel_.get());
}

void DCELTestScene::RenderDCEL() {
  // Render all faces with light color
  for (auto* face : triangular_faces_) {
    RenderFace(face, 0.3f, 0.4f, 0.5f, 0.3f);
  }
  
  // Render all edges
  for (size_t i = 0; i < dcel_->GetHalfEdgeCount(); ++i) {
    auto* edge = dcel_->GetHalfEdge(i);
    if (edge != nullptr && edge->GetOrigin() != nullptr && edge->GetTwin() != nullptr) {
      auto* dest = edge->GetDestination();
      if (dest != nullptr) {
        // Only render each edge once (use ID to avoid duplicates)
        if (edge->GetId() < edge->GetTwin()->GetId()) {
          RenderHalfEdge(edge, 0.8f, 0.8f, 0.8f);
        }
      }
    }
  }
  
  // Render all vertices
  for (size_t i = 0; i < dcel_->GetVertexCount(); ++i) {
    auto* vertex = dcel_->GetVertex(i);
    if (vertex != nullptr) {
      RenderVertex(vertex, 1.0f, 0.0f, 0.0f);
    }
  }
}

void DCELTestScene::RenderFace(geometry::Face* face, float r, float g, float b, float a) {
  if (face == nullptr) {
    return;
  }
  
  auto& canvas = GeometryCanvas::GetInstance();
  auto vertices = face->GetOuterBoundaryVertices();
  
  if (vertices.size() < 3) {
    return;
  }
  
  // Draw filled polygon
  std::vector<float> x_coords, y_coords;
  for (auto* vertex : vertices) {
    x_coords.push_back(static_cast<float>(vertex->GetCoordinates().x));
    y_coords.push_back(static_cast<float>(vertex->GetCoordinates().y));
  }
  
  canvas.DrawFilledPolygon(x_coords, y_coords, r, g, b, a);
}

void DCELTestScene::RenderVertex(geometry::Vertex* vertex, float r, float g, float b) {
  auto& canvas = GeometryCanvas::GetInstance();
  const auto& pos = vertex->GetCoordinates();
  canvas.DrawPoint(pos.x, pos.y, r, g, b, 6.0f);
}

void DCELTestScene::RenderHalfEdge(geometry::HalfEdge* edge, float r, float g, float b) {
  auto& canvas = GeometryCanvas::GetInstance();
  auto* origin = edge->GetOrigin();
  auto* dest = edge->GetDestination();
  
  if (origin != nullptr && dest != nullptr) {
    const auto& p1 = origin->GetCoordinates();
    const auto& p2 = dest->GetCoordinates();
    canvas.DrawLine(p1.x, p1.y, p2.x, p2.y, r, g, b, 1.5f);
  }
}

void DCELTestScene::RenderCircle(const geometry::Point2D& center, double radius, float r, float g, float b, float a) {
  auto& canvas = GeometryCanvas::GetInstance();
  
  // Draw circle as polygon approximation
  const int num_segments = 32;
  std::vector<float> x_coords, y_coords;
  
  for (int i = 0; i <= num_segments; ++i) {
    double angle = 2.0 * M_PI * i / num_segments;
    x_coords.push_back(static_cast<float>(center.x + radius * std::cos(angle)));
    y_coords.push_back(static_cast<float>(center.y + radius * std::sin(angle)));
  }
  
  canvas.DrawPolygon(x_coords, y_coords, r, g, b, 2.0f);
}

// Feature 1: Point Location

void DCELTestScene::HandlePointLocation(const geometry::Point2D& point) {
  highlighted_face_ = FindFaceContainingPoint(point);
}

geometry::Face* DCELTestScene::FindFaceContainingPoint(const geometry::Point2D& point) {
  for (auto* face : triangular_faces_) {
    auto vertices = face->GetOuterBoundaryVertices();
    if (PointInTriangle(point, vertices)) {
      return face;
    }
  }
  return nullptr;
}

bool DCELTestScene::PointInTriangle(const geometry::Point2D& point, const std::vector<geometry::Vertex*>& vertices) {
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

void DCELTestScene::DrawCircumcircle(geometry::Face* face) {
  auto vertices = face->GetOuterBoundaryVertices();
  if (vertices.size() != 3) {
    return;
  }
  
  geometry::Point2D center = ComputeCircumcenter(vertices);
  double radius = ComputeCircumradius(center, vertices);
  
  RenderCircle(center, radius, 0.0f, 1.0f, 1.0f, 0.8f);
  
  // Draw center point
  auto& canvas = GeometryCanvas::GetInstance();
  canvas.DrawPoint(center.x, center.y, 1.0f, 0.0f, 1.0f, 5.0f);
}

geometry::Point2D DCELTestScene::ComputeCircumcenter(const std::vector<geometry::Vertex*>& vertices) {
  if (vertices.size() != 3) {
    return geometry::Point2D(0, 0);
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
    return geometry::Point2D((ax + bx + cx) / 3.0, (ay + by + cy) / 3.0);
  }
  
  double ux = ((ax * ax + ay * ay) * (by - cy) + 
               (bx * bx + by * by) * (cy - ay) + 
               (cx * cx + cy * cy) * (ay - by)) / d;
  
  double uy = ((ax * ax + ay * ay) * (cx - bx) + 
               (bx * bx + by * by) * (ax - cx) + 
               (cx * cx + cy * cy) * (bx - ax)) / d;
  
  return geometry::Point2D(ux, uy);
}

double DCELTestScene::ComputeCircumradius(const geometry::Point2D& center, const std::vector<geometry::Vertex*>& vertices) {
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
  
  geometry::Edge2D line(line_start_, line_end_);
  swept_faces_ = FindIntersectedFaces(line);
}

std::vector<geometry::Face*> DCELTestScene::FindIntersectedFaces(const geometry::Edge2D& line) {
  std::vector<geometry::Face*> intersected;
  
  for (auto* face : triangular_faces_) {
    auto vertices = face->GetOuterBoundaryVertices();
    if (LineIntersectsTriangle(line, vertices)) {
      intersected.push_back(face);
    }
  }
  
  return intersected;
}

bool DCELTestScene::LineIntersectsTriangle(const geometry::Edge2D& line, const std::vector<geometry::Vertex*>& vertices) {
  if (vertices.size() != 3) {
    return false;
  }
  
  const auto& p0 = vertices[0]->GetCoordinates();
  const auto& p1 = vertices[1]->GetCoordinates();
  const auto& p2 = vertices[2]->GetCoordinates();
  
  // Check if line intersects any edge of the triangle
  bool intersects_edge1 = LineIntersectsSegment(line.start, line.end, p0, p1);
  bool intersects_edge2 = LineIntersectsSegment(line.start, line.end, p1, p2);
  bool intersects_edge3 = LineIntersectsSegment(line.start, line.end, p2, p0);
  
  // Also check if line is completely inside triangle
  bool start_inside = PointInTriangle(line.start, vertices);
  bool end_inside = PointInTriangle(line.end, vertices);
  
  return intersects_edge1 || intersects_edge2 || intersects_edge3 || (start_inside && end_inside);
}

bool DCELTestScene::LineIntersectsSegment(const geometry::Point2D& line_start, const geometry::Point2D& line_end,
                                          const geometry::Point2D& seg_start, const geometry::Point2D& seg_end) {
  // Line-line intersection test using cross products
  auto cross = [](const geometry::Point2D& a, const geometry::Point2D& b, const geometry::Point2D& c) {
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

}  // namespace viewer
