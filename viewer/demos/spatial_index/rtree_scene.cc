#include "rtree_scene.h"
#include <imgui.h>
#include <chrono>
#include <algorithm>
#include <random>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace geometry {

RTreeScene::RTreeScene()
    : mode_(RTreeDemoMode::InsertRectangles),
      is_creating_rect_(false),
      show_query_(false),
      show_rectangles_(true),
      show_tree_(true),
      show_query_results_(true),
      show_mbrs_(true),
      max_entries_(4),
      min_entries_(2),
      last_query_count_(0),
      last_query_time_ms_(0.0),
      canvas_width_(800),
      canvas_height_(600) {
  rtree_ = std::make_unique<RTree>(max_entries_, min_entries_);
}

void RTreeScene::Reset() {
  rectangles_.clear();
  query_results_.clear();
  show_query_ = false;
  is_creating_rect_ = false;
  rtree_ = std::make_unique<RTree>(max_entries_, min_entries_);
}

void RTreeScene::RebuildTree() {
  rtree_ = std::make_unique<RTree>(max_entries_, min_entries_);
  rtree_->Insert(rectangles_);
}

bool RTreeScene::OnMouseClicked(double x, double y) {
  Point2D point(x, y);
  
  switch (mode_) {
    case RTreeDemoMode::InsertRectangles:
      if (!is_creating_rect_) {
        // Start creating rectangle
        rect_start_ = point;
        rect_end_ = point;
        is_creating_rect_ = true;
      } else {
        // Finish creating rectangle
        rect_end_ = point;
        
        // Create valid rectangle (ensure min < max)
        double min_x = std::min(rect_start_.x, rect_end_.x);
        double max_x = std::max(rect_start_.x, rect_end_.x);
        double min_y = std::min(rect_start_.y, rect_end_.y);
        double max_y = std::max(rect_start_.y, rect_end_.y);
        
        // Only add if rectangle has some size
        if (max_x - min_x > 1.0 && max_y - min_y > 1.0) {
          BoundingBox bounds(min_x, min_y, max_x, max_y);
          int data_id = static_cast<int>(rectangles_.size());
          rectangles_.push_back({bounds, data_id});
          rtree_->Insert(bounds, data_id);
        }
        
        is_creating_rect_ = false;
      }
      break;
      
    case RTreeDemoMode::RangeQuery:
      if (!is_creating_rect_) {
        // Start dragging for range query
        rect_start_ = point;
        rect_end_ = point;
        is_creating_rect_ = true;
        show_query_ = false;
      } else {
        // Complete range query
        rect_end_ = point;
        is_creating_rect_ = false;
        
        double min_x = std::min(rect_start_.x, rect_end_.x);
        double max_x = std::max(rect_start_.x, rect_end_.x);
        double min_y = std::min(rect_start_.y, rect_end_.y);
        double max_y = std::max(rect_start_.y, rect_end_.y);
        
        query_range_ = BoundingBox(min_x, min_y, max_x, max_y);
        PerformRangeQuery(query_range_);
      }
      break;
      
    case RTreeDemoMode::PointQuery:
      query_point_ = point;
      PerformPointQuery(query_point_);
      break;
      
    case RTreeDemoMode::IntersectionQuery:
      if (!is_creating_rect_) {
        // Start dragging for intersection query
        rect_start_ = point;
        rect_end_ = point;
        is_creating_rect_ = true;
        show_query_ = false;
      } else {
        // Complete intersection query
        rect_end_ = point;
        is_creating_rect_ = false;
        
        double min_x = std::min(rect_start_.x, rect_end_.x);
        double max_x = std::max(rect_start_.x, rect_end_.x);
        double min_y = std::min(rect_start_.y, rect_end_.y);
        double max_y = std::max(rect_start_.y, rect_end_.y);
        
        query_range_ = BoundingBox(min_x, min_y, max_x, max_y);
        PerformIntersectionQuery(query_range_);
      }
      break;
  }
  
  return true;
}

void RTreeScene::UpdateMousePosition(double x, double y) {
  current_mouse_pos_ = Point2D(x, y);
  
  if (is_creating_rect_) {
    rect_end_ = current_mouse_pos_;
  }
}

