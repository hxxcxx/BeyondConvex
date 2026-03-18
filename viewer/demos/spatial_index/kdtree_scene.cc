#include "kdtree_scene.h"
#include <imgui.h>
#include <chrono>
#include <algorithm>
#include <random>

namespace geometry {

KDTreeScene::KDTreeScene()
    : mode_(KDTreeDemoMode::InsertPoints),
      show_query_(false),
      show_points_(true),
      show_tree_(true),
      show_query_results_(true),
      is_dragging_(false),
      k_neighbors_(5),
      last_query_count_(0),
      last_query_time_ms_(0.0),
      canvas_width_(800),
      canvas_height_(600),
      bounds_initialized_(false) {
  // Don't initialize kdtree yet - wait for canvas size
}

void KDTreeScene::Reset() {
  points_.clear();
  query_results_.clear();
  show_query_ = false;
  is_dragging_ = false;
  query_radius_ = 0.0;
  bounds_initialized_ = false;
  kdtree_.reset();
}

void KDTreeScene::InitializeBounds(float canvas_width, float canvas_height) {
  canvas_width_ = canvas_width;
  canvas_height_ = canvas_height;
  bounds_initialized_ = true;
}

void KDTreeScene::InitializeKDTree() {
  if (!kdtree_) {
    kdtree_ = std::make_unique<KDTree>();
  }
}

void KDTreeScene::RebuildKDTree() {
  if (!points_.empty()) {
    kdtree_ = std::make_unique<KDTree>();
    kdtree_->Build(points_);
  } else {
    kdtree_.reset();
  }
}

bool KDTreeScene::OnMouseClicked(double x, double y) {
  // Ensure kdtree is initialized
  if (!kdtree_) {
    InitializeKDTree();
  }
  
  Point2D point(x, y);
  
  switch (mode_) {
    case KDTreeDemoMode::InsertPoints:
      points_.push_back(point);
      kdtree_->Insert(point);
      break;
      
    case KDTreeDemoMode::NearestNeighbor:
      query_point_ = point;
      PerformNearestNeighbor(query_point_);
      break;
      
    case KDTreeDemoMode::KNearestNeighbors:
      query_point_ = point;
      PerformKNearestNeighbors(query_point_, k_neighbors_);
      break;
      
    case KDTreeDemoMode::RangeQuery:
      if (!is_dragging_) {
        drag_start_ = point;
        drag_end_ = point;
        is_dragging_ = true;
        show_query_ = false;
      } else {
        drag_end_ = point;
        is_dragging_ = false;
        
        double min_x = std::min(drag_start_.x, drag_end_.x);
        double max_x = std::max(drag_start_.x, drag_end_.x);
        double min_y = std::min(drag_start_.y, drag_end_.y);
        double max_y = std::max(drag_start_.y, drag_end_.y);
        
        query_range_ = BoundingBox(min_x, min_y, max_x, max_y);
        PerformRangeQuery(query_range_);
      }
      break;
      
    case KDTreeDemoMode::RadiusQuery:
      if (!is_dragging_) {
        drag_start_ = point;
        drag_end_ = point;
        is_dragging_ = true;
        show_query_ = false;
      } else {
        drag_end_ = point;
        is_dragging_ = false;
        
        double radius = drag_start_.DistanceTo(drag_end_);
        query_point_ = drag_start_;
        PerformRadiusQuery(query_point_, radius);
      }
      break;
  }
  
  return true;
}

void KDTreeScene::PerformNearestNeighbor(const Point2D& query) {
  if (!kdtree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  Point2D nearest;
  if (kdtree_->NearestNeighbor(query, nearest)) {
    query_results_ = {nearest};
  } else {
    query_results_.clear();
  }
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  query_point_ = query;
  show_query_ = true;
}

void KDTreeScene::PerformKNearestNeighbors(const Point2D& query, int k) {
  if (!kdtree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  query_results_ = kdtree_->KNearestNeighbors(query, k);
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  query_point_ = query;
  show_query_ = true;
}

void KDTreeScene::PerformRangeQuery(const BoundingBox& range) {
  if (!kdtree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  query_results_ = kdtree_->RangeQuery(range);
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  show_query_ = true;
}

void KDTreeScene::PerformRadiusQuery(const Point2D& center, double radius) {
  if (!kdtree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  query_results_ = kdtree_->RadiusQuery(center, radius);
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  query_point_ = center;
  query_radius_ = radius;
  show_query_ = true;
}

void KDTreeScene::GenerateRandomPoints(int count) {
  if (!kdtree_) {
    InitializeKDTree();
  }
  
  double max_x = bounds_initialized_ ? canvas_width_ : 800.0;
  double max_y = bounds_initialized_ ? canvas_height_ : 600.0;
  
  double padding = 20.0;
  double min_x = padding;
  double min_y = padding;
  max_x -= padding;
  max_y -= padding;
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dis_x(min_x, max_x);
  std::uniform_real_distribution<double> dis_y(min_y, max_y);
  
  for (int i = 0; i < count; ++i) {
    double x = dis_x(gen);
    double y = dis_y(gen);
    Point2D point(x, y);
    
    points_.push_back(point);
    if (kdtree_) {
      kdtree_->Insert(point);
    }
  }
}

void KDTreeScene::UpdateMousePosition(double x, double y) {
  current_mouse_pos_ = Point2D(x, y);
  
  if (is_dragging_) {
    drag_end_ = current_mouse_pos_;
  }
}

void KDTreeScene::Render(float canvas_x, float canvas_y,
                        float canvas_width, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  if (!bounds_initialized_) {
    InitializeBounds(canvas_width, canvas_height);
  }
  
  // Update mouse position
  ImVec2 mouse_pos = ImGui::GetMousePos();
  ImVec2 canvas_p0(canvas_x, canvas_y);
  ImVec2 canvas_p1(canvas_x + canvas_width, canvas_y + canvas_height);
  
  if (ImGui::IsMouseHoveringRect(canvas_p0, canvas_p1)) {
    double mouse_x = mouse_pos.x - canvas_x;
    double mouse_y = canvas_height - (mouse_pos.y - canvas_y);
    UpdateMousePosition(mouse_x, mouse_y);
  }
  
  // Draw KD-tree structure
  if (show_tree_ && kdtree_ && kdtree_->GetRoot()) {
    BoundingBox full_bounds(0, 0, canvas_width, canvas_height);
    RenderKDTree(kdtree_->GetRoot(), canvas_x, canvas_y, canvas_height, 0, full_bounds);
  }
  
  // Draw points
  if (show_points_) {
    RenderPoints(canvas_x, canvas_y, canvas_height);
  }
  
  // Draw query visualization
  if (show_query_ && show_query_results_) {
    RenderQuery(canvas_x, canvas_y, canvas_height);
  }
  
  // Draw dragging visualization
  if (is_dragging_) {
    if (mode_ == KDTreeDemoMode::RangeQuery) {
      float x1 = canvas_x + std::min(drag_start_.x, drag_end_.x);
      float y1 = canvas_y + canvas_height - std::max(drag_start_.y, drag_end_.y);
      float x2 = canvas_x + std::max(drag_start_.x, drag_end_.x);
      float y2 = canvas_y + canvas_height - std::min(drag_start_.y, drag_end_.y);
      
      draw_list->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), 
                        IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
      draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), 
                              IM_COL32(255, 255, 0, 50));
    } else if (mode_ == KDTreeDemoMode::RadiusQuery) {
      double radius = drag_start_.DistanceTo(drag_end_);
      ImVec2 center(canvas_x + drag_start_.x, 
                   canvas_y + canvas_height - drag_start_.y);
      
      draw_list->AddCircle(center, static_cast<float>(radius), 
                          IM_COL32(0, 255, 0, 255), 0, 2.0f);
      draw_list->AddCircleFilled(center, static_cast<float>(radius), 
                                IM_COL32(0, 255, 0, 30));
      
      ImVec2 edge(canvas_x + drag_end_.x, 
                 canvas_y + canvas_height - drag_end_.y);
      draw_list->AddLine(center, edge, IM_COL32(0, 255, 0, 255), 1.0f);
    }
  }
}

void KDTreeScene::RenderKDTree(const KDTreeNode* node,
                              float canvas_x, float canvas_y,
                              float canvas_height, int depth,
                              const BoundingBox& bounds) {
  if (!node) return;
  
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Draw split line
  float x1, y1, x2, y2;
  const Point2D& pt = node->GetPoint();
  float screen_x = canvas_x + pt.x;
  float screen_y = canvas_y + canvas_height - pt.y;
  
  if (node->GetSplitAxis() == KDTreeNode::X_AXIS) {
    // Vertical line
    x1 = x2 = screen_x;
    y1 = canvas_y + canvas_height - bounds.min_y;
    y2 = canvas_y + canvas_height - bounds.max_y;
  } else {
    // Horizontal line
    x1 = canvas_x + bounds.min_x;
    x2 = canvas_x + bounds.max_x;
    y1 = y2 = screen_y;
  }
  
  ImU32 color = GetDepthColor(depth, kdtree_ ? kdtree_->GetDepth() : 0);
  draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), color, 1.5f);
  
