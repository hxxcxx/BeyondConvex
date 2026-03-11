#pragma once

#include "../scene_base.h"
#include "../../../src/dcel/dcel.h"
#include "../../../src/dcel/dcel_builder.h"
#include "../../../src/core/point2d.h"
#include "../../../src/core/edge2d.h"
#include <vector>
#include <memory>

// Forward declarations and ImGui types
#include <imgui.h>
struct ImDrawList;

namespace geometry {

/**
 * @brief DCEL Test Scene
 * 
 * Features:
 * 1. Point location: Click to find and highlight the triangle
 * 2. Circumcircle visualization: Show circumcircle of triangles
 * 3. Line sweep: Draw a line and fill intersected triangles
 */
class DCELTestScene : public GeometryScene {
 public:
  DCELTestScene();
  ~DCELTestScene() override = default;
  
  // GeometryScene interface
  std::string Name() const override;
  std::string Description() const override;
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  void Update(float delta_time) override;
  
 private:
  // Initialization
  void InitializeDCEL();
  void GenerateRandomTriangulation();
  
  // Feature 1: Point location
  void HandlePointLocation(const Point2D& point);
  Face* FindFaceContainingPoint(const Point2D& point);
  bool PointInTriangle(const Point2D& point, const std::vector<Vertex*>& vertices);
  
  // Feature 2: Circumcircle
  void DrawCircumcircle(ImDrawList* draw_list, Face* face,
                       float canvas_x, float canvas_y, float canvas_height);
  Point2D ComputeCircumcenter(const std::vector<Vertex*>& vertices);
  double ComputeCircumradius(const Point2D& center, const std::vector<Vertex*>& vertices);
  
  // Feature 3: Line sweep
  void HandleLineSweep();
  std::vector<Face*> FindIntersectedFaces(const Edge2D& line);
  bool LineIntersectsTriangle(const Edge2D& line, const std::vector<Vertex*>& vertices);
  bool LineIntersectsSegment(const Point2D& line_start, const Point2D& line_end,
                             const Point2D& seg_start, const Point2D& seg_end);
  
  // Rendering helpers
  void RenderDCEL(ImDrawList* draw_list, float canvas_x, float canvas_y, float canvas_height);
  void RenderFace(ImDrawList* draw_list, Face* face, 
                  float canvas_x, float canvas_y, float canvas_height, ImU32 color);
  void RenderVertex(ImDrawList* draw_list, Vertex* vertex,
                   float canvas_x, float canvas_y, float canvas_height, ImU32 color);
  void RenderHalfEdge(ImDrawList* draw_list, HalfEdge* edge,
                     float canvas_x, float canvas_y, float canvas_height, ImU32 color);
  void RenderCircle(ImDrawList* draw_list, const Point2D& center, double radius,
                   float canvas_x, float canvas_y, float canvas_height, ImU32 color);
  
  // DCEL data
  std::unique_ptr<DCEL> dcel_;
  std::vector<Face*> triangular_faces_;
  
  // Feature states
  enum class Mode {
    PointLocation,    // Click to find triangle
    Circumcircle,     // Show circumcircles
    LineSweep         // Draw line to sweep triangles
  };
  Mode current_mode_;
  
  // Point location state
  Face* highlighted_face_;
  
  // Circumcircle state
  bool show_circumcircles_;
  
  // Line sweep state
  Point2D line_start_;
  Point2D line_end_;
  bool drawing_line_;
  bool line_complete_;
  std::vector<Face*> swept_faces_;
  
  // Canvas bounds
  double canvas_min_x_;
  double canvas_min_y_;
  double canvas_max_x_;
  double canvas_max_y_;
  
  // Random triangulation parameters
  int num_random_points_;
  double point_margin_;
  
  // Key state tracking
  bool key_1_pressed_;
  bool key_2_pressed_;
  bool key_3_pressed_;
  bool key_r_pressed_;
};

}  // namespace geometry
