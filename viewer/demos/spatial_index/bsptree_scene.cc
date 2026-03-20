#include "bsptree_scene.h"
#include <random>
#include <cmath>
#include <algorithm>
#include <imgui.h>

using namespace geometry;

BSPTreeScene::BSPTreeScene()
    : canvas_width_(800),
      canvas_height_(600),
      bounds_initialized_(false) {
  // Don't initialize BSP tree yet - wait for canvas size
  // Initialize display options
  show_nearest_ = true;
  show_knn_ = false;
  show_radius_query_ = false;
  show_range_query_ = false;
  show_tree_structure_ = true;
  k_value_ = 5;
  nearest_found_ = false;
  
  // Set query point
  query_point_ = Point2D(400, 300);
  
  // Set radius for radius query
  query_radius_ = 100.0f;
  
  // Set range for range query
  query_range_ = BoundingBox(300, 200, 500, 400);
}


void BSPTreeScene::InitializeBounds(float canvas_width, float canvas_height) {
  canvas_width_ = canvas_width;
  canvas_height_ = canvas_height;
  
  if (!bounds_initialized_) {
    // Create BSP tree with bounds matching canvas size
    bounds_ = BoundingBox(0, 0, canvas_width, canvas_height);
    bsp_tree_ = std::make_unique<BSPTree>(bounds_, 4);  // Capacity = 4
    
    // Generate random points
    GenerateRandomPoints(100);
    
    // Insert points into tree
    bsp_tree_->Insert(points_);
    
    // Perform queries
    PerformQueries();
    
    bounds_initialized_ = true;
  }
}

void BSPTreeScene::Update(float delta_time) {
    // Only update if tree is initialized
    if (!bsp_tree_) return;
    
    // Update query point with mouse
    if (ImGui::IsMousePosValid()) {
      auto mouse_pos = ImGui::GetMousePos();
      query_point_.x = mouse_pos.x;
      query_point_.y = mouse_pos.y;
      
      // Clamp to bounds
      query_point_.x = std::max(0.0, std::min(static_cast<double>(canvas_width_), query_point_.x));
      query_point_.y = std::max(0.0, std::min(static_cast<double>(canvas_height_), query_point_.y));
      
      // Re-perform queries
      PerformQueries();
    }
  }
  
