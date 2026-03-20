#include "scene_manager.h"
#include "demos/voronoi/voronoi_scene.h"
#include "demos/dcel/dcel_test_scene.h"
#include "demos/spatial_index/bsptree_scene.h"
#include "demos/spatial_index/bvh_scene.h"

namespace geometry {

SceneManager::SceneManager() {
  // Register all scenes here
  scenes_.push_back(std::make_unique<ToLeftTestScene>());
  scenes_.push_back(std::make_unique<IncrementalConstructionScene>());
  scenes_.push_back(std::make_unique<ConvexHullScene>());
  scenes_.push_back(std::make_unique<LineSegmentIntersectionScene>());
  scenes_.push_back(std::make_unique<ConvexPolygonIntersectionScene>());
  scenes_.push_back(std::make_unique<TriangulationScene>());
  scenes_.push_back(std::make_unique<VoronoiScene>());
  scenes_.push_back(std::make_unique<DCELTestScene>());
  scenes_.push_back(std::make_unique<QuadtreeScene>());
  scenes_.push_back(std::make_unique<KDTreeScene>());
  scenes_.push_back(std::make_unique<BSPTreeScene>());
  scenes_.push_back(std::make_unique<BVHScene>());
  
  // Set first scene as active
  if (!scenes_.empty()) {
    scenes_[0]->SetActive(true);
  }
}

}  // namespace geometry
