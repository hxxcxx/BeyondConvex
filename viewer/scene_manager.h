#ifndef VIEWER_SCENE_MANAGER_H_
#define VIEWER_SCENE_MANAGER_H_

#include "demos/scene_base.h"
#include "demos/convex_hull/to_left_test_scene.h"
#include "demos/convex_hull/incremental_construction_scene.h"
#include "demos/convex_hull/convex_hull_scene.h"
#include "demos/intersection/line_segment_intersection_scene.h"
#include "demos/intersection/convex_polygon_intersection_scene.h"
#include "demos/triangulation/triangulation_scene.h"
#include <memory>
#include <vector>

namespace geometry {

// Manager for all geometry scenes
class SceneManager {
 public:
  SceneManager();
  
  // Get all available scenes
  const std::vector<std::unique_ptr<GeometryScene>>& GetScenes() const {
    return scenes_;
  }
  
  // Get current active scene
  GeometryScene* GetCurrentScene() {
    return current_scene_index_ >= 0 ? scenes_[current_scene_index_].get() : nullptr;
  }
  
  // Set current scene by index
  void SetCurrentScene(int index) {
    if (index >= 0 && index < static_cast<int>(scenes_.size())) {
      if (GetCurrentScene()) {
        GetCurrentScene()->Reset();
      }
      current_scene_index_ = index;
      GetCurrentScene()->SetActive(true);
    }
  }
  
  // Get current scene index
  int GetCurrentSceneIndex() const { return current_scene_index_; }
  
  // Reset current scene
  void ResetCurrentScene() {
    if (auto* scene = GetCurrentScene()) {
      scene->Reset();
    }
  }
  
 private:
  std::vector<std::unique_ptr<GeometryScene>> scenes_;
  int current_scene_index_ = 0;
};

}  // namespace geometry

#endif  // VIEWER_SCENE_MANAGER_H_