void RTreeScene::PerformRangeQuery(const BoundingBox& range) {
  if (!rtree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  query_results_ = rtree_->RangeQuery(range);
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  show_query_ = true;
}

void RTreeScene::PerformPointQuery(const Point2D& point) {
  if (!rtree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  query_results_ = rtree_->PointQuery(point);
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  show_query_ = true;
}

void RTreeScene::PerformIntersectionQuery(const BoundingBox& bounds) {
  if (!rtree_) return;
  
  auto start = std::chrono::high_resolution_clock::now();
  query_results_ = rtree_->IntersectionQuery(bounds);
  auto end = std::chrono::high_resolution_clock::now();
  
  last_query_count_ = static_cast<int>(query_results_.size());
  last_query_time_ms_ = std::chrono::duration<double, std::milli>(end - start).count();
  show_query_ = true;
}

void RTreeScene::GenerateRandomRectangles(int count) {
  rectangles_.clear();
  rtree_ = std::make_unique<RTree>(max_entries_, min_entries_);
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> x_dist(50, canvas_width_ - 100);
  std::uniform_real_distribution<> y_dist(50, canvas_height_ - 100);
  std::uniform_real_distribution<> size_dist(30, 100);
  
  for (int i = 0; i < count; ++i) {
    double x = x_dist(gen);
    double y = y_dist(gen);
    double w = size_dist(gen);
    double h = size_dist(gen);
    
    BoundingBox bounds(x, y, x + w, y + h);
    rectangles_.push_back({bounds, i});
    rtree_->Insert(bounds, i);
  }
}

void RTreeScene::GenerateGridRectangles(int rows, int cols) {
  rectangles_.clear();
  rtree_ = std::make_unique<RTree>(max_entries_, min_entries_);
  
  double cell_width = canvas_width_ / cols;
  double cell_height = canvas_height_ / rows;
  
  int id = 0;
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      double x = col * cell_width + 10;
      double y = row * cell_height + 10;
      double w = cell_width - 20;
      double h = cell_height - 20;
      
      if (w > 5 && h > 5) {
        BoundingBox bounds(x, y, x + w, y + h);
        rectangles_.push_back({bounds, id++});
        rtree_->Insert(bounds, id - 1);
      }
    }
  }
}

void RTreeScene::GenerateOverlappingRectangles(int count) {
  rectangles_.clear();
  rtree_ = std::make_unique<RTree>(max_entries_, min_entries_);
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> offset_dist(-40, 40);
  std::uniform_real_distribution<> size_dist(50, 120);
  
  // Create base rectangles
  double base_x = canvas_width_ / 2 - 100;
  double base_y = canvas_height_ / 2 - 100;
  
  for (int i = 0; i < count; ++i) {
    double offset_x = offset_dist(gen);
    double offset_y = offset_dist(gen);
    double w = size_dist(gen);
    double h = size_dist(gen);
    
    double x = base_x + offset_x;
    double y = base_y + offset_y;
    
    BoundingBox bounds(x, y, x + w, y + h);
    rectangles_.push_back({bounds, i});
    rtree_->Insert(bounds, i);
  }
}

void RTreeScene::GenerateSpiralRectangles(int count) {
  rectangles_.clear();
  rtree_ = std::make_unique<RTree>(max_entries_, min_entries_);
  
  double center_x = canvas_width_ / 2;
  double center_y = canvas_height_ / 2;
  double max_radius = std::min(canvas_width_, canvas_height_) / 2 - 50;
  
  for (int i = 0; i < count; ++i) {
    double t = static_cast<double>(i) / count;
    double angle = t * 8.0 * M_PI;  // Multiple rotations
    double radius = t * max_radius;
    
    double x = center_x + radius * cos(angle) - 25;
    double y = center_y + radius * sin(angle) - 25;
    double size = 20 + t * 40;
    
    BoundingBox bounds(x, y, x + size, y + size);
    rectangles_.push_back({bounds, i});
    rtree_->Insert(bounds, i);
  }
}

void RTreeScene::GenerateConcentricRectangles(int rings, int per_ring) {
  rectangles_.clear();
  rtree_ = std::make_unique<RTree>(max_entries_, min_entries_);
  
  double center_x = canvas_width_ / 2;
  double center_y = canvas_height_ / 2;
  double max_radius = std::min(canvas_width_, canvas_height_) / 2 - 50;
  
  int id = 0;
  for (int ring = 0; ring < rings; ++ring) {
    double t = static_cast<double>(ring + 1) / rings;
    double radius = t * max_radius;
    
    for (int i = 0; i < per_ring; ++i) {
      double angle = (2.0 * M_PI * i) / per_ring;
      double size = 15 + t * 25;
      
      double x = center_x + radius * cos(angle) - size / 2;
      double y = center_y + radius * sin(angle) - size / 2;
      
      BoundingBox bounds(x, y, x + size, y + size);
      rectangles_.push_back({bounds, id++});
      rtree_->Insert(bounds, id - 1);
    }
  }
}