  // Draw point
  draw_list->AddCircleFilled(ImVec2(screen_x, screen_y), 4.0f, IM_COL32(100, 100, 255, 255));
  
  // Recursively render children
  BoundingBox left_bounds, right_bounds;
  
  if (node->GetSplitAxis() == KDTreeNode::X_AXIS) {
    left_bounds = BoundingBox(bounds.min_x, bounds.min_y, pt.x, bounds.max_y);
    right_bounds = BoundingBox(pt.x, bounds.min_y, bounds.max_x, bounds.max_y);
  } else {
    left_bounds = BoundingBox(bounds.min_x, bounds.min_y, bounds.max_x, pt.y);
    right_bounds = BoundingBox(bounds.min_x, pt.y, bounds.max_x, bounds.max_y);
  }
  
  RenderKDTree(node->GetLeft(), canvas_x, canvas_y, canvas_height, depth + 1, left_bounds);
  RenderKDTree(node->GetRight(), canvas_x, canvas_y, canvas_height, depth + 1, right_bounds);
}

void KDTreeScene::RenderPoints(float canvas_x, float canvas_y, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  for (const auto& pt : points_) {
    ImVec2 pos(canvas_x + pt.x, canvas_y + canvas_height - pt.y);
    
    bool is_result = std::find(query_results_.begin(), query_results_.end(), pt) 
                    != query_results_.end();
    
    if (is_result) {
      draw_list->AddCircleFilled(pos, 8.0f, IM_COL32(255, 100, 100, 255));
      draw_list->AddCircle(pos, 8.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);
    } else {
      draw_list->AddCircleFilled(pos, 5.0f, IM_COL32(150, 150, 150, 255));
    }
  }
}

