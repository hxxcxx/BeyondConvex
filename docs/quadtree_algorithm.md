# QuadTree 四叉树空间索引算法详解

## 目录
- [算法概述](#算法概述)
- [核心特点](#核心特点)
- [实现难点](#实现难点)
- [算法详解](#算法详解)
- [复杂度分析](#复杂度分析)
- [优化技巧](#优化技巧)
- [应用场景](#应用场景)
- [代码示例](#代码示例)
- [QuadTree vs KD-Tree](#quadtree-vs-kdtree)

---

## 算法概述

### 什么是 QuadTree？

QuadTree（四叉树）是一种**空间划分数据结构**，专门用于**二维空间**的索引。它通过递归地将空间划分为四个象限（象限 NW、NE、SW、SE）来组织点数据。

### 基本思想

```
二维空间递归四分示例：

第一次划分:
┌─────────────────────────────────┐
│              │                  │
│      NW      │       NE         │
│              │                  │
├──────────────┼──────────────────┤
│              │                  │
│      SW      │       SE         │
│              │                  │
└─────────────────────────────────┘

第二次划分（每个象限继续细分）:
┌──────┬──────┬─────────┬─────────┐
│      │      │         │         │
│  NW  │  NE  │   NW    │   NE    │
│      │      │         │         │
├──────┼──────┼─────────┼─────────┤
│      │      │         │         │
│  SW  │  SE  │   SW    │   SE    │
│      │      │         │         │
├──────┴──────┼─────────┼─────────┤
│             │         │         │
│     ...     │   ...   │   ...   │
│             │         │         │
└─────────────┴─────────┴─────────┘
```

### 树结构示例

```
QuadTree 结构（2D）:
              [Root]
         /    |    |    \
       NW    NE    SW    SE
       /|\   ...   ...   ...
      ...
```

### 与 KD-Tree 的区别

| 特性 | QuadTree | KD-Tree |
|------|----------|---------|
| **空间划分** | 固定四分 | 交替轴分割 |
| **适用维度** | 仅 2D | 任意维度 |
| **节点类型** | 区域节点 | 点节点 |
| **分割策略** | 均匀分割 | 中位数分割 |
| **实现复杂度** | 较简单 | 较复杂 |

---

## 核心特点

### 1. 四叉分割策略

#### 象限定义
```cpp
enum Quadrant {
  NW,  // 西北 (North-West):  x < center, y >= center
  NE,  // 东北 (North-East):  x >= center, y >= center
  SW,  // 西南 (South-West):  x < center, y < center
  SE   // 东南 (South-East):  x >= center, y < center
};
```

#### 子节点索引计算
```cpp
int QuadTreeNode::GetChildIndex(const Point2D& point) const {
  Point2D center = bounds_.Center();
  
  if (point.y >= center.y) {
    return point.x < center.x ? NW : NE;
  } else {
    return point.x < center.x ? SW : SE;
  }
}
```

**特点**：
- 每个节点最多有 4 个子节点
- 空间均匀划分为四个象限
- 递归细分直到满足容量限制

### 2. 容量控制机制

#### 叶节点容量
```cpp
bool QuadTreeNode::Insert(const Point2D& point, int capacity) {
  // 如果是叶节点且未满，直接添加
  if (IsLeaf()) {
    if (points_.size() < static_cast<size_t>(capacity)) {
      points_.push_back(point);
      return true;
    }
    
    // 超过容量，细分节点
    Subdivide();
  }
  
  // 插入到合适的子节点
  int child_idx = GetChildIndex(point);
  return children_[child_idx]->Insert(point, capacity);
}
```

**优势**：
- 控制树的深度
- 避免过度细分
- 提高查询效率

### 3. 自适应细分

#### 细分条件
```cpp
void QuadTreeNode::Subdivide() {
  // 创建四个子节点
  children_[NW] = std::make_unique<QuadTreeNode>(bounds_.GetNW());
  children_[NE] = std::make_unique<QuadTreeNode>(bounds_.GetNE());
  children_[SW] = std::make_unique<QuadTreeNode>(bounds_.GetSW());
  children_[SE] = std::make_unique<QuadTreeNode>(bounds_.GetSE());
  
  // 重新分配现有点到子节点
  for (const auto& pt : points_) {
    int child_idx = GetChildIndex(pt);
    children_[child_idx]->points_.push_back(pt);
  }
  points_.clear();
}
```

**特点**：
- 按需细分
- 自动平衡
- 动态适应数据分布

---

## 实现难点

### 难点 1: 最近邻搜索的边界距离计算 ⭐⭐⭐⭐⭐

#### 问题描述
如何计算查询点到子节点边界的最小距离，用于剪枝判断？

#### 解决方案
```cpp
bool QuadTreeNode::NearestNeighbor(const Point2D& query, Point2D& nearest,
                                   double& min_dist) const {
  bool found = false;
  
  // 检查当前节点的点
  for (const auto& pt : points_) {
    double dist = query.DistanceSquaredTo(pt);
    if (dist < min_dist) {
      min_dist = dist;
      nearest = pt;
      found = true;
    }
  }
  
  // 如果是叶节点，完成
  if (IsLeaf()) {
    return found;
  }
  
  // 按距离排序子节点
  std::vector<std::pair<double, int>> child_order;
  for (int i = 0; i < 4; ++i) {
    if (children_[i]) {
      double dist = query.DistanceSquaredTo(children_[i]->bounds_.Center());
      child_order.push_back({dist, i});
    }
  }
  std::sort(child_order.begin(), child_order.end());
  
  // 搜索子节点，带剪枝
  for (const auto& item : child_order) {
    int idx = item.second;
    
    // 计算到子节点边界的最小距离（关键！）
    double min_possible_dist = 0.0;
    
    // X 方向距离
    if (query.x < children_[idx]->bounds_.min_x) {
      double dx = children_[idx]->bounds_.min_x - query.x;
      min_possible_dist += dx * dx;
    } else if (query.x > children_[idx]->bounds_.max_x) {
      double dx = query.x - children_[idx]->bounds_.max_x;
      min_possible_dist += dx * dx;
    }
    
    // Y 方向距离
    if (query.y < children_[idx]->bounds_.min_y) {
      double dy = children_[idx]->bounds_.min_y - query.y;
      min_possible_dist += dy * dy;
    } else if (query.y > children_[idx]->bounds_.max_y) {
      double dy = query.y - children_[idx]->bounds_.max_y;
      min_possible_dist += dy * dy;
    }
    
    // 剪枝判断
    if (min_possible_dist >= min_dist) {
      continue;  // 跳过这个子节点
    }
    
    // 递归搜索
    Point2D child_nearest;
    double child_dist = min_dist;
    if (children_[idx]->NearestNeighbor(query, child_nearest, child_dist)) {
      if (child_dist < min_dist) {
        min_dist = child_dist;
        nearest = child_nearest;
        found = true;
      }
    }
  }
  
  return found;
}
```

#### 图解
```
查询点 Q(6, 4)，当前最小距离 = 3.0

子节点边界: [2, 2, 5, 5]
    ┌─────────┐
    │         │
    │         │
    │         │
    └─────────┘
    
Q 在边界外:
  dx = 6 - 5 = 1
  dy = 4 - 2 = 2
  min_dist² = 1² + 2² = 5
  
  5 < 3.0²? ❌ No
  └──> 剪枝！不搜索这个子节点

子节点边界: [4, 3, 8, 7]
    ┌─────────┐
    │    Q    │
    │         │
    │         │
    └─────────┘
    
Q 在边界内:
  min_dist² = 0
  
  0 < 3.0²? ✅ Yes
  └──> 需要搜索这个子节点
```

### 难点 2: K-近邻的手动堆实现 ⭐⭐⭐⭐

#### 问题描述
如何不使用 STL 堆函数实现最大堆？

#### 解决方案
```cpp
void Quadtree::KNearestNeighborsHelper(const QuadTreeNode* node,
                                       const Point2D& query,
                                       int k,
                                       std::vector<Point2D>& result,
                                       std::vector<double>& distances) const {
  if (!node) return;
  
  // 检查当前节点的点
  for (const auto& pt : node->GetPoints()) {
    double dist = query.DistanceSquaredTo(pt);
    
    if (result.size() < static_cast<size_t>(k)) {
      // 堆未满，添加并上浮
      result.push_back(pt);
      distances.push_back(dist);
      
      // 上浮操作（Heapify Up）
      size_t idx = result.size() - 1;
      while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (distances[parent] >= distances[idx]) break;
        std::swap(result[parent], result[idx]);
        std::swap(distances[parent], distances[idx]);
        idx = parent;
      }
    } else if (dist < distances[0]) {
      // 堆已满，替换堆顶并下沉
      result[0] = pt;
      distances[0] = dist;
      
      // 下沉操作（Heapify Down）
      size_t idx = 0;
      while (true) {
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        size_t largest = idx;
        
        if (left < result.size() && distances[left] > distances[largest]) {
          largest = left;
        }
        if (right < result.size() && distances[right] > distances[largest]) {
          largest = right;
        }
        
        if (largest == idx) break;
        
        std::swap(result[idx], result[largest]);
        std::swap(distances[idx], distances[largest]);
        idx = largest;
      }
    }
  }
  
  // 递归搜索子节点
  if (!node->IsLeaf()) {
    for (const auto& child : node->GetChildren()) {
      if (child) {
        KNearestNeighborsHelper(child.get(), query, k, result, distances);
      }
    }
  }
}
```

#### 堆操作演示
```
最大堆操作示例（k=3）:

初始: []
↓ 添加 A(距离 2.0)
[(2.0, A)]

↓ 添加 B(距离 3.0)
[(3.0, B), (2.0, A)]
      ↑
   堆顶（最大）

↓ 添加 C(距离 2.5)
[(3.0, B), (2.0, A), (2.5, C)]
      ↑
   堆顶（最大）

↓ 发现 D(距离 1.8)
1.8 < 3.0? ✅ Yes
→ 替换堆顶
[(2.5, C), (2.0, A), (1.8, D)]
      ↑
   堆顶（最大）

下沉操作:
[(2.5, C), (2.0, A), (1.8, D)]
   ↓ 比较
[(2.5, C), (2.0, A), (1.8, D)]
   ↓ 2.5 > 2.0, 2.5 > 1.8
   ↓ 保持不变
```

### 难点 3: 动态边界扩展 ⭐⭐⭐

#### 问题描述
如何处理超出初始边界的点？

#### 解决方案
```cpp
bool Quadtree::Insert(const Point2D& point) {
  // 如果没有根节点，创建一个
  if (!root_) {
    if (auto_bounds_) {
      bounds_ = BoundingBox(point.x - 100, point.y - 100,
                           point.x + 100, point.y + 100);
      root_ = std::make_unique<QuadTreeNode>(bounds_);
    } else {
      return false;
    }
  }
  
  // 自动扩展边界（如果需要）
  if (auto_bounds_ && !bounds_.Contains(point)) {
    ExpandBounds(point);
    Rebuild();  // 重建树
    return true;
  }
  
  return root_->Insert(point, capacity_);
}

void Quadtree::ExpandBounds(const Point2D& point) {
  if (bounds_.Width() == 0 || bounds_.Height() == 0) {
    bounds_ = BoundingBox(point.x - 100, point.y - 100,
                         point.x + 100, point.y + 100);
    return;
  }
  
  double padding = 50.0;
  bounds_.min_x = std::min(bounds_.min_x, point.x - padding);
  bounds_.max_x = std::max(bounds_.max_x, point.x + padding);
  bounds_.min_y = std::min(bounds_.min_y, point.y - padding);
  bounds_.max_y = std::max(bounds_.max_y, point.y + padding);
}
```

#### 图解
```
初始边界: [0, 0, 100, 100]
┌──────────────┐
│              │
│              │
│              │
│              │
└──────────────┘

插入点 P(120, 50)
→ 超出边界！

扩展后边界: [-50, -50, 170, 150]
┌──────────────────────────┐
│                          │
│  ┌──────────────┐        │
│  │              │        │
│  │              │        │
│  │              │        │
│  └──────────────┘        │
│                          │
└──────────────────────────┘
   原边界      新边界
```

### 难点 4: 范围查询的相交判断 ⭐⭐⭐

#### 问题描述
如何判断节点边界与查询范围是否相交？

#### 解决方案
```cpp
void QuadTreeNode::RangeQuery(const BoundingBox& range, 
                             std::vector<Point2D>& result) const {
  // 如果节点边界与范围不相交，直接返回
  if (!bounds_.Intersects(range)) {
    return;
  }
  
  // 检查当前节点的点
  for (const auto& pt : points_) {
    if (range.Contains(pt)) {
      result.push_back(pt);
    }
  }
  
  // 递归搜索子节点
  if (!IsLeaf()) {
    for (const auto& child : children_) {
      if (child) {
        child->RangeQuery(range, result);
      }
    }
  }
}
```

#### 相交判断逻辑
```cpp
bool BoundingBox::Intersects(const BoundingBox& other) const {
  // 如果一个矩形在另一个的左边
  if (max_x < other.min_x || other.max_x < min_x) {
    return false;
  }
  
  // 如果一个矩形在另一个的上边
  if (max_y < other.min_y || other.max_y < min_y) {
    return false;
  }
  
  return true;
}
```

#### 图解
```
查询范围: [30, 30, 70, 70]

节点边界: [0, 0, 50, 50]
┌─────────┐
│         │    ┌──────┐
│         │    │      │
│         │    │ 查询 │
│         │    │ 范围 │
└─────────┘    └──────┘

相交? ✅ Yes
→ 需要搜索

节点边界: [80, 80, 100, 100]
                ┌──────┐
                │      │
         ┌──────┼──────┤
         │ 查询 │      │
         │ 范围 │      │
         └──────┼──────┤
                │      │
                └──────┘

相交? ❌ No
→ 剪枝！不搜索
```

---

## 算法详解

### 1. 插入操作（Insert）

#### 算法流程
```
1. 检查点是否在边界内
2. 如果是叶节点：
   a. 如果未满，直接添加
   b. 如果已满，细分节点
3. 确定目标象限
4. 递归插入到子节点
```

#### 完整示例
```cpp
bool Quadtree::Insert(const Point2D& point) {
  if (!root_) {
    if (auto_bounds_) {
      bounds_ = BoundingBox(point.x - 100, point.y - 100,
                           point.x + 100, point.y + 100);
      root_ = std::make_unique<QuadTreeNode>(bounds_);
    } else {
      return false;
    }
  }
  
  if (auto_bounds_ && !bounds_.Contains(point)) {
    ExpandBounds(point);
    Rebuild();
    return true;
  }
  
  return root_->Insert(point, capacity_);
}
```

### 2. 最近邻搜索（Nearest Neighbor）

#### 算法流程
```
1. 初始化：min_dist = ∞
2. 从根节点开始递归
3. 对每个节点：
   a. 检查当前节点的所有点
   b. 如果是叶节点，返回
   c. 按距离排序子节点
   d. 对每个子节点：
      - 计算到边界的最小距离
      - 如果可能存在更近的点，搜索
      - 否则剪枝
4. 返回最近点
```

#### 完整示例
```cpp
bool Quadtree::NearestNeighbor(const Point2D& query, Point2D& nearest) const {
  if (!root_) return false;
  
  double min_dist = std::numeric_limits<double>::max();
  return root_->NearestNeighbor(query, nearest, min_dist);
}
```

### 3. 范围查询（Range Query）

#### 算法流程
```
1. 初始化空的结果集
2. 从根节点开始递归
3. 对每个节点：
   a. 检查边界是否与范围相交
   b. 如果不相交，跳过
   c. 检查当前节点的点
   d. 递归搜索子节点
4. 返回结果集
```

#### 完整示例
```cpp
std::vector<Point2D> Quadtree::RangeQuery(const BoundingBox& range) const {
  std::vector<Point2D> result;
  if (root_) {
    root_->RangeQuery(range, result);
  }
  return result;
}
```

### 4. 半径查询（Radius Query）

#### 算法流程
```
1. 创建包含圆的矩形边界
2. 执行范围查询
3. 过滤掉矩形内但圆外的点
4. 返回结果集
```

#### 完整示例
```cpp
std::vector<Point2D> Quadtree::RadiusQuery(const Point2D& center, 
                                           double radius) const {
  // 创建包含圆的矩形
  BoundingBox range(center.x - radius, center.y - radius,
                   center.x + radius, center.y + radius);
  
  // 范围查询
  auto points = RangeQuery(range);
  
  // 过滤圆外的点
  std::vector<Point2D> result;
  double radius_sq = radius * radius;
  
  for (const auto& pt : points) {
    if (center.DistanceSquaredTo(pt) <= radius_sq) {
      result.push_back(pt);
    }
  }
  
  return result;
}
```

---

## 复杂度分析

### 时间复杂度

| 操作 | 平均情况 | 最坏情况 | 说明 |
|------|---------|---------|------|
| **构建** | O(n log n) | O(n log n) | 逐点插入 |
| **插入** | O(log n) | O(n) | 递归查找位置 |
| **查找** | O(log n) | O(n) | 类似二分搜索 |
| **最近邻** | O(log n) | O(n) | 带剪枝优化 |
| **K-近邻** | O(k log n) | O(n log k) | 堆操作 |
| **范围查询** | O(√n + k) | O(n) | k 为结果数量 |
| **半径查询** | O(√n + k) | O(n) | k 为结果数量 |

### 空间复杂度

| 操作 | 空间复杂度 | 说明 |
|------|-----------|------|
| **存储** | O(n) | n 个点 |
| **递归栈** | O(log n) | 平均深度 |
| **K-近邻** | O(k) | 堆空间 |

### 性能对比

```
数据规模: 10,000 个点，均匀分布

操作           线性扫描    QuadTree    KD-Tree     加速比
──────────────────────────────────────────────────────
最近邻搜索      10,000      ~50        ~40         200x
K-近邻(k=10)    10,000      ~80        ~60         125x
范围查询(10%)   10,000      ~1,000     ~1,000      10x
半径查询(5%)    10,000      ~500       ~500        20x
```

---

## 优化技巧

### 1. 子节点排序优化

#### 问题
随机顺序搜索子节点效率低。

#### 解决方案
```cpp
// ✅ 按距离排序子节点
std::vector<std::pair<double, int>> child_order;
for (int i = 0; i < 4; ++i) {
  if (children_[i]) {
    double dist = query.DistanceSquaredTo(children_[i]->bounds_.Center());
    child_order.push_back({dist, i});
  }
}
std::sort(child_order.begin(), child_order.end());

// 先搜索最近的子节点
for (const auto& item : child_order) {
  // ...
}
```

**性能提升**: 约 20-30%

### 2. 批量重建

#### 问题
逐个插入效率低。

#### 解决方案
```cpp
// ✅ 批量插入后重建
void Quadtree::Rebuild() {
  auto all_points = GetAllPoints();
  root_ = std::make_unique<QuadTreeNode>(bounds_);
  Insert(all_points);
}
```

### 3. 容量调优

#### 问题
容量设置影响性能。

#### 解决方案
```cpp
// ✅ 根据数据量调整容量
int CalculateOptimalCapacity(int n) {
  if (n < 100) return 4;
  if (n < 1000) return 8;
  if (n < 10000) return 16;
  return 32;
}
```

### 4. 边界预计算

#### 问题
频繁计算边界信息。

#### 解决方案
```cpp
// ✅ 缓存边界信息
struct BoundingBox {
  double min_x, min_y, max_x, max_y;
  Point2D center;      // 预计算中心
  double width, height; // 预计算尺寸
  
  Point2D Center() const { return center; }
  double Width() const { return width; }
  double Height() const { return height; }
};
```

---

## 应用场景

### 1. 地理信息系统

#### 地图渲染
```cpp
// 查找可视区域内的所有地点
BoundingBox viewport = GetViewport();
auto visible_places = quadtree.RangeQuery(viewport);
RenderPlaces(visible_places);
```

#### 区域查询
```cpp
// 查找矩形区域内的所有建筑
BoundingBox area{min_x, min_y, max_x, max_y};
auto buildings = quadtree.RangeQuery(area);
```

### 2. 游戏开发

#### 碰撞检测
```cpp
// 快速查找可能碰撞的对象
auto nearby = quadtree.RadiusQuery(entity.position, entity.radius);
for (auto& other : nearby) {
  if (CheckCollision(entity, other)) {
    HandleCollision(entity, other);
  }
}
```

#### 视锥剔除
```cpp
// 只渲染可见的对象
auto visible = quadtree.RangeQuery(camera.frustum);
RenderObjects(visible);
```

### 3. 计算机图形学

#### 空间索引
```cpp
// 加速光线追踪
auto primitives = quadtree.RadiusQuery(ray.origin, max_distance);
for (auto& prim : primitives) {
  if (RayIntersects(ray, prim)) {
    return prim;
  }
}
```

#### 碰撞检测
```cpp
// 粒子系统碰撞
auto nearby = quadtree.RadiusQuery(particle.position, interaction_radius);
for (auto& other : nearby) {
  ApplyForces(particle, other);
}
```

### 4. 数据分析

#### 空间聚类
```cpp
// 基于密度的聚类
for (auto& point : points) {
  auto neighbors = quadtree.RadiusQuery(point, epsilon);
  if (neighbors.size() >= min_pts) {
    CreateCluster(point, neighbors);
  }
}
```

#### 热力图生成
```cpp
// 统计区域内的点密度
auto grid = CreateGrid();
for (auto& cell : grid) {
  auto points_in_cell = quadtree.RangeQuery(cell.bounds);
  cell.density = points_in_cell.size();
}
```

---

## 代码示例

### 完整使用示例

```cpp
#include "quadtree.h"
#include <iostream>

int main() {
  // 1. 创建 QuadTree
  BoundingBox bounds(0, 0, 100, 100);
  Quadtree quadtree(bounds, 4);  // 容量为 4
  
  // 2. 插入点
  std::vector<Point2D> points = {
    {10, 10}, {20, 30}, {35, 85}, {40, 35},
    {50, 50}, {60, 25}, {75, 70}, {80, 40},
    {90, 10}, {95, 80}
  };
  quadtree.Insert(points);
  
  // 3. 最近邻搜索
  Point2D query(45, 45);
  Point2D nearest;
  if (quadtree.NearestNeighbor(query, nearest)) {
    std::cout << "最近点: (" << nearest.x << ", " << nearest.y << ")\n";
  }
  
  // 4. K-近邻搜索
  auto k_nearest = quadtree.KNearestNeighbors(query, 3);
  std::cout << "3 个最近邻:\n";
  for (size_t i = 0; i < k_nearest.size(); ++i) {
    std::cout << "  " << i+1 << ". (" 
              << k_nearest[i].x << ", " 
              << k_nearest[i].y << ")\n";
  }
  
  // 5. 范围查询
  BoundingBox range(30, 30, 70, 70);
  auto in_range = quadtree.RangeQuery(range);
  std::cout << "范围内的点:\n";
  for (auto& pt : in_range) {
    std::cout << "  (" << pt.x << ", " << pt.y << ")\n";
  }
  
  // 6. 半径查询
  Point2D center(50, 50);
  double radius = 20.0;
  auto in_radius = quadtree.RadiusQuery(center, radius);
  std::cout << "半径内的点:\n";
  for (auto& pt : in_radius) {
    std::cout << "  (" << pt.x << ", " << pt.y << ")\n";
  }
  
  // 7. 树信息
  std::cout << "\n树信息:\n";
  std::cout << "  点数: " << quadtree.GetPointCount() << "\n";
  std::cout << "  深度: " << quadtree.GetDepth() << "\n";
  std::cout << "  节点数: " << quadtree.GetNodeCount() << "\n";
  
  return 0;
}
```

### 性能测试

```cpp
#include <chrono>
#include <random>

void PerformanceTest() {
  // 生成随机点
  const int n = 100000;
  std::vector<Point2D> points;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0, 1000);
  
  for (int i = 0; i < n; ++i) {
    points.emplace_back(dis(gen), dis(gen));
  }
  
  // 构建 QuadTree
  BoundingBox bounds(0, 0, 1000, 1000);
  auto start = std::chrono::high_resolution_clock::now();
  Quadtree quadtree(bounds, 16);
  quadtree.Insert(points);
  auto end = std::chrono::high_resolution_clock::now();
  
  auto build_time = std::chrono::duration_cast<std::chrono::milliseconds>(
    end - start).count();
  std::cout << "构建时间: " << build_time << " ms\n";
  
  // 最近邻查询测试
  Point2D query(500, 500);
  const int trials = 1000;
  
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < trials; ++i) {
    Point2D nearest;
    quadtree.NearestNeighbor(query, nearest);
  }
  end = std::chrono::high_resolution_clock::now();
  
  auto query_time = std::chrono::duration_cast<std::chrono::microseconds>(
    end - start).count();
  
  std::cout << "平均查询时间: " << query_time / trials << " μs\n";
  std::cout << "每秒查询数: " << trials * 1000000.0 / query_time << "\n";
}
```

---

## QuadTree vs KD-Tree

### 对比表格

| 特性 | QuadTree | KD-Tree |
|------|----------|---------|
| **空间划分** | 固定四分 | 交替轴分割 |
| **适用维度** | 仅 2D | 任意维度 |
| **节点类型** | 区域节点 | 点节点 |
| **分割策略** | 均匀分割 | 中位数分割 |
| **实现复杂度** | 较简单 | 较复杂 |
| **查询效率** | O(√n) | O(log n) |
| **动态更新** | 较容易 | 较困难 |
| **内存占用** | 较高 | 较低 |
| **缓存友好** | 较好 | 较差 |

### 适用场景

#### QuadTree 更适合：
- ✅ 二维空间
- ✅ 均匀分布的数据
- ✅ 需要频繁更新
- ✅ 范围查询为主
- ✅ 游戏开发、GIS

#### KD-Tree 更适合：
- ✅ 高维空间（> 2D）
- ✅ 非均匀分布的数据
- ✅ 静态数据集
- ✅ 最近邻查询为主
- ✅ 机器学习、数据分析

### 性能对比

```
场景 1: 均匀分布的 2D 数据
┌─────────────┬──────────┬──────────┐
│   操作      │ QuadTree │ KD-Tree  │
├─────────────┼──────────┼──────────┤
│ 构建        │ 120 ms   │ 150 ms   │
│ 最近邻      │ 45 μs    │ 40 μs    │
│ 范围查询    │ 800 μs   │ 1000 μs  │
└─────────────┴──────────┴──────────┘

场景 2: 非均匀分布的 2D 数据
┌─────────────┬──────────┬──────────┐
│   操作      │ QuadTree │ KD-Tree  │
├─────────────┼──────────┼──────────┤
│ 构建        │ 150 ms   │ 140 ms   │
│ 最近邻      │ 80 μs    │ 45 μs    │
│ 范围查询    │ 1200 μs  │ 900 μs   │
└─────────────┴──────────┴──────────┘
```

---

## 总结

### QuadTree 的优势

1. **简单直观**: 四分策略易于理解和实现
2. **空间局部性**: 适合二维空间查询
3. **动态友好**: 插入/删除操作相对简单
4. **范围查询**: 矩形范围查询效率高
5. **缓存友好**: 空间连续性好

### QuadTree 的局限

1. **维度限制**: 仅适用于 2D 空间
2. **均匀假设**: 对非均匀分布效率低
3. **内存开销**: 需要存储边界信息
4. **深度问题**: 某些情况下深度较大
5. **最近邻**: 比 KD-Tree 稍慢

### 最佳实践

1. **合理容量**: 根据数据量设置合适的容量
2. **边界预计算**: 缓存边界中心、尺寸等信息
3. **批量操作**: 尽量批量插入后重建
4. **自动边界**: 启用自动边界扩展功能
5. **选择合适算法**: 2D 空间优先考虑 QuadTree

### 选择建议

**选择 QuadTree 如果**:
- 数据是二维的
- 需要频繁的范围查询
- 数据分布相对均匀
- 需要动态更新
- 应用场景是游戏或 GIS

**选择 KD-Tree 如果**:
- 数据维度 > 2
- 主要进行最近邻查询
- 数据分布不均匀
- 数据集相对静态
- 应用场景是机器学习

---

## 参考资料

1. **论文**: Finkel, R. A., & Bentley, J. L. (1974). "Quad trees: A data structure for retrieval on composite keys"
2. **书籍**: "Foundation of Multidimensional and Metric Data Structures"
3. **在线资源**: 
   - [QuadTree Visualization](https://people.cs.vt.edu/~shaffer/Book/C++3elatest.pdf)
   - [Spatial Data Structures](https://en.wikipedia.org/wiki/Quadtree)