void RTreeScene::GenerateCityBlocks(int rows, int cols) {
  rectangles_.clear();
  rtree_ = std::make_unique<RTree>(max_entries_, min_entries_);
  
  double block_width = (canvas_width_ - 100) / cols;
  double block_height = (canvas_height_ - 100) / rows;
  double street_width = 15;
  
  int id = 0;
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      double x = 50 + col * block_width + street_width / 2;
      double y = 50 + row * block_height + street_width / 2;
      double w = block_width - street_width;
      double h = block_height - street_width;
      
      // Add building
      if (w > 10 && h > 10) {
        BoundingBox bounds(x, y, x + w, y + h);
        rectangles_.push_back({bounds, id++});
        rtree_->Insert(bounds, id - 1);
      }
    }
  }
}

void RTreeScene::GenerateRandomClusters(int clusters, int per_cluster) {
  rectangles_.clear();
  rtree_ = std::make_unique<RTree>(max_entries_, min_entries_);
  
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> cluster_x_dist(100, canvas_width_ - 100);
  std::uniform_real_distribution<> cluster_y_dist(100, canvas_height_ - 100);
  std::uniform_real_distribution<> offset_dist(-50, 50);
  std::uniform_real_distribution<> size_dist(20, 50);
  
  int id = 0;
  for (int c = 0; c < clusters; ++c) {
    double cluster_x = cluster_x_dist(gen);
    double cluster_y = cluster_y_dist(gen);
    
    for (int i = 0; i < per_cluster; ++i) {
      double offset_x = offset_dist(gen);
      double offset_y = offset_dist(gen);
      double w = size_dist(gen);
      double h = size_dist(gen);
      
      double x = cluster_x + offset_x;
      double y = cluster_y + offset_y;
      
      BoundingBox bounds(x, y, x + w, y + h);
      rectangles_.push_back({bounds, id++});
      rtree_->Insert(bounds, id - 1);
    }
  }
}

void RTreeScene::Render(float canvas_x, float canvas_y, 
                       float canvas_width, float canvas_height) {
  canvas_width_ = canvas_width;
  canvas_height_ = canvas_height;
  
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Render rectangles
  if (show_rectangles_) {
    RenderRectangles(canvas_x, canvas_y, canvas_height);
  }
  
  // Render R-tree structure
  if (show_tree_ && rtree_) {
    RenderRTree(rtree_->GetRoot(), canvas_x, canvas_y, canvas_height, 0);
  }
  
  // Render query visualization
  if (show_query_) {
    RenderQuery(canvas_x, canvas_y, canvas_height);
  }
  
  // Render rectangle being created
  if (is_creating_rect_) {
    double min_x = std::min(rect_start_.x, rect_end_.x);
    double max_x = std::max(rect_start_.x, rect_end_.x);
    double min_y = std::min(rect_start_.y, rect_end_.y);
    double max_y = std::max(rect_start_.y, rect_end_.y);
    
    BoundingBox temp_rect(min_x, min_y, max_x, max_y);
    RenderRectangle(temp_rect, IM_COL32(255, 255, 0, 255), 0.1f,
                   canvas_x, canvas_y, canvas_height);
  }
}

void RTreeScene::RenderRectangles(float canvas_x, float canvas_y, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  for (size_t i = 0; i < rectangles_.size(); ++i) {
    const auto& rect = rectangles_[i];
    
    // Check if this rectangle is in query results
    bool is_highlighted = false;
    if (show_query_ && show_query_results_) {
      for (int result_id : query_results_) {
        if (result_id == rect.second) {
          is_highlighted = true;
          break;
        }
      }
    }
    
    if (is_highlighted) {
      // Highlighted rectangles get a glowing effect
      ImU32 glow_color = IM_COL32(100, 255, 100, 100);
      RenderRectangle(rect.first, glow_color, 0.3f, canvas_x, canvas_y, canvas_height);
      RenderRectangle(rect.first, IM_COL32(150, 255, 150, 255), 0.5f, canvas_x, canvas_y, canvas_height);
      RenderRectangle(rect.first, IM_COL32(255, 255, 255, 255), 0.0f, canvas_x, canvas_y, canvas_height);
    } else {
      // Normal rectangles with beautiful colors
      ImU32 color = GetRectangleColor(static_cast<int>(i));
      RenderRectangle(rect.first, color, 0.25f, canvas_x, canvas_y, canvas_height);
    }
  }
}