void BSPTreeScene::Render() {
    // Only render if tree is initialized
    if (!bsp_tree_) return;
    
    auto draw_list = ImGui::GetWindowDrawList();
    
    // Draw all points
    for (const auto& pt : points_) {
      ImVec2 imgui_pt(pt.x, pt.y);
      draw_list->AddCircleFilled(imgui_pt, 3.0f, IM_COL32(150, 150, 150, 255));
    }
    
    // Draw BSP tree structure (optional)
    if (show_tree_structure_) {
      DrawBSPTree(draw_list, bsp_tree_->GetRoot(), 0);
    }
    
    // Draw query point
    ImVec2 query_pos(query_point_.x, query_point_.y);
    draw_list->AddCircleFilled(query_pos, 8.0f, IM_COL32(255, 0, 0, 255));
    draw_list->AddCircle(query_pos, 8.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);
    
    // Draw nearest neighbor
    if (nearest_found_) {
      ImVec2 nearest_pos(nearest_point_.x, nearest_point_.y);
      draw_list->AddCircleFilled(nearest_pos, 6.0f, IM_COL32(0, 255, 0, 255));
      draw_list->AddLine(query_pos, nearest_pos, IM_COL32(0, 255, 0, 255), 2.0f);
    }
    
    // Draw radius query results
    if (show_radius_query_) {
      ImVec2 center(query_point_.x, query_point_.y);
      draw_list->AddCircle(center, query_radius_, IM_COL32(0, 100, 255, 255), 32, 2.0f);
      
      for (const auto& pt : radius_results_) {
        ImVec2 pt_pos(pt.x, pt.y);
        draw_list->AddCircleFilled(pt_pos, 4.0f, IM_COL32(0, 100, 255, 255));
      }
    }
    
    // Draw range query results
    if (show_range_query_) {
      ImVec2 min(query_range_.min_x, query_range_.min_y);
      ImVec2 max(query_range_.max_x, query_range_.max_y);
      draw_list->AddRect(min, max, IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
      
      for (const auto& pt : range_results_) {
        ImVec2 pt_pos(pt.x, pt.y);
        draw_list->AddCircleFilled(pt_pos, 4.0f, IM_COL32(255, 255, 0, 255));
      }
    }
    
    // Draw k-nearest neighbors
    if (show_knn_) {
      for (size_t i = 0; i < knn_results_.size(); ++i) {
        ImVec2 pt_pos(knn_results_[i].x, knn_results_[i].y);
        ImU32 color = IM_COL32(255 - i * 50, 100 + i * 30, 150, 255);
        draw_list->AddCircleFilled(pt_pos, 5.0f, color);
        draw_list->AddCircle(pt_pos, 5.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);
      }
    }
  }
  
void BSPTreeScene::RenderUI() {
    // Tree information
    if (bsp_tree_) {
      ImGui::Text("Points: %d", bsp_tree_->GetPointCount());
      ImGui::Text("Nodes: %d", bsp_tree_->GetNodeCount());
      ImGui::Text("Depth: %d", bsp_tree_->GetDepth());
    } else {
      ImGui::Text("Points: 0");
      ImGui::Text("Nodes: 0");
      ImGui::Text("Depth: 0");
    }
    
    ImGui::Separator();
    
    // Query point
    ImGui::Text("Query Point: (%.1f, %.1f)", query_point_.x, query_point_.y);
    
    ImGui::Separator();
    
    // Display options
    ImGui::Text("Display Options:");
    ImGui::Checkbox("Show Nearest Neighbor", &show_nearest_);
    ImGui::Checkbox("Show K-NN", &show_knn_);
    ImGui::Checkbox("Show Radius Query", &show_radius_query_);
    ImGui::Checkbox("Show Range Query", &show_range_query_);
    ImGui::Checkbox("Show Tree Structure", &show_tree_structure_);
    
    ImGui::Separator();
    
    // K value slider
    if (show_knn_) {
      ImGui::SliderInt("K", &k_value_, 1, 20);
    }
    
    // Radius slider
    if (show_radius_query_) {
      ImGui::SliderFloat("Radius", &query_radius_, 10.0f, 200.0f);
    }
    
    ImGui::Separator();
    
    // Query results
    if (show_nearest_ && nearest_found_) {
      ImGui::Text("Nearest: (%.1f, %.1f)", nearest_point_.x, nearest_point_.y);
      ImGui::Text("Distance: %.2f", std::sqrt(nearest_distance_));
    }
    
    if (show_knn_) {
      ImGui::Text("K-NN Results: %zu neighbors", knn_results_.size());
    }
    
    if (show_radius_query_) {
      ImGui::Text("Radius Results: %zu points", radius_results_.size());
    }
    
    if (show_range_query_) {
      ImGui::Text("Range Results: %zu points", range_results_.size());
    }
    
    ImGui::Separator();
    
    // Controls
    if (ImGui::Button("Regenerate Points")) {
      points_.clear();
      GenerateRandomPoints(100);
      bsp_tree_->Clear();
      bsp_tree_->Insert(points_);
      PerformQueries();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Clear Tree")) {
      points_.clear();
      bsp_tree_->Clear();
      PerformQueries();
    }
    
    ImGui::Separator();
    
    // Instructions
    ImGui::TextColored(ImVec4(0.5f, 0.5, 0.5, 1.0f), 
                      "Move mouse to change query point");
  }
  
void BSPTreeScene::DrawBSPTree(ImDrawList* draw_list, const BSPNode* node, int depth) {
    if (!node) return;
    
    // Draw node bounds
    const auto& bounds = node->GetBounds();
    ImVec2 min(bounds.min_x, bounds.min_y);
    ImVec2 max(bounds.max_x, bounds.max_y);
    
    // Color based on depth
    ImU32 color = IM_COL32(100 + depth * 30, 100, 100 + depth * 20, 50);
    draw_list->AddRect(min, max, color, 0.0f, 0, 1.0f);
    
    // Draw split plane if not leaf
    if (!node->IsLeaf()) {
      const auto& plane = node->GetSplitPlane();
      
      // Draw split line
      Point2D center = bounds.Center();
      ImVec2 line_start, line_end;
      
      // Check if plane is axis-aligned
      bool is_x_aligned = std::abs(plane.normal.x) > 0.9;
      bool is_y_aligned = std::abs(plane.normal.y) > 0.9;
      
      if (is_x_aligned) {
        // Vertical line
        line_start = ImVec2(plane.point.x, bounds.min_y);
        line_end = ImVec2(plane.point.x, bounds.max_y);
      } else if (is_y_aligned) {
        // Horizontal line
        line_start = ImVec2(bounds.min_x, plane.point.y);
        line_end = ImVec2(bounds.max_x, plane.point.y);
      }
      
      draw_list->AddLine(line_start, line_end, IM_COL32(255, 100, 100, 150), 2.0f);
      
      // Recursively draw children
      DrawBSPTree(draw_list, node->GetFrontChild(), depth + 1);
      DrawBSPTree(draw_list, node->GetBackChild(), depth + 1);
    }
  }
  
void BSPTreeScene::GenerateRandomPoints(int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis_x(0, canvas_width_);
    std::uniform_real_distribution<> dis_y(0, canvas_height_);
    
    for (int i = 0; i < count; ++i) {
      points_.emplace_back(dis_x(gen), dis_y(gen));
    }
  }
  
void BSPTreeScene::PerformQueries() {
    // Only perform queries if tree is initialized
    if (!bsp_tree_) return;
    
    // Nearest neighbor
    nearest_found_ = bsp_tree_->NearestNeighbor(query_point_, nearest_point_);
    if (nearest_found_) {
      nearest_distance_ = query_point_.DistanceSquaredTo(nearest_point_);
    }
    
    // K-nearest neighbors
    if (show_knn_) {
      knn_results_ = bsp_tree_->KNearestNeighbors(query_point_, k_value_);
    }
    
    // Radius query
    if (show_radius_query_) {
      radius_results_ = bsp_tree_->RadiusQuery(query_point_, query_radius_);
    }
    
    // Range query
    if (show_range_query_) {
      range_results_ = bsp_tree_->RangeQuery(query_range_);
    }
  }

void BSPTreeScene::Reset() {
    // Only reset if tree is initialized
    if (!bsp_tree_) return;
    
    // Clear and regenerate
    points_.clear();
    bsp_tree_->Clear();
    GenerateRandomPoints(100);
    bsp_tree_->Insert(points_);
    PerformQueries();
  }

bool BSPTreeScene::OnMouseClicked(double x, double y) {
    // Only handle click if tree is initialized
    if (!bsp_tree_) return false;
    
    // Handle mouse click - add a new point at click location
    Point2D new_point(x, y);
    points_.push_back(new_point);
    bsp_tree_->Insert(new_point);
    PerformQueries();
    return true;
  }

void BSPTreeScene::Render(float canvas_x, float canvas_y, 
                          float canvas_width, float canvas_height) {
    // Initialize bounds if not done yet
    InitializeBounds(canvas_width, canvas_height);
    
    // Call the internal Render method
    Render();
  }