void KDTreeScene::RenderQuery(float canvas_x, float canvas_y, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  if (mode_ == KDTreeDemoMode::NearestNeighbor && !query_results_.empty()) {
    ImVec2 p1(canvas_x + query_point_.x, canvas_y + canvas_height - query_point_.y);
    ImVec2 p2(canvas_x + query_results_[0].x, 
             canvas_y + canvas_height - query_results_[0].y);
    draw_list->AddLine(p1, p2, IM_COL32(0, 255, 255, 255), 2.0f);
    draw_list->AddCircleFilled(p1, 6.0f, IM_COL32(0, 255, 255, 255));
  } else if (mode_ == KDTreeDemoMode::KNearestNeighbors) {
    ImVec2 center(canvas_x + query_point_.x, canvas_y + canvas_height - query_point_.y);
    draw_list->AddCircleFilled(center, 6.0f, IM_COL32(0, 255, 255, 255));
    
    // Draw lines to k nearest neighbors
    for (size_t i = 0; i < query_results_.size(); ++i) {
      ImVec2 p(canvas_x + query_results_[i].x, 
              canvas_y + canvas_height - query_results_[i].y);
      ImU32 color = IM_COL32(0, 255, 255 - static_cast<int>(i * 200 / query_results_.size()), 255);
      draw_list->AddLine(center, p, color, 2.0f);
    }
  } else if (mode_ == KDTreeDemoMode::RangeQuery) {
    float x1 = canvas_x + query_range_.min_x;
    float y1 = canvas_y + canvas_height - query_range_.max_y;
    float x2 = canvas_x + query_range_.max_x;
    float y2 = canvas_y + canvas_height - query_range_.min_y;
    draw_list->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), 
                      IM_COL32(255, 255, 0, 255), 0.0f, 0, 3.0f);
  } else if (mode_ == KDTreeDemoMode::RadiusQuery) {
    ImVec2 center(canvas_x + query_point_.x, canvas_y + canvas_height - query_point_.y);
    draw_list->AddCircle(center, static_cast<float>(query_radius_), 
                        IM_COL32(0, 255, 0, 255), 0, 2.0f);
    draw_list->AddCircleFilled(center, 6.0f, IM_COL32(0, 255, 0, 255));
  }
}