void RTreeScene::RenderRTree(const RTreeNode* node,
                            float canvas_x, float canvas_y,
                            float canvas_height, int depth) {
  if (!node) return;
  
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  int max_depth = rtree_->GetDepth();
  
  // Render MBR for this node with beautiful depth-based colors
  if (show_mbrs_ && depth > 0) {
    const BoundingBox& mbr = node->GetMBR();
    if (mbr.Width() > 0 && mbr.Height() > 0) {
      ImU32 color = GetDepthColor(depth, max_depth);
      
      // Convert to screen coordinates
      float min_x = static_cast<float>(canvas_x + mbr.min_x);
      float min_y = static_cast<float>(canvas_y + canvas_height - mbr.max_y);
      float max_x = static_cast<float>(canvas_x + mbr.max_x);
      float max_y = static_cast<float>(canvas_y + canvas_height - mbr.min_y);
      
      ImVec2 p1(min_x, min_y);
      ImVec2 p2(max_x, max_y);
      
      // Fill with very transparent color
      ImU32 fill_color = (color & 0xFF000000) | 
                         (static_cast<int>(30) << 16) |
                         (static_cast<int>(30) << 8) |
                         static_cast<int>(30);
      draw_list->AddRectFilled(p1, p2, fill_color);
      
      // Draw border with thickness based on depth
      float thickness = std::max(1.0f, 3.0f - depth * 0.3f);
      draw_list->AddRect(p1, p2, color, 0.0f, 0, thickness);
      
      // Draw corner markers for better visibility
      float corner_size = 5.0f;
      draw_list->AddLine(ImVec2(min_x, min_y), ImVec2(min_x + corner_size, min_y), color, thickness);
      draw_list->AddLine(ImVec2(min_x, min_y), ImVec2(min_x, min_y + corner_size), color, thickness);
      draw_list->AddLine(ImVec2(max_x, min_y), ImVec2(max_x - corner_size, min_y), color, thickness);
      draw_list->AddLine(ImVec2(max_x, min_y), ImVec2(max_x, min_y + corner_size), color, thickness);
      draw_list->AddLine(ImVec2(min_x, max_y), ImVec2(min_x + corner_size, max_y), color, thickness);
      draw_list->AddLine(ImVec2(min_x, max_y), ImVec2(min_x, max_y - corner_size), color, thickness);
      draw_list->AddLine(ImVec2(max_x, max_y), ImVec2(max_x - corner_size, max_y), color, thickness);
      draw_list->AddLine(ImVec2(max_x, max_y), ImVec2(max_x, max_y - corner_size), color, thickness);
    }
  }
  
  // Recursively render children
  if (!node->IsLeaf()) {
    for (const auto& entry : node->GetEntries()) {
      if (entry.child) {
        RenderRTree(entry.child.get(), canvas_x, canvas_y, canvas_height, depth + 1);
      }
    }
  }
}

void RTreeScene::RenderQuery(float canvas_x, float canvas_y, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  // Render query range with animated effect
  if (query_range_.Width() > 0 && query_range_.Height() > 0) {
    float min_x = static_cast<float>(canvas_x + query_range_.min_x);
    float min_y = static_cast<float>(canvas_y + canvas_height - query_range_.max_y);
    float max_x = static_cast<float>(canvas_x + query_range_.max_x);
    float max_y = static_cast<float>(canvas_y + canvas_height - query_range_.min_y);
    
    ImVec2 p1(min_x, min_y);
    ImVec2 p2(max_x, max_y);
    
    // Semi-transparent yellow fill
    draw_list->AddRectFilled(p1, p2, IM_COL32(255, 255, 0, 30));
    
    // Bright yellow border with glow effect
    draw_list->AddRect(p1, p2, IM_COL32(255, 255, 0, 150), 0.0f, 0, 3.0f);
    draw_list->AddRect(p1, p2, IM_COL32(255, 255, 0, 255), 0.0f, 0, 1.5f);
  }
  
  // Render query point for point queries with pulsing effect
  if (mode_ == RTreeDemoMode::PointQuery) {
    float px = static_cast<float>(canvas_x + query_point_.x);
    float py = static_cast<float>(canvas_y + canvas_height - query_point_.y);
    
    // Outer glow
    draw_list->AddCircle(ImVec2(px, py), 15.0f, IM_COL32(255, 255, 0, 50), 0, 2.0f);
    draw_list->AddCircle(ImVec2(px, py), 10.0f, IM_COL32(255, 255, 0, 100), 0, 2.0f);
    // Inner bright circle
    draw_list->AddCircleFilled(ImVec2(px, py), 6.0f, IM_COL32(255, 255, 0, 200));
    draw_list->AddCircle(ImVec2(px, py), 6.0f, IM_COL32(255, 255, 255, 255), 0, 2.0f);
  }
}

