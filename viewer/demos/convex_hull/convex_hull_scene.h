#ifndef VIEWER_DEMOS_CONVEX_HULL_CONVEX_HULL_SCENE_H_
#define VIEWER_DEMOS_CONVEX_HULL_CONVEX_HULL_SCENE_H_

#include "../scene_base.h"
#include "../../../src/core/point2d.h"
#include "../../../src/convex_hull/convex_hull_factory.h"
#include "../../../src/convex_hull/convex_hull.h"
#include <vector>

namespace geometry {

// Enhanced convex hull scene with algorithm selection
class ConvexHullScene : public GeometryScene {
 public:
  ConvexHullScene() : current_algorithm_(ConvexHullAlgorithm::JarvisMarch) {}
  
  std::string Name() const override {
    return "Convex Hull Demo";
  }
  
  std::string Description() const override {
    return "Compare different convex hull algorithms. Click to add points.";
  }
  
  void Reset() override;
  bool OnMouseClicked(double x, double y) override;
  void Render(float canvas_x, float canvas_y, 
             float canvas_width, float canvas_height) override;
  void RenderUI() override;
  
  // Set the algorithm to use
  void SetAlgorithm(ConvexHullAlgorithm algorithm) {
    current_algorithm_ = algorithm;
    RebuildHull();
  }
  
 private:
  std::vector<Point2D> points_;
  ConvexHull hull_;
  ConvexHullAlgorithm current_algorithm_;
  bool initialized_ = false;
  
  // Cache for algorithm names (to avoid flickering)
  std::vector<std::string> algorithm_names_cache_;
  std::vector<ConvexHullAlgorithm> algorithms_cache_;
  
  void RebuildHull();
  void UpdateAlgorithmCache();
};

}  // namespace geometry

#endif  // VIEWER_DEMOS_CONVEX_HULL_CONVEX_HULL_SCENE_H_
