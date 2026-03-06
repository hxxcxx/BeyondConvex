#ifndef VIEWER_TRIANGULATION_SCENE_H_
#define VIEWER_TRIANGULATION_SCENE_H_

#include "../scene_base.h"
#include "../../../src/triangulation/triangulation.h"
#include <vector>

namespace geometry {

class TriangulationScene : public GeometryScene {
 public:
  TriangulationScene();
  virtual ~TriangulationScene() = default;
  
  std::string Name() const override {
    return "Triangulation";
  }
  
  std::string Description() const override {
    return "Triangulate polygons using different algorithms.";
  }
  
  void Initialize() override;
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  void Update(float delta_time) override;
  
 private:
  void GenerateRandomPolygon();
  void ComputeTriangulation();
  void SwitchAlgorithm();
  
  // Helper functions for rendering
  void DrawPolygon(const std::vector<Point2D>& vertices, float r, float g, float b, float alpha);
  void DrawFilledPolygon(const std::vector<Point2D>& vertices, float r, float g, float b, float alpha);
  void DrawTriangle(const Triangle& tri, float r, float g, float b, float alpha);
  void DrawLine(const Point2D& p1, const Point2D& p2, float width, float r, float g, float b);
  void DrawPoint(const Point2D& p, float size, float r, float g, float b);
  
  // Polygon vertices
  std::vector<Point2D> polygon_points_;
  
  // Triangulation result
  TriangulationResult triangulation_result_;
  
  // Algorithm selection
  int selected_algorithm_;
  std::vector<TriangulationAlgorithm> algorithms_;
  
  // Canvas parameters
  float canvas_x_, canvas_y_, canvas_width_, canvas_height_;

  // Visualization parameters
  float polygon_color_[3];
  float triangle_colors_[3][3];  // Colors for different triangles

  // Show triangulation result
  bool show_triangulation_;
};

}  // namespace geometry

#endif  // VIEWER_TRIANGULATION_SCENE_H_
