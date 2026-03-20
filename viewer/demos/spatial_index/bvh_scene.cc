#include "bvh_scene.h"
#include <imgui.h>
#include <chrono>
#include <algorithm>
#include <random>

namespace geometry {

BVHScene::BVHScene()
    : mode_(BVHDemoMode::InsertPoints),
      show_query_(false),
      show_points_(true),
      show_tree_(true),
      show_query_results_(true),
      show_bounds_(true),
      is_dragging_(false),
      k_neighbors_(5),
      last_query_count_(0),
      last_query_time_ms_(0.0),
      canvas_width_(800),
      canvas_height_(600),
      bounds_initialized_(false),
      capacity_(4) {
  // Don't initialize bvh tree yet - wait for canvas size
}

void BVHScene::Reset() {
  points_.clear();
  query_results_.clear();
  show_query_ = false;
  is_dragging_ = false;
  query_radius_ = 0.0;
  bounds_initialized_ = false;
  bvh_tree_.reset();
}

void BVHScene::InitializeBounds(float canvas_width, float canvas_height) {
  canvas_width_ = canvas_width;
  canvas_height_ = canvas_height;
  bounds_initialized_ = true;
}

void BVHScene::InitializeBVHTree() {
  if (!bvh_tree_) {
    BoundingBox bounds(0, 0, canvas_width_, canvas_height_);
    bvh_tree_ = std::make_unique<BVHTree>(bounds, capacity_);
  }
}

void BVHScene::RebuildBVHTree() {
  if (!points_.empty()) {
    BoundingBox bounds(0, 0, canvas_width_, canvas_height_);
    bvh_tree_ = std::make_unique<BVHTree>(bounds, capacity_);
    bvh_tree_->Build(points_);
  } else {
    bvh_tree_.reset();
  }
}

bool BVHScene::OnMouseClicked(double x, double y) {
  // Ensure bvh tree is initialized
  if (!bvh_tree_) {
    InitializeBVHTree();
  }
  
  Point2D point(x, y);
  
  switch (mode_) {
    case BVHDemoMode::InsertPoints:
      points_.push_back(point);
      bvh_tree_->Insert(point);
      break;
      
    case BVHDemoMode::NearestNeighbor:
      query_point_ = point;
      PerformNearestNeighbor(query_point_);
      break;
      
    case BVHDemoMode::KNearestNeighbors:
      query_point_ = point;
      PerformKNearestNeighbors(query_point_, k_neighbors_);
      break;
      
    case BVHDemoMode::RangeQuery:
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
      
    case BVHDemoMode::RadiusQuery:
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

void BVHScene::UpdateMousePosition(double x, double y) {
  current_mouse_pos_ = Point2D(x, y);
}

void BVHScene::PerformNearestNeighbor(const Point2D& query) {
  if (!bvh_tree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  Point2D nearest;
  if (bvh_tree_->NearestNeighbor(query, nearest)) {
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

void BVHScene::PerformKNearestNeighbors(const Point2D& query, int k) {
  if (!bvh_tree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  query_results_ = bvh_tree_->KNearestNeighbors(query, k);
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  query_point_ = query;
  show_query_ = true;
}

void BVHScene::PerformRangeQuery(const BoundingBox& range) {
  if (!bvh_tree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  query_results_ = bvh_tree_->RangeQuery(range);
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  show_query_ = true;
}

void BVHScene::PerformRadiusQuery(const Point2D& center, double radius) {
  if (!bvh_tree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  query_results_ = bvh_tree_->RadiusQuery(center, radius);
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  query_point_ = center;
  query_radius_ = radius;
  show_query_ = true;
}

void BVHScene::GenerateRandomPoints(int count) {
  // Ensure bvh tree is initialized
  if (!bvh_tree_) {
    InitializeBVHTree();
  }
  
  // Use current canvas size or default
  double max_x = bounds_initialized_ ? canvas_width_ : 800.0;
  double max_y = bounds_initialized_ ? canvas_height_ : 600.0;
  
  // Add some padding to keep points away from edges
  double padding = 20.0;
  double min_x = padding;
  double min_y = padding;
  max_x -= padding;
  max_y -= padding;
  
  // Random number generation
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dis_x(min_x, max_x);
  std::uniform_real_distribution<double> dis_y(min_y, max_y);
  
  // Generate and insert points
  for (int i = 0; i < count; ++i) {
    double x = dis_x(gen);
    double y = dis_y(gen);
    Point2D point(x, y);
    
    points_.push_back(point);
    if (bvh_tree_) {
      bvh_tree_->Insert(point);
    }
  }
}

void BVHScene::Render(float canvas_x, float canvas_y,
                     float canvas_width, float canvas_height) {
  // Initialize bounds if needed
  if (!bounds_initialized_) {
    InitializeBounds(canvas_width, canvas_height);
  }
  
  auto draw_list = ImGui::GetWindowDrawList();
  ImVec2 canvas_p0(canvas_x, canvas_y);
  ImVec2 canvas_p1(canvas_x + canvas_width, canvas_y + canvas_height);
  
  // Draw canvas background
  draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(30, 30, 35, 255));
  
  // Draw BVH tree structure
  if (bvh_tree_ && show_tree_) {
    RenderBVHTree(bvh_tree_->GetRoot(), canvas_x, canvas_y, canvas_height, 0);
  }
  
  // Draw all points
  if (show_points_) {
    RenderPoints(canvas_x, canvas_y, canvas_height);
  }
  
  // Draw query visualization
  if (show_query_) {
    RenderQuery(canvas_x, canvas_y, canvas_height);
  }
  
  // Draw current drag operation
  if (is_dragging_ && (mode_ == BVHDemoMode::RangeQuery || 
                       mode_ == BVHDemoMode::RadiusQuery)) {
    ImVec2 start(drag_start_.x + canvas_x, canvas_y + canvas_height - drag_start_.y);
    ImVec2 end(current_mouse_pos_.x + canvas_x, canvas_y + canvas_height - current_mouse_pos_.y);
    
    if (mode_ == BVHDemoMode::RangeQuery) {
      draw_list->AddRect(start, end, IM_COL32(255, 255, 0, 150), 0.0f, 0, 2.0f);
    } else if (mode_ == BVHDemoMode::RadiusQuery) {
      double radius = drag_start_.DistanceTo(current_mouse_pos_);
      draw_list->AddCircle(start, radius, IM_COL32(0, 150, 255, 150), 32, 2.0f);
    }
  }
  
  // Draw canvas border
  draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(100, 100, 100, 255));
}

void BVHScene::RenderBVHTree(const BVHNode* node,
                            float canvas_x, float canvas_y,
                            float canvas_height, int depth) {
  if (!node) return;
  
  auto draw_list = ImGui::GetWindowDrawList();
  const BoundingBox& bounds = node->GetBounds();
  
  // Calculate screen coordinates
  float x1 = bounds.min_x + canvas_x;
  float y1 = canvas_y + canvas_height - bounds.max_y;
  float x2 = bounds.max_x + canvas_x;
  float y2 = canvas_y + canvas_height - bounds.min_y;
  
  // Get depth-based color
  ImU32 color = GetDepthColor(depth, 20);
  
  // Draw bounding box
  float thickness = node->IsLeaf() ? 1.5f : 1.0f;
  draw_list->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), color, 0.0f, 0, thickness);
  
  // Recursively render children
  if (!node->IsLeaf()) {
    RenderBVHTree(node->GetLeft(), canvas_x, canvas_y, canvas_height, depth + 1);
    RenderBVHTree(node->GetRight(), canvas_x, canvas_y, canvas_height, depth + 1);
  }
}

void BVHScene::RenderPoints(float canvas_x, float canvas_y, float canvas_height) {
  auto draw_list = ImGui::GetWindowDrawList();
  
  for (const auto& pt : points_) {
    ImVec2 pos(pt.x + canvas_x, canvas_y + canvas_height - pt.y);
    
    // Check if point is in query results
    bool is_result = false;
    if (show_query_ && show_query_results_) {
      is_result = std::find(query_results_.begin(), query_results_.end(), pt) 
                  != query_results_.end();
    }
    
    // Draw point
    if (is_result) {
      draw_list->AddCircleFilled(pos, 6.0f, IM_COL32(255, 200, 50, 255));
      draw_list->AddCircle(pos, 6.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);
    } else {
      draw_list->AddCircleFilled(pos, 3.0f, IM_COL32(150, 150, 150, 255));
    }
  }
}

void BVHScene::RenderQuery(float canvas_x, float canvas_y, float canvas_height) {
  auto draw_list = ImGui::GetWindowDrawList();
  
  switch (mode_) {
    case BVHDemoMode::NearestNeighbor:
    case BVHDemoMode::KNearestNeighbors: {
      // Draw query point
      ImVec2 query_pos(query_point_.x + canvas_x, 
                       canvas_y + canvas_height - query_point_.y);
      draw_list->AddCircleFilled(query_pos, 8.0f, IM_COL32(255, 0, 0, 255));
      draw_list->AddCircle(query_pos, 8.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);
      
      // Draw lines to query results
      if (show_query_results_) {
        for (size_t i = 0; i < query_results_.size(); ++i) {
          ImVec2 result_pos(query_results_[i].x + canvas_x,
                           canvas_y + canvas_height - query_results_[i].y);
          ImU32 line_color = (i == 0) ? IM_COL32(0, 255, 0, 255) : 
                            IM_COL32(100, 200, 100, 150);
          draw_list->AddLine(query_pos, result_pos, line_color, 2.0f);
        }
      }
      break;
    }
    
    case BVHDemoMode::RangeQuery: {
      float x1 = query_range_.min_x + canvas_x;
      float y1 = canvas_y + canvas_height - query_range_.max_y;
      float x2 = query_range_.max_x + canvas_x;
      float y2 = canvas_y + canvas_height - query_range_.min_y;
      draw_list->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), 
                        IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
      break;
    }
    
    case BVHDemoMode::RadiusQuery: {
      ImVec2 center(query_point_.x + canvas_x,
                   canvas_y + canvas_height - query_point_.y);
      draw_list->AddCircle(center, query_radius_, IM_COL32(0, 150, 255, 255), 32, 2.0f);
      break;
    }
    
    default:
      break;
  }
}

ImU32 BVHScene::GetDepthColor(int depth, int max_depth) {
  float t = static_cast<float>(depth) / static_cast<float>(max_depth);
  
  // Color gradient from blue to red based on depth
  int r = static_cast<int>(100 + 155 * t);
  int g = static_cast<int>(150 * (1.0f - t));
  int b = static_cast<int>(255 * (1.0f - t));
  
  return IM_COL32(r, g, b, 200);
}

void BVHScene::RenderUI() {
  ImGui::Text("Points: %zu", points_.size());
  
  if (bvh_tree_) {
    ImGui::Text("Tree Depth: %d", bvh_tree_->GetDepth());
    ImGui::Text("Nodes: %d", bvh_tree_->GetNodeCount());
  } else {
    ImGui::Text("Tree Depth: N/A");
    ImGui::Text("Nodes: N/A");
  }
  
  ImGui::Separator();
  
  // Mode selection
  ImGui::Text("Demo Mode:");
  const char* modes[] = { "Insert Points", "Range Query", "Nearest Neighbor", 
                          "K-Nearest Neighbors", "Radius Query" };
  int current_mode = static_cast<int>(mode_);
  if (ImGui::Combo("##Mode", &current_mode, modes, 5)) {
    mode_ = static_cast<BVHDemoMode>(current_mode);
    query_results_.clear();
    show_query_ = false;
  }
  
  ImGui::Separator();
  
  // Settings
  ImGui::Text("Settings:");
  ImGui::Checkbox("Show Points", &show_points_);
  ImGui::Checkbox("Show Tree", &show_tree_);
  ImGui::Checkbox("Show Bounds", &show_bounds_);
  ImGui::Checkbox("Show Query Results", &show_query_results_);
  
  ImGui::SliderInt("Node Capacity", &capacity_, 1, 10);
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Maximum points per leaf node");
  }
  
  if (mode_ == BVHDemoMode::KNearestNeighbors) {
    ImGui::SliderInt("K Neighbors", &k_neighbors_, 1, 20);
  }
  
  ImGui::Separator();
  
  // Rebuild button
  if (ImGui::Button("Rebuild Tree")) {
    RebuildBVHTree();
  }
  
  // Clear button
  ImGui::SameLine();
  if (ImGui::Button("Clear All")) {
    Reset();
  }
  
  ImGui::Separator();
  
  // Generate random points buttons
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
  
  // Query statistics
  if (show_query_) {
    ImGui::Text("Query Results: %d", last_query_count_);
    ImGui::Text("Query Time: %.3f ms", last_query_time_ms_);
  }
  
  ImGui::Separator();
  
  // Instructions
  ImGui::Text("Instructions:");
  if (mode_ == BVHDemoMode::InsertPoints) {
    ImGui::BulletText("Left-click to add points");
  } else if (mode_ == BVHDemoMode::RangeQuery) {
    ImGui::BulletText("Click once to start dragging");
    ImGui::BulletText("Drag to define rectangle");
    ImGui::BulletText("Click again to complete query");
  } else if (mode_ == BVHDemoMode::NearestNeighbor) {
    ImGui::BulletText("Click to find nearest neighbor");
  } else if (mode_ == BVHDemoMode::KNearestNeighbors) {
    ImGui::BulletText("Click to find k nearest neighbors");
  } else if (mode_ == BVHDemoMode::RadiusQuery) {
    ImGui::BulletText("Click once to set center");
    ImGui::BulletText("Drag to define radius");
    ImGui::BulletText("Click again to complete query");
  }
  
  ImGui::Separator();
  
  // Algorithm info
  ImGui::Text("BVH Tree Properties:");
  ImGui::BulletText("Space: O(n)");
  ImGui::BulletText("Build: O(n log n) with SAH");
  ImGui::BulletText("Query: O(log n) average");
  ImGui::BulletText("Uses Surface Area Heuristic");
  ImGui::BulletText("Optimal for complex geometries");
}

}  // namespace geometry