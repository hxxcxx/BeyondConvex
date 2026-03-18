#include "quadtree_scene.h"
#include <imgui.h>
#include <chrono>
#include <algorithm>

namespace geometry {

QuadtreeScene::QuadtreeScene()
    : mode_(QuadtreeDemoMode::InsertPoints),
      show_query_(false),
      capacity_(4),
      auto_bounds_(true),  // Enable auto bounds by default
      show_points_(true),
      show_bounds_(true),
      show_query_results_(true),
      is_dragging_(false),
      last_query_count_(0),
      last_query_time_ms_(0.0),
      canvas_width_(800),
      canvas_height_(600),
      bounds_initialized_(false) {
  // Don't initialize quadtree yet - wait for canvas size
}

void QuadtreeScene::Reset() {
  points_.clear();
  query_results_.clear();
  show_query_ = false;
  is_dragging_ = false;
  query_radius_ = 0.0;
  bounds_initialized_ = false;
  quadtree_.reset();
}

void QuadtreeScene::InitializeBounds(float canvas_width, float canvas_height) {
  canvas_width_ = canvas_width;
  canvas_height_ = canvas_height;
  
  if (!bounds_initialized_) {
    // Create quadtree with bounds matching canvas size
    BoundingBox bounds(0, 0, canvas_width, canvas_height);
    quadtree_ = std::make_unique<Quadtree>(bounds, capacity_);
    quadtree_->SetAutoBounds(auto_bounds_);
    bounds_initialized_ = true;
  }
}

void QuadtreeScene::InitializeQuadtree() {
  // This method is called when we need to create the quadtree
  // but bounds should be initialized via InitializeBounds
  if (!bounds_initialized_) {
    // Fallback to default if InitializeBounds hasn't been called yet
    BoundingBox bounds(0, 0, canvas_width_, canvas_height_);
    quadtree_ = std::make_unique<Quadtree>(bounds, capacity_);
    quadtree_->SetAutoBounds(auto_bounds_);
    bounds_initialized_ = true;
  }
}

void QuadtreeScene::RebuildQuadtree() {
  if (!points_.empty()) {
    BoundingBox bounds = Quadtree::CalculateBounds(points_);
    quadtree_ = std::make_unique<Quadtree>(bounds, capacity_);
    quadtree_->Insert(points_);
  } else {
    InitializeQuadtree();
  }
}

bool QuadtreeScene::OnMouseClicked(double x, double y) {
  // Ensure quadtree is initialized
  if (!quadtree_) {
    InitializeQuadtree();
  }
  
  Point2D point(x, y);
  
  switch (mode_) {
    case QuadtreeDemoMode::InsertPoints:
      points_.push_back(point);
      quadtree_->Insert(point);
      break;
      
    case QuadtreeDemoMode::RangeQuery:
      if (!is_dragging_) {
        // First click - start dragging
        drag_start_ = point;
        drag_end_ = point;
        is_dragging_ = true;
        show_query_ = false;  // Hide previous query results
      } else {
        // Second click - complete the query
        drag_end_ = point;
        is_dragging_ = false;
        
        // Create range from drag points
        double min_x = std::min(drag_start_.x, drag_end_.x);
        double max_x = std::max(drag_start_.x, drag_end_.x);
        double min_y = std::min(drag_start_.y, drag_end_.y);
        double max_y = std::max(drag_start_.y, drag_end_.y);
        
        query_range_ = BoundingBox(min_x, min_y, max_x, max_y);
        PerformRangeQuery(query_range_);
      }
      break;
      
    case QuadtreeDemoMode::NearestNeighbor:
      query_point_ = point;
      PerformNearestNeighbor(query_point_);
      break;
      
    case QuadtreeDemoMode::RadiusQuery:
      if (!is_dragging_) {
        // First click - start dragging
        drag_start_ = point;
        drag_end_ = point;
        is_dragging_ = true;
        show_query_ = false;  // Hide previous query results
      } else {
        // Second click - complete the query
        drag_end_ = point;
        is_dragging_ = false;
        
        // Calculate radius from drag distance
        double radius = drag_start_.DistanceTo(drag_end_);
        query_point_ = drag_start_;
        PerformRadiusQuery(query_point_, radius);
      }
      break;
  }
  
  return true;
}

void QuadtreeScene::UpdateMousePosition(double x, double y) {
  current_mouse_pos_ = Point2D(x, y);
  
  // Update drag end position while dragging
  if (is_dragging_) {
    drag_end_ = current_mouse_pos_;
  }
}

void QuadtreeScene::PerformRangeQuery(const BoundingBox& range) {
  if (!quadtree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  query_results_ = quadtree_->RangeQuery(range);
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  show_query_ = true;
}

void QuadtreeScene::PerformNearestNeighbor(const Point2D& query) {
  if (!quadtree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  Point2D nearest;
  if (quadtree_->NearestNeighbor(query, nearest)) {
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

void QuadtreeScene::PerformRadiusQuery(const Point2D& center, double radius) {
  if (!quadtree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  query_results_ = quadtree_->RadiusQuery(center, radius);
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  query_point_ = center;
  query_radius_ = radius;  // Save the query radius
  show_query_ = true;
}

void QuadtreeScene::Render(float canvas_x, float canvas_y,
                          float canvas_width, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Initialize bounds on first render to match canvas size
  if (!bounds_initialized_) {
    InitializeBounds(canvas_width, canvas_height);
  }
  
  // Update mouse position if hovering over canvas
  ImVec2 mouse_pos = ImGui::GetMousePos();
  ImVec2 canvas_p0(canvas_x, canvas_y);
  ImVec2 canvas_p1(canvas_x + canvas_width, canvas_y + canvas_height);
  
  if (ImGui::IsMouseHoveringRect(canvas_p0, canvas_p1)) {
    double mouse_x = mouse_pos.x - canvas_x;
    double mouse_y = canvas_height - (mouse_pos.y - canvas_y);  // Flip Y
    UpdateMousePosition(mouse_x, mouse_y);
  }
  
  // Draw quadtree structure
  if (show_bounds_ && quadtree_ && quadtree_->GetRoot()) {
    RenderQuadtree(quadtree_->GetRoot(), canvas_x, canvas_y, canvas_height, 0);
  }
  
  // Draw points
  if (show_points_) {
    RenderPoints(canvas_x, canvas_y, canvas_height);
  }
  
  // Draw query visualization (completed queries)
  if (show_query_ && show_query_results_) {
    RenderQuery(canvas_x, canvas_y, canvas_height);
  }
  
  // Draw dragging visualization
  if (is_dragging_) {
    if (mode_ == QuadtreeDemoMode::RangeQuery) {
      // Draw rectangle being dragged
      float x1 = canvas_x + std::min(drag_start_.x, drag_end_.x);
      float y1 = canvas_y + canvas_height - std::max(drag_start_.y, drag_end_.y);
      float x2 = canvas_x + std::max(drag_start_.x, drag_end_.x);
      float y2 = canvas_y + canvas_height - std::min(drag_start_.y, drag_end_.y);
      
      draw_list->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), 
                        IM_COL32(255, 255, 0, 255), 0.0f, 0, 2.0f);
      
      // Draw semi-transparent fill
      draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), 
                              IM_COL32(255, 255, 0, 50));
      
      // Draw size info
      double width = std::abs(drag_end_.x - drag_start_.x);
      double height = std::abs(drag_end_.y - drag_start_.y);
      char info[64];
      snprintf(info, sizeof(info), "%.1f x %.1f", width, height);
      draw_list->AddText(ImVec2(x1, y1 - 20), IM_COL32(255, 255, 0, 255), info);
      
    } else if (mode_ == QuadtreeDemoMode::RadiusQuery) {
      // Draw circle being dragged
      double radius = drag_start_.DistanceTo(drag_end_);
      ImVec2 center(canvas_x + drag_start_.x, 
                   canvas_y + canvas_height - drag_start_.y);
      
      draw_list->AddCircle(center, static_cast<float>(radius), 
                          IM_COL32(0, 255, 0, 255), 0, 2.0f);
      
      // Draw semi-transparent fill
      draw_list->AddCircleFilled(center, static_cast<float>(radius), 
                                IM_COL32(0, 255, 0, 30));
      
      // Draw line from center to mouse
      ImVec2 edge(canvas_x + drag_end_.x, 
                 canvas_y + canvas_height - drag_end_.y);
      draw_list->AddLine(center, edge, IM_COL32(0, 255, 0, 255), 1.0f);
      
      // Draw radius info
      char info[64];
      snprintf(info, sizeof(info), "Radius: %.1f", radius);
      draw_list->AddText(ImVec2(center.x + 10, center.y - 10), 
                        IM_COL32(0, 255, 0, 255), info);
    }
  }
}

void QuadtreeScene::RenderQuadtree(const QuadTreeNode* node,
                                  float canvas_x, float canvas_y,
                                  float canvas_height, int depth) {
  if (!node) return;
  
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Get node bounds
  const BoundingBox& bounds = node->GetBounds();
  
  // Convert to screen coordinates
  float x1 = canvas_x + bounds.min_x;
  float y1 = canvas_y + canvas_height - bounds.max_y;
  float x2 = canvas_x + bounds.max_x;
  float y2 = canvas_y + canvas_height - bounds.min_y;
  
  // Draw rectangle with depth-based color
  ImU32 color = GetDepthColor(depth, quadtree_->GetDepth());
  draw_list->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), color, 0.0f, 0, 1.5f);
  