void RTreeScene::RenderRectangle(const BoundingBox& bounds, ImU32 color,
                                float fill_alpha, float canvas_x,
                                float canvas_y, float canvas_height) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  
  float min_x = static_cast<float>(canvas_x + bounds.min_x);
  float min_y = static_cast<float>(canvas_y + canvas_height - bounds.max_y);
  float max_x = static_cast<float>(canvas_x + bounds.max_x);
  float max_y = static_cast<float>(canvas_y + canvas_height - bounds.min_y);
  
  ImVec2 p1(min_x, min_y);
  ImVec2 p2(max_x, max_y);
  
  if (fill_alpha > 0) {
    ImU32 fill_color = (color & 0xFF000000) | 
                       (static_cast<int>(fill_alpha * 255) << 16) |
                       (static_cast<int>(fill_alpha * 255) << 8) |
                       static_cast<int>(fill_alpha * 255);
    draw_list->AddRectFilled(p1, p2, fill_color);
  }
  
  // Draw border with slight rounding for better appearance
  draw_list->AddRect(p1, p2, color, 2.0f, 0, 2.0f);
}

ImU32 RTreeScene::GetDepthColor(int depth, int max_depth) {
  // Beautiful gradient: cyan -> blue -> purple -> magenta -> orange
  float t = max_depth > 0 ? static_cast<float>(depth) / max_depth : 0;
  
  float r, g, b;
  if (t < 0.25f) {
    // Cyan to Blue
    float local_t = t / 0.25f;
    r = 0;
    g = 255 * (1 - local_t);
    b = 255;
  } else if (t < 0.5f) {
    // Blue to Purple
    float local_t = (t - 0.25f) / 0.25f;
    r = 128 * local_t;
    g = 0;
    b = 255;
  } else if (t < 0.75f) {
    // Purple to Magenta
    float local_t = (t - 0.5f) / 0.25f;
    r = 128 + 127 * local_t;
    g = 0;
    b = 255 * (1 - local_t);
  } else {
    // Magenta to Orange
    float local_t = (t - 0.75f) / 0.25f;
    r = 255;
    g = 165 * local_t;
    b = 0;
  }
  
  return IM_COL32(static_cast<int>(r), 
                  static_cast<int>(g), 
                  static_cast<int>(b), 180);
}

ImU32 RTreeScene::GetRectangleColor(int index) {
  // Generate beautiful, distinct colors using golden angle
  float hue = (index * 137.508f) / 360.0f;  // Golden angle in degrees
  
  // HSV to RGB with high saturation and value for vibrant colors
  float h = hue * 6.0f;  // 0-6
  float s = 0.75f;
  float v = 0.95f;
  
  float c = v * s;
  float x = c * (1 - std::abs(std::fmod(h, 2.0f) - 1.0f));
  float m = v - c;
  
  float r, g, b;
  if (h < 1) { r = c; g = x; b = 0; }
  else if (h < 2) { r = x; g = c; b = 0; }
  else if (h < 3) { r = 0; g = c; b = x; }
  else if (h < 4) { r = 0; g = x; b = c; }
  else if (h < 5) { r = x; g = 0; b = c; }
  else { r = c; g = 0; b = x; }
  
  return IM_COL32(static_cast<int>((r + m) * 255), 
                  static_cast<int>((g + m) * 255), 
                  static_cast<int>((b + m) * 255), 255);
}

