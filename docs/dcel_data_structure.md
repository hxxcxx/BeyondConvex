# DCEL 数据结构

## 1. 概述

### 1.1 什么是 DCEL？

**DCEL**（Doubly Connected Edge List，双向连接边表）是一种用于表示**平面剖分**（Planar Subdivision）的经典数据结构。它将平面图分解为三种基本元素：顶点（Vertex）、边（Edge）和面（Face），并通过指针连接这些元素，支持高效的拓扑查询和遍历。

### 1.2 核心思想

DCEL 的核心思想是将每条**无向边**拆分为两条**有向半边**（Half-Edge），每条半边包含：

- **起点**（Origin Vertex）
- **对偶半边**（Twin Half-Edge）- 反方向的半边
- **下一条半边**（Next Half-Edge）- 同一面边界的下一条边
- **上一条半边**（Prev Half-Edge）- 同一面边界的上一条边
- **左侧面**（Face）- 该半边左侧的面

### 1.3 数据结构特点

| 特性 | 描述 |
|------|------|
| **空间复杂度** | O(V + E + F) - 线性空间 |
| **边查询** | O(1) - 通过半边指针直接访问 |
| **面遍历** | O(k) - k 为面边界边数 |
| **邻接查询** | O(1) - 通过半边指针获取邻接信息 |
| **适用场景** | 平面剖分、多边形、三角剖分、Voronoi 图 |

---

## 2. DCEL 组成元素

### 2.1 顶点（Vertex）

```cpp
class Vertex {
  Point2D coordinates_;      // 顶点坐标
  int id_;                   // 唯一标识符
  HalfEdge* incident_edge_;  // 一条从该顶点出发的半边
};
```

**职责：**
- 存储顶点的空间位置
- 维护指向一条关联半边的指针
- 支持度数计算和边界判断

**关键操作：**
- `Degree()` - 计算顶点度数（关联边数）
- `IsOnBoundary()` - 判断是否在剖分边界上

### 2.2 半边（Half-Edge）

```cpp
class HalfEdge {
  int id_;              // 唯一标识符
  Vertex* origin_;      // 起点
  HalfEdge* twin_;      // 对偶半边（反向）
  HalfEdge* next_;      // 下一条半边（逆时针）
  HalfEdge* prev_;      // 上一条半边
  Face* face_;          // 左侧的面
};
```

**职责：**
- 表示有向边
- 维护拓扑连接关系
- 支持高效的邻接查询

**关键操作：**
- `GetDestination()` - 获取终点（通过 twin->origin）
- `IsOnBoundary()` - 判断是否在边界上
- `GetAdjacentEdges()` - 获取同面的邻接半边

### 2.3 面（Face）

```cpp
class Face {
  int id_;                           // 唯一标识符
  HalfEdge* outer_component_;        // 外边界的任意一条半边
  std::vector<HalfEdge*> inner_components_;  // 内边界（孔洞）
};
```

**职责：**
- 表示平面剖分中的区域
- 维护外边界和内边界（孔洞）
- 支持边界遍历

**关键操作：**
- `GetOuterBoundaryVertices()` - 获取外边界顶点（逆时针）
- `GetHoleVertices()` - 获取孔洞顶点（顺时针）
- `OuterBoundarySize()` - 外边界边数
- `IsUnbounded()` - 判断是否为无界面

---

## 3. DCEL 结构示意图

### 3.1 基本结构

```
                    ┌─────────────────┐
                    │   Face (面)     │
                    │                 │
                    │ outer_component │──┐
                    └─────────────────┘  │
                                         ▼
                    ┌─────────────────────────────────┐
                    │         Half-Edge (半边)         │
                    │                                 │
                    │  origin ──────────────────► Vertex
                    │  twin ◄──────────────────► Half-Edge
                    │  next ───────────────────► Half-Edge
                    │  prev ◄──────────────────► Half-Edge
                    │  face ───────────────────► Face
                    └─────────────────────────────────┘
```

### 3.2 半边连接关系

