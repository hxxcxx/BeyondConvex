#ifndef VIEWER_DEMO_MANAGER_H_
#define VIEWER_DEMO_MANAGER_H_

#include "demos/demo_base.h"
#include "demos/to_left_test_demo.h"
#include "demos/point_draw_demo.h"
#include <memory>
#include <vector>

namespace geometry {

// Manager for all geometry demos
class DemoManager {
 public:
  DemoManager();
  
  // Get all available demos
  const std::vector<std::unique_ptr<DemoBase>>& GetDemos() const {
    return demos_;
  }
  
  // Get current active demo
  DemoBase* GetCurrentDemo() {
    return current_demo_index_ >= 0 ? demos_[current_demo_index_].get() : nullptr;
  }
  
  // Set current demo by index
  void SetCurrentDemo(int index) {
    if (index >= 0 && index < static_cast<int>(demos_.size())) {
      if (GetCurrentDemo()) {
        GetCurrentDemo()->Reset();
      }
      current_demo_index_ = index;
      GetCurrentDemo()->SetActive(true);
    }
  }
  
  // Get current demo index
  int GetCurrentDemoIndex() const { return current_demo_index_; }
  
  // Reset current demo
  void ResetCurrentDemo() {
    if (auto* demo = GetCurrentDemo()) {
      demo->Reset();
    }
  }
  
 private:
  std::vector<std::unique_ptr<DemoBase>> demos_;
  int current_demo_index_ = 0;
};

}  // namespace geometry

#endif  // VIEWER_DEMO_MANAGER_H_