void RTreeScene::RenderUI() {
  ImGui::Text("R-Tree Demo");
  ImGui::Separator();
  
  // Mode selection
  const char* modes[] = { "Insert Rectangles", "Range Query", 
                          "Point Query", "Intersection Query" };
  int current_mode = static_cast<int>(mode_);
  if (ImGui::Combo("Mode", &current_mode, modes, 4)) {
    mode_ = static_cast<RTreeDemoMode>(current_mode);
    show_query_ = false;
    is_creating_rect_ = false;
  }
  
  ImGui::Separator();
  
  // Settings
  ImGui::Checkbox("Show Rectangles", &show_rectangles_);
  ImGui::Checkbox("Show Tree Structure", &show_tree_);
  ImGui::Checkbox("Show MBRs", &show_mbrs_);
  ImGui::Checkbox("Show Query Results", &show_query_results_);
  
  ImGui::Separator();
  
  // R-tree parameters
  int old_max = max_entries_;
  int old_min = min_entries_;
  
  ImGui::SliderInt("Max Entries", &max_entries_, 2, 10);
  ImGui::SliderInt("Min Entries", &min_entries_, 1, max_entries_ / 2);
  
  if (old_max != max_entries_ || old_min != min_entries_) {
    RebuildTree();
  }
  
  ImGui::Separator();
  
  // Test data generation with categories
  ImGui::Text("Basic Patterns:");
  if (ImGui::Button("Random (30)")) {
    GenerateRandomRectangles(30);
  }
  ImGui::SameLine();
  if (ImGui::Button("Random (60)")) {
    GenerateRandomRectangles(60);
  }
  ImGui::SameLine();
  if (ImGui::Button("Random (100)")) {
    GenerateRandomRectangles(100);
  }
  
  if (ImGui::Button("Grid 5x5")) {
    GenerateGridRectangles(5, 5);
  }
  ImGui::SameLine();
  if (ImGui::Button("Grid 8x8")) {
    GenerateGridRectangles(8, 8);
  }
  ImGui::SameLine();
  if (ImGui::Button("Grid 10x10")) {
    GenerateGridRectangles(10, 10);
  }
  
  ImGui::Text("Artistic Patterns:");
  if (ImGui::Button("Spiral (50)")) {
    GenerateSpiralRectangles(50);
  }
  ImGui::SameLine();
  if (ImGui::Button("Spiral (100)")) {
    GenerateSpiralRectangles(100);
  }
  
  if (ImGui::Button("Concentric (4x12)")) {
    GenerateConcentricRectangles(4, 12);
  }
  ImGui::SameLine();
  if (ImGui::Button("Concentric (6x16)")) {
    GenerateConcentricRectangles(6, 16);
  }
  
  ImGui::Text("Real-world Patterns:");
  if (ImGui::Button("City Blocks (6x8)")) {
    GenerateCityBlocks(6, 8);
  }
  ImGui::SameLine();
  if (ImGui::Button("City Blocks (8x10)")) {
    GenerateCityBlocks(8, 10);
  }
  
  if (ImGui::Button("Clusters (5x10)")) {
    GenerateRandomClusters(5, 10);
  }
  ImGui::SameLine();
  if (ImGui::Button("Clusters (8x8)")) {
    GenerateRandomClusters(8, 8);
  }
  
  ImGui::Text("Stress Tests:");
  if (ImGui::Button("Overlapping (30)")) {
    GenerateOverlappingRectangles(30);
  }
  ImGui::SameLine();
  if (ImGui::Button("Overlapping (60)")) {
    GenerateOverlappingRectangles(60);
  }
  
  ImGui::Separator();
  
  // Statistics
  ImGui::Text("Statistics:");
  ImGui::Text("Rectangles: %zu", rectangles_.size());
  ImGui::Text("Tree Depth: %d", rtree_->GetDepth());
  ImGui::Text("Node Count: %d", rtree_->GetNodeCount());
  ImGui::Text("Tree Height: %d", rtree_->GetHeight());
  
  if (show_query_) {
    ImGui::Separator();
    ImGui::Text("Query Results:");
    ImGui::Text("Found: %d rectangles", last_query_count_);
    ImGui::Text("Time: %.3f ms", last_query_time_ms_);
  }
  
  ImGui::Separator();
  
  // Instructions
  ImGui::Text("Instructions:");
  if (mode_ == RTreeDemoMode::InsertRectangles) {
    ImGui::BulletText("Click and drag to create rectangles");
  } else if (mode_ == RTreeDemoMode::RangeQuery) {
    ImGui::BulletText("Click and drag to define query range");
  } else if (mode_ == RTreeDemoMode::PointQuery) {
    ImGui::BulletText("Click to query rectangles at point");
  } else if (mode_ == RTreeDemoMode::IntersectionQuery) {
    ImGui::BulletText("Click and drag to define query rectangle");
  }
}

}  // namespace geometry