```
        ┌─────────────────────────────────────────────────────┐
        │                                                     │
        │   v1 ──────────────────────────────────────────► v2 │
        │        he1 (next → he2, prev → he3)                  │
        │                                                     │
        │   v2 ──────────────────────────────────────────► v3 │
        │        he2 (next → he3, prev → he1)                  │
        │                                                     │
        │   v3 ──────────────────────────────────────────► v1 │
        │        he3 (next → he1, prev → he2)                  │
        │                                                     │
        │   所有半边的 face 指向同一个 Face                     │
        │                                                     │
        └─────────────────────────────────────────────────────┘
```

### 3.3 对偶半边关系

```
        ┌─────────────────────────────────────────────────────┐
        │                                                     │
        │   v1 ──────────────────────────────────────────► v2 │
        │        he (face → Face A)                           │
        │                                                     │
        │   v2 ◄─────────────────────────────────────────── v1 │
        │        he->twin (face → Face B)                     │
        │                                                     │
        │   he 和 he->twin 互为对偶，方向相反                   │
        │   he 的左侧是 Face A，he->twin 的左侧是 Face B        │
        │                                                     │
        └─────────────────────────────────────────────────────┘
```

---

## 4. DCEL 操作

### 4.1 基本操作

#### 创建顶点
```cpp
DCEL dcel;
Vertex* v1 = dcel.CreateVertex(Point2D(0.0, 0.0));
Vertex* v2 = dcel.CreateVertex(Point2D(1.0, 0.0));
Vertex* v3 = dcel.CreateVertex(Point2D(0.5, 1.0));
```

#### 创建边
```cpp
// 创建一条无向边（自动创建两条半边）
HalfEdge* he1 = dcel.CreateEdge(v1, v2);  // he1: v1 → v2
HalfEdge* he2 = he1->GetTwin();           // he2: v2 → v1
```

#### 创建面
```cpp
Face* face = dcel.CreateFace();
```

#### 连接半边
```cpp
// 将半边连接成环（逆时针）
dcel.ConnectHalfEdges(he1, he2);  // he1 的 next 是 he2
dcel.ConnectHalfEdges(he2, he3);  // he2 的 next 是 he3
dcel.ConnectHalfEdges(he3, he1);  // he3 的 next 是 he1（闭合）
```

#### 设置面
```cpp
// 将半边环设置为一个面的边界
dcel.SetFaceOfCycle(he1, face);
```

### 4.2 高级操作

#### 分割面
```cpp
// 在面内添加对角线，将面分割为两个面
HalfEdge* diagonal = dcel.SplitFace(face, v1, v3);
```

#### 合并面
```cpp
// 移除两个面之间的边，合并为一个面
Face* merged = dcel.MergeFaces(edge);
```

#### 验证结构
```cpp
bool valid = dcel.Validate();  // 检查 DCEL 结构完整性
```

---

## 5. DCELBuilder 使用

### 5.1 构建简单多边形

```cpp
DCEL dcel;
std::vector<Point2D> vertices = {
  {0.0, 0.0}, {2.0, 0.0}, {2.0, 2.0}, {0.0, 2.0}
};

// 构建矩形（逆时针顺序）
Face* face = DCELBuilder::BuildPolygon(vertices, &dcel);

// 访问边界
auto boundary_vertices = face->GetOuterBoundaryVertices();
// 结果: {(0,0), (2,0), (2,2), (0,2)}
```

### 5.2 构建带孔多边形

```cpp
DCEL dcel;

// 外边界（逆时针）
std::vector<Point2D> outer = {
  {0.0, 0.0}, {4.0, 0.0}, {4.0, 4.0}, {0.0, 4.0}
};

// 孔洞（顺时针）
std::vector<std::vector<Point2D>> holes = {
  {{1.0, 1.0}, {3.0, 1.0}, {3.0, 3.0}, {1.0, 3.0}}
};

Face* face = DCELBuilder::BuildPolygonWithHoles(outer, holes, &dcel);
```

