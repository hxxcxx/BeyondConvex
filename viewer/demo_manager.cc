#include "demo_manager.h"

namespace geometry {

DemoManager::DemoManager() {
  // Register all demos here
  demos_.push_back(std::make_unique<PointDrawDemo>());
  demos_.push_back(std::make_unique<ToLeftTestDemo>());
  
  // Set first demo as active
  if (!demos_.empty()) {
    demos_[0]->SetActive(true);
  }
}

}  // namespace geometry