ImU32 KDTreeScene::GetDepthColor(int depth, int max_depth) {
  float t = max_depth > 0 ? static_cast<float>(depth) / max_depth : 0.0f;
  
  int r = static_cast<int>(255 * t);
  int g = static_cast<int>(255 * (1.0f - t) * 0.5f);
  int b = static_cast<int>(255 * (1.0f - t));
  
  return IM_COL32(r, g, b, 200);
}

void KDTreeScene::RenderUI() {
  ImGui::Text("Points: %zu", points_.size());
  
  if (kdtree_) {
    ImGui::Text("Tree Depth: %d", kdtree_->GetDepth());
    ImGui::Text("Nodes: %d", kdtree_->GetNodeCount());
  } else {
    ImGui::Text("Tree Depth: N/A");
    ImGui::Text("Nodes: N/A");
  }
  
  ImGui::Separator();
  
  // Mode selection
  ImGui::Text("Demo Mode:");
  const char* modes[] = { "Insert Points", "Nearest Neighbor", "K-Nearest Neighbors", 
                          "Range Query", "Radius Query" };
  int current_mode = static_cast<int>(mode_);
  if (ImGui::Combo("##Mode", &current_mode, modes, 5)) {
    mode_ = static_cast<KDTreeDemoMode>(current_mode);
    query_results_.clear();
    show_query_ = false;
  }
  
  ImGui::Separator();
  
  // Settings
  ImGui::Text("Settings:");
  ImGui::Checkbox("Show Points", &show_points_);
  ImGui::Checkbox("Show Tree", &show_tree_);
  ImGui::Checkbox("Show Query Results", &show_query_results_);
  
  if (mode_ == KDTreeDemoMode::KNearestNeighbors) {
    ImGui::SliderInt("K Value", &k_neighbors_, 1, 20);
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("Number of nearest neighbors to find");
    }
  }
  
  ImGui::Separator();
  
  // Generate random points
  ImGui::Text("Generate Random Points:");
  if (ImGui::Button("Add 10 Points")) {
    GenerateRandomPoints(10);
  }
  ImGui::SameLine();
  if (ImGui::Button("Add 50 Points")) {
    GenerateRandomPoints(50);
  }
  ImGui::SameLine();
  if (ImGui::Button("Add 100 Points")) {
    GenerateRandomPoints(100);
  }
  
  ImGui::Separator();
  
  // Rebuild button
  if (ImGui::Button("Rebuild Tree")) {
    RebuildKDTree();
  }
  
  // Clear button
  ImGui::SameLine();
  if (ImGui::Button("Clear All")) {
    Reset();
  }
  
  ImGui::Separator();
  
  // Query statistics
  if (show_query_) {
    ImGui::Text("Query Results: %d", last_query_count_);
    ImGui::Text("Query Time: %.3f ms", last_query_time_ms_);
  }
  
  ImGui::Separator();
  
  // Instructions
  ImGui::Text("Instructions:");
  if (mode_ == KDTreeDemoMode::InsertPoints) {
    ImGui::BulletText("Left-click to add points");
  } else if (mode_ == KDTreeDemoMode::NearestNeighbor) {
    ImGui::BulletText("Click to find nearest neighbor");
  } else if (mode_ == KDTreeDemoMode::KNearestNeighbors) {
    ImGui::BulletText("Click to find K nearest neighbors");
    ImGui::BulletText("Adjust K value with slider");
  } else if (mode_ == KDTreeDemoMode::RangeQuery) {
    ImGui::BulletText("Click once to start dragging");
    ImGui::BulletText("Drag to define rectangle");
    ImGui::BulletText("Click again to complete query");
  } else if (mode_ == KDTreeDemoMode::RadiusQuery) {
    ImGui::BulletText("Click once to set center");
    ImGui::BulletText("Drag to define radius");
    ImGui::BulletText("Click again to complete query");
  }
  
  ImGui::Separator();
  
  // Algorithm info
  ImGui::Text("KD-Tree Properties:");
  ImGui::BulletText("Space: O(n)");
  ImGui::BulletText("Build: O(n log n)");
  ImGui::BulletText("NN Search: O(log n) average");
  ImGui::BulletText("Best for: Nearest neighbor queries");
}

}  // namespace geometry