  // Recursively render children
  if (!node->IsLeaf()) {
    for (const auto& child : node->GetChildren()) {
      if (child) {
        RenderQuadtree(child.get(), canvas_x, canvas_y, canvas_height, depth + 1);
      }
    }
  }
}

void QuadtreeScene::RenderPoints(float canvas_x, float canvas_y, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  for (const auto& pt : points_) {
    ImVec2 pos(canvas_x + pt.x, canvas_y + canvas_height - pt.y);
    
    // Check if this point is in query results
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

void QuadtreeScene::RenderQuery(float canvas_x, float canvas_y, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  if (mode_ == QuadtreeDemoMode::RangeQuery && show_query_) {
    // Draw query range rectangle
    float x1 = canvas_x + query_range_.min_x;
    float y1 = canvas_y + canvas_height - query_range_.max_y;
    float x2 = canvas_x + query_range_.max_x;
    float y2 = canvas_y + canvas_height - query_range_.min_y;
    draw_list->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), 
                      IM_COL32(255, 255, 0, 255), 0.0f, 0, 3.0f);
  } else if (mode_ == QuadtreeDemoMode::NearestNeighbor && !query_results_.empty()) {
    // Draw line to nearest neighbor
    ImVec2 p1(canvas_x + query_point_.x, canvas_y + canvas_height - query_point_.y);
    ImVec2 p2(canvas_x + query_results_[0].x, 
             canvas_y + canvas_height - query_results_[0].y);
    draw_list->AddLine(p1, p2, IM_COL32(0, 255, 255, 255), 2.0f);
    draw_list->AddCircleFilled(p1, 6.0f, IM_COL32(0, 255, 255, 255));
  } else if (mode_ == QuadtreeDemoMode::RadiusQuery) {
    // Draw radius circle with actual query radius
    ImVec2 center(canvas_x + query_point_.x, canvas_y + canvas_height - query_point_.y);
    draw_list->AddCircle(center, static_cast<float>(query_radius_), IM_COL32(0, 255, 0, 255), 0, 2.0f);
    draw_list->AddCircleFilled(center, 6.0f, IM_COL32(0, 255, 0, 255));
  }
}

