#include "scene_manager.h"

namespace geometry {

SceneManager::SceneManager() {
  // Register all scenes here
  scenes_.push_back(std::make_unique<ToLeftTestScene>());
  scenes_.push_back(std::make_unique<IncrementalConstructionScene>());
  
  // Set first scene as active
  if (!scenes_.empty()) {
    scenes_[0]->SetActive(true);
  }
}

}  // namespace geometry