### 5.3 构建三角剖分

```cpp
DCEL dcel;

// 顶点
std::vector<Point2D> vertices = {
  {0.0, 0.0}, {1.0, 0.0}, {0.5, 1.0}, {0.5, 0.5}
};

// 三角形索引（每个三元组是一个三角形）
std::vector<std::array<int, 3>> triangles = {
  {0, 1, 3},  // 下半部分
  {0, 3, 2},  // 左上半部分
  {1, 2, 3}   // 右上半部分
};

auto faces = DCELBuilder::BuildTriangulation(vertices, triangles, &dcel);
// 返回 3 个面，每个面对应一个三角形
```

### 5.4 构建包围盒

```cpp
DCEL dcel;
Face* bbox = DCELBuilder::BuildBoundingBox(
  -10.0, -10.0,  // min_x, min_y
   10.0,  10.0,  // max_x, max_y
  &dcel
);
```

### 5.5 构建 Voronoi 图

```cpp
DCEL dcel;
std::vector<Point2D> sites = {{0, 0}, {1, 1}, {2, 0}};
std::vector<Edge2D> voronoi_edges = /* ... */;

auto cells = DCELBuilder::BuildVoronoiDiagram(
  sites, voronoi_edges,
  -10.0, -10.0,  // bounds_min
   10.0,  10.0,  // bounds_max
  &dcel
);
// 返回每个 Voronoi 单元对应的面
```

---

## 6. 遍历操作

### 6.1 遍历面的边界

```cpp
void TraverseFaceBoundary(Face* face) {
  HalfEdge* start = face->GetOuterComponent();
  HalfEdge* current = start;
  
  do {
    Vertex* v = current->GetOrigin();
    std::cout << "Vertex: (" << v->GetCoordinates().x 
              << ", " << v->GetCoordinates().y << ")\n";
    
    current = current->GetNext();
  } while (current != start);
}
```

### 6.2 遍历顶点的邻接边

```cpp
void TraverseVertexEdges(Vertex* vertex) {
  HalfEdge* start = vertex->GetIncidentEdge();
  HalfEdge* current = start;
  
  do {
    // 获取从该顶点出发的半边
    std::cout << "Edge to: " 
              << current->GetDestination()->GetId() << "\n";
    
    // 移动到下一条邻接半边（逆时针）
    current = current->GetTwin()->GetNext();
  } while (current != start);
}
```

### 6.3 遍历所有面

```cpp
void TraverseAllFaces(const DCEL& dcel) {
  for (const auto& face : dcel.GetFaces()) {
    if (!face->IsUnbounded()) {
      std::cout << "Face " << face->GetId() 
                << " has " << face->OuterBoundarySize() 
                << " boundary edges\n";
    }
  }
}
```

---

## 7. 欧拉公式验证

### 7.1 平面图欧拉公式

对于连通平面图：
$$V - E + F = 1 + C$$

其中：
- $V$ = 顶点数
- $E$ = 边数（无向边）
- $F$ = 面数（包括无界面）
- $C$ = 连通分量数

### 7.2 DCEL 中的验证

```cpp
void ValidateEulerFormula(const DCEL& dcel) {
  size_t V = dcel.GetVertexCount();
  size_t E = dcel.GetEdgeCount();  // 半边数 / 2
  size_t F = dcel.GetFaceCount();
  
  int euler_characteristic = static_cast<int>(V) - 
                             static_cast<int>(E) + 
                             static_cast<int>(F);
  
  std::cout << "V - E + F = " << euler_characteristic << "\n";
  // 对于单连通平面图，应该等于 2（包括无界面）
}
```

---

## 8. 应用场景

### 8.1 多边形操作

- **多边形分割**：将复杂多边形分割为简单多边形
- **多边形合并**：合并相邻的多边形区域
- **孔洞检测**：识别和处理多边形中的孔洞

### 8.2 三角剖分

- **Delaunay 三角剖分**：存储和查询三角形
- **约束三角剖分**：处理预定义边约束
- **三角网格优化**：边翻转和顶点插入

