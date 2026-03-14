#ifndef VIEWER_VORONOI_SCENE_H_
#define VIEWER_VORONOI_SCENE_H_

#include "../scene_base.h"
#include "voronoi/voronoi.h"
#include <vector>

namespace geometry {

class VoronoiScene : public GeometryScene {
 public:
  VoronoiScene();
  virtual ~VoronoiScene() = default;
  
  std::string Name() const override {
    return "Voronoi Diagram";
  }
  
  std::string Description() const override {
    return "Generate Voronoi diagrams from point sites. Switch between different algorithms.";
  }
  
  void Initialize() override;
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  void Update(float delta_time) override;
  
 private:
  void GenerateVoronoi();
  void GenerateRandomSites(int count);
  void Clear();
  void InitializeAlgorithms();
  
  std::vector<Point2D> sites_;
  VoronoiDiagramResult voronoi_result_;
  
  // Algorithm selection
  std::vector<VoronoiAlgorithmType> available_algorithms_;
  int current_algorithm_index_;
  
  bool show_voronoi_;
  bool show_sites_;
  bool show_edges_;
  
  double bounds_min_x_;
  double bounds_min_y_;
  double bounds_max_x_;
  double bounds_max_y_;
  
  float site_color_[3];
  float edge_color_[3];
  float site_radius_;
};

}  // namespace geometry

#endif  // VIEWER_VORONOI_SCENE_H_