ImU32 QuadtreeScene::GetDepthColor(int depth, int max_depth) {
  // Generate color based on depth (gradient from blue to red)
  float t = max_depth > 0 ? static_cast<float>(depth) / max_depth : 0.0f;
  
  int r = static_cast<int>(255 * t);
  int g = static_cast<int>(255 * (1.0f - t) * 0.5f);
  int b = static_cast<int>(255 * (1.0f - t));
  
  return IM_COL32(r, g, b, 200);
}

void QuadtreeScene::RenderUI() {
  ImGui::Text("Points: %zu", points_.size());
  
  if (quadtree_) {
    ImGui::Text("Tree Depth: %d", quadtree_->GetDepth());
    ImGui::Text("Nodes: %d", quadtree_->GetNodeCount());
  } else {
    ImGui::Text("Tree Depth: N/A");
    ImGui::Text("Nodes: N/A");
  }
  
  ImGui::Separator();
  
  // Mode selection
  ImGui::Text("Demo Mode:");
  const char* modes[] = { "Insert Points", "Range Query", "Nearest Neighbor", "Radius Query" };
  int current_mode = static_cast<int>(mode_);
  if (ImGui::Combo("##Mode", &current_mode, modes, 4)) {
    mode_ = static_cast<QuadtreeDemoMode>(current_mode);
    query_results_.clear();
    show_query_ = false;
  }
  
  ImGui::Separator();
  
  // Settings
  ImGui::Text("Settings:");
  ImGui::Checkbox("Show Points", &show_points_);
  ImGui::Checkbox("Show Bounds", &show_bounds_);
  ImGui::Checkbox("Show Query Results", &show_query_results_);
  
  ImGui::SliderInt("Node Capacity", &capacity_, 1, 10);
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Maximum points per node before subdivision");
  }
  
  if (ImGui::Checkbox("Auto Bounds", &auto_bounds_)) {
    if (quadtree_) {
      quadtree_->SetAutoBounds(auto_bounds_);
    }
  }
  if (ImGui::IsItemHovered()) {
    ImGui::SetTooltip("Automatically expand bounds when needed");
  }
  
  ImGui::Separator();
  
  // Rebuild button
  if (ImGui::Button("Rebuild Tree")) {
    RebuildQuadtree();
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
  if (mode_ == QuadtreeDemoMode::InsertPoints) {
    ImGui::BulletText("Left-click to add points");
  } else if (mode_ == QuadtreeDemoMode::RangeQuery) {
    ImGui::BulletText("Click once to start dragging");
    ImGui::BulletText("Drag to define rectangle");
    ImGui::BulletText("Click again to complete query");
  } else if (mode_ == QuadtreeDemoMode::NearestNeighbor) {
    ImGui::BulletText("Click to find nearest neighbor");
  } else if (mode_ == QuadtreeDemoMode::RadiusQuery) {
    ImGui::BulletText("Click once to set center");
    ImGui::BulletText("Drag to define radius");
    ImGui::BulletText("Click again to complete query");
  }
  
  ImGui::Separator();
  
  // Algorithm info
  ImGui::Text("Quadtree Properties:");
  ImGui::BulletText("Space: O(n)");
  ImGui::BulletText("Build: O(n log n)");
  ImGui::BulletText("Query: O(log n) average");
}

}  // namespace geometry