### 8.3 Voronoi 图

- **Voronoi 单元查询**：快速访问每个单元的边界
- **最近邻搜索**：通过 Voronoi 对偶关系
- **空间划分**：用于空间索引

### 8.4 计算几何算法

- **线段求交**：维护平面剖分状态
- **多边形求交**：计算两个多边形的交集
- **点定位**：确定点在哪个面中

---

## 9. 复杂度分析

### 9.1 空间复杂度

| 元素 | 存储空间 | 说明 |
|------|---------|------|
| 顶点 | O(V) | 每个顶点存储坐标和一条半边指针 |
| 半边 | O(2E) | 每条无向边拆分为两条半边 |
| 面 | O(F) | 每个面存储边界指针和孔洞列表 |
| **总计** | **O(V + E + F)** | 线性空间 |

### 9.2 时间复杂度

| 操作 | 时间复杂度 | 说明 |
|------|-----------|------|
| 创建顶点 | O(1) | 直接添加到顶点列表 |
| 创建边 | O(1) | 创建两条半边并设置对偶关系 |
| 创建面 | O(1) | 直接添加到面列表 |
| 连接半边 | O(1) | 设置 next/prev 指针 |
| 遍历面边界 | O(k) | k 为边界边数 |
| 遍历顶点邻接边 | O(d) | d 为顶点度数 |
| 分割面 | O(1) | 添加对角线并更新指针 |
| 合并面 | O(1) | 移除边并更新指针 |
| 验证结构 | O(V + E + F) | 遍历所有元素检查一致性 |

---

## 10. 实现细节

### 10.1 内存管理

```cpp
class DCEL {
  // 使用 unique_ptr 自动管理内存
  std::vector<std::unique_ptr<Vertex>> vertices_;
  std::vector<std::unique_ptr<HalfEdge>> half_edges_;
  std::vector<std::unique_ptr<Face>> faces_;
  
  // 禁止拷贝，允许移动
  DCEL(const DCEL&) = delete;
  DCEL& operator=(const DCEL&) = delete;
  DCEL(DCEL&&) = default;
  DCEL& operator=(DCEL&&) = default;
};
```

### 10.2 ID 分配

```cpp
class DCEL {
  int next_vertex_id_ = 0;
  int next_edge_id_ = 0;
  int next_face_id_ = 0;
  
  Vertex* CreateVertex(const Point2D& coordinates) {
    auto vertex = std::make_unique<Vertex>(coordinates, next_vertex_id_++);
    vertices_.push_back(std::move(vertex));
    return vertices_.back().get();
  }
};
```

### 10.3 边界判断

```cpp
bool HalfEdge::IsOnBoundary() const {
  // 如果对偶半边没有面，则在边界上
  return twin_->face_ == nullptr || face_ == nullptr;
}

bool Vertex::IsOnBoundary() const {
  // 检查所有关联半边是否有一条在边界上
  HalfEdge* start = incident_edge_;
  HalfEdge* current = start;
  
  do {
    if (current->IsOnBoundary()) {
      return true;
    }
    current = current->GetTwin()->GetNext();
  } while (current != start);
  
  return false;
}
```

---

## 11. 调试和验证

### 11.1 结构验证

```cpp
bool DCEL::Validate() const {
  // 检查所有半边的对偶关系
  for (const auto& he : half_edges_) {
    if (he->GetTwin()->GetTwin() != he.get()) {
      return false;  // 对偶关系不对称
    }
  }
  
  // 检查所有半边的 next/prev 关系
  for (const auto& he : half_edges_) {
    if (he->GetNext()->GetPrev() != he.get()) {
      return false;  // next/prev 关系不对称
    }
  }
  
  // 检查所有面的边界环
  for (const auto& face : faces_) {
    if (face->HasOuterComponent()) {
      HalfEdge* start = face->GetOuterComponent();
      HalfEdge* current = start;
      int count = 0;
      
      do {
        if (current->GetFace() != face.get()) {
          return false;  // 半边的面指针不正确
        }
        current = current->GetNext();
        count++;
      } while (current != start && count < 10000);
      
      if (count >= 10000) {
        return false;  // 可能存在循环
      }
    }
  }
  
  return true;
}
```

### 11.2 统计信息

```cpp
void DCEL::PrintStats() const {
  std::cout << "=== DCEL Statistics ===\n";
  std::cout << "Vertices: " << GetVertexCount() << "\n";
  std::cout << "Half-Edges: " << GetHalfEdgeCount() << "\n";
  std::cout << "Edges: " << GetEdgeCount() << "\n";
  std::cout << "Faces: " << GetFaceCount() << "\n";
  
  // 计算欧拉示性数
  int euler = static_cast<int>(GetVertexCount()) - 
              static_cast<int>(GetEdgeCount()) + 
              static_cast<int>(GetFaceCount());
  std::cout << "Euler Characteristic (V - E + F): " << euler << "\n";
  
  // 统计边界边
  int boundary_edges = 0;
  for (const auto& he : half_edges_) {
    if (he->IsOnBoundary()) {
      boundary_edges++;
    }
  }
  std::cout << "Boundary Half-Edges: " << boundary_edges << "\n";
}
```

---

## 12. 与其他数据结构的比较

### 12.1 DCEL vs. 邻接表

| 特性 | DCEL | 邻接表 |
|------|------|--------|
| 面信息 | ✅ 显式存储 | ❌ 不支持 |
| 边遍历 | ✅ O(1) 指针访问 | ⚠️ O(degree) 查找 |
| 空间 | O(V + E + F) | O(V + E) |
| 实现复杂度 | ⚠️ 较高 | ✅ 简单 |
| 适用场景 | 平面剖分 | 一般图 |

### 12.2 DCEL vs. 四叉树

| 特性 | DCEL | 四叉树 |
|------|------|--------|
| 精确表示 | ✅ 精确拓扑 | ⚠️ 近似空间划分 |
| 点定位 | ⚠️ O(n) 线性查找 | ✅ O(log n) 树查找 |
| 动态更新 | ✅ 局部更新 | ⚠️ 需要重建 |
| 内存 | O(V + E + F) | O(n) 节点数 |

---

## 13. 最佳实践

### 13.1 顶点顺序

- **外边界**：逆时针（CCW）顺序
- **内边界（孔洞）**：顺时针（CW）顺序
- 这样可以保证半边的左侧始终在面内部

### 13.2 内存效率

- 使用 `unique_ptr` 自动管理内存
- 避免循环引用（DCEL 中不存在）
- 考虑使用对象池减少分配开销

### 13.3 错误处理

```cpp
Vertex* GetVertex(int id) const {
  if (id < 0 || id >= static_cast<int>(vertices_.size())) {
    return nullptr;  // 或抛出异常
  }
  return vertices_[id].get();
}
```

### 13.4 性能优化

- 预分配内存：`vertices_.reserve(expected_count);`
- 使用迭代器而非索引遍历
- 缓存频繁访问的指针

---

## 14. 总结

DCEL 是表示平面剖分的强大数据结构，具有以下优势：

✅ **完整的拓扑信息**：存储顶点、边、面的完整关系  
✅ **高效的邻接查询**：O(1) 时间获取邻接信息  
✅ **灵活的拓扑操作**：支持面分割、合并等操作  
✅ **广泛的应用**：适用于多种计算几何算法  

⚠️ **注意事项**：
- 实现复杂度较高
- 需要仔细维护指针一致性
- 动态更新时需要验证结构完整性

---

## 15. 参考资源

- **Computational Geometry: Algorithms and Applications** - de Berg et al.
- **Half-Edge Data Structure** - CGAL Documentation
- **Planar Subdivisions** - Computational Geometry Literature

---

<div align="center">

**DCEL 数据结构实现位置**：`src/dcel/`

**测试场景**：运行 `geometry_viewer` → 选择 "DCEL Test" 场景

</div>
