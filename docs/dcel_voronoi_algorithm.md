# DCEL Voronoi 算法详解

## 目录

1. [算法概述](#算法概述)
2. [DCEL数据结构](#dcel数据结构)
3. [算法原理](#算法原理)
4. [实现细节](#实现细节)
5. [时间复杂度分析](#时间复杂度分析)
6. [空间复杂度分析](#空间复杂度分析)
7. [优缺点分析](#优缺点分析)
8. [代码示例](#代码示例)
9. [与其他算法对比](#与其他算法对比)

---

## 算法概述

### 什么是Voronoi图？

Voronoi图（又称沃罗诺伊图）是一种将平面划分为若干区域的方法。给定平面上的n个点（称为站点或sites），Voronoi图将平面划分为n个cell，每个cell包含所有距离该cell对应站点最近的点。

### 什么是DCEL？

DCEL（Doubly Connected Edge List，双向连通边表）是一种用于表示平面细分的数据结构。它维护三种类型的记录：

- **顶点（Vertex）**：平面上的点
- **半边（Half-edge）**：有向边，每条无向边由两条方向相反的半边组成
- **面（Face）**：平面细分中的区域

### DCEL Voronoi算法

DCEL Voronoi算法是一种增量式算法，通过逐步裁剪每个site的cell来构建Voronoi图。与简单的增量算法不同，DCEL算法在构建过程中维护完整的拓扑关系信息。

**算法特点：**
- 时间复杂度：O(n²)
- 空间复杂度：O(n²)
- 维护拓扑关系
- 支持邻接查询
- 可扩展性强

---

## DCEL数据结构

### 核心组件

#### 1. 顶点（Vertex）

```cpp
class Vertex {
private:
    int id_;                    // 唯一标识符
    Point2D coordinates_;       // 坐标
    HalfEdge* incident_edge_;   // 指向一条入射边
};
```

**作用：**
- 存储点的位置信息
- 通过`incident_edge_`可以访问所有相邻的边和面

#### 2. 半边（Half-edge）

```cpp
class HalfEdge {
private:
    int id_;            // 唯一标识符
    Vertex* origin_;    // 起点顶点
    HalfEdge* twin_;    // 双向边（方向相反的边）
    HalfEdge* next_;    // 下一条边（沿面边界逆时针）
    HalfEdge* prev_;    // 前一条边（沿面边界逆时针）
    Face* face_;        // 左侧的面
};
```

**作用：**
- 表示有向边
- 通过`twin_`访问反向边
- 通过`next_`和`prev_`遍历面边界
- 通过`face_`访问左侧的面

#### 3. 面（Face）

```cpp
class Face {
private:
    int id_;                       // 唯一标识符
    HalfEdge* outer_component_;    // 外边界的一条边
    std::vector<HalfEdge*> inner_components_;  // 内边界（洞）
};
```

**作用：**
- 表示平面细分中的区域
- 通过`outer_component_`访问外边界
- 支持带洞的多边形

### DCEL结构的关系

```
Vertex → incident_edge_ → HalfEdge
                          ↓
                        origin_ (指向起点)
                          ↓
                        twin_ (指向反向边)
                          ↓
                        next_ (指向下一条边)
                          ↓
                        face_ (指向左侧的面)
                          ↓
                        Face
                          ↓
                        outer_component_ (指向外边界的一条边)
```

---

## 算法原理

### 核心思想

DCEL Voronoi算法采用**增量式**方法构建Voronoi图：

1. **初始化**：为每个site创建一个初始的bounding box cell
2. **裁剪**：对每个cell，用其他site的垂直平分线进行半平面裁剪
3. **合并**：识别并合并共享边，建立twin关系
4. **提取**：从DCEL结构中提取Voronoi图的顶点、边和cell

### 半平面裁剪

**垂直平分线：**
- 给定两个点P和Q，它们的垂直平分线是到P和Q距离相等的点的集合
- 垂直平分线将平面分为两个半平面：
  - 包含P的半平面：距离P < 距离Q
  - 包含Q的半平面：距离Q < 距离P

**半平面裁剪算法：**
```
ClipPolygon(polygon, point_on_line, normal):
    result = []
    for each edge (p1, p2) in polygon:
        inside1 = IsInsideHalfPlane(p1, point_on_line, normal)
        inside2 = IsInsideHalfPlane(p2, point_on_line, normal)
        
        if inside1:
            result.append(p1)
        
        if inside1 != inside2:
            intersection = IntersectSegmentWithLine(p1, p2, point_on_line, normal)
            result.append(intersection)
    
    return result
```

### 算法流程图

```
开始
  ↓
创建DCEL结构
  ↓
为每个site创建bounding box cell
  ↓
for i = 0 to n-1:
  for j = 0 to n-1, j ≠ i:
    计算site[i]和site[j]的垂直平分线
    ↓
    获取cell[i]的当前顶点
    ↓
    用垂直平分线裁剪cell[i]
    ↓
    在DCEL中创建新的顶点和边
    ↓
    更新cell[i]指向新的边环
  ↓
合并共享边（设置twin关系）
  ↓
从DCEL提取Voronoi图
  ↓
结束
```

---

## 实现细节

### 1. 创建Bounding Box Cell

```cpp
Face* CreateBoundingBoxCell(DCEL* dcel, const Point2D& site, const VoronoiBounds& bounds) {
    // 创建4个顶点（bounding box的角）
    Vertex* v0 = dcel->CreateVertex(Point2D(bounds.min_x, bounds.min_y));
    Vertex* v1 = dcel->CreateVertex(Point2D(bounds.max_x, bounds.min_y));
    Vertex* v2 = dcel->CreateVertex(Point2D(bounds.max_x, bounds.max_y));
    Vertex* v3 = dcel->CreateVertex(Point2D(bounds.min_x, bounds.max_y));
    
    // 创建4条边
    HalfEdge* he0 = dcel->CreateEdge(v0, v1);
    HalfEdge* he1 = dcel->CreateEdge(v1, v2);
    HalfEdge* he2 = dcel->CreateEdge(v2, v3);
    HalfEdge* he3 = dcel->CreateEdge(v3, v0);
    
    // 连接边成环
    dcel->ConnectHalfEdges(he0, he1);
    dcel->ConnectHalfEdges(he1, he2);
    dcel->ConnectHalfEdges(he2, he3);
    dcel->ConnectHalfEdges(he3, he0);
    
    // 创建面
    Face* face = dcel->CreateFace();
    dcel->SetFaceOfCycle(he0, face);
    face->SetOuterComponent(he0);  // 关键：设置外边界指针
    
    return face;
}
```

**关键点：**
- 创建4个顶点表示bounding box
- 创建4条边并连接成环
- 创建Face并设置`outer_component_`指针
- `outer_component_`指针对于后续的顶点遍历至关重要

### 2. 裁剪Cell

```cpp
void ClipCell(DCEL* dcel, Face* cell, const Point2D& midpoint, const Vector2D& normal) {
    // 获取当前cell的顶点
    auto boundary_vertices = cell->GetOuterBoundaryVertices();
    
    // 转换为Point2D向量
    std::vector<Point2D> polygon;
    for (auto* v : boundary_vertices) {
        polygon.push_back(v->GetCoordinates());
    }
    
    // 使用半平面裁剪
    std::vector<Point2D> clipped = HalfPlaneClipper::ClipPolygon(polygon, midpoint, normal);
    
    if (clipped.size() >= 3) {
        // 创建新的顶点和边
        std::vector<Vertex*> new_vertices;
        std::vector<HalfEdge*> new_edges;
        
        for (const auto& p : clipped) {
            new_vertices.push_back(dcel->CreateVertex(p));
        }
        
        for (size_t i = 0; i < new_vertices.size(); ++i) {
            Vertex* v1 = new_vertices[i];
            Vertex* v2 = new_vertices[(i + 1) % new_vertices.size()];
            HalfEdge* he = dcel->CreateEdge(v1, v2);
            new_edges.push_back(he);
        }
        
        // 连接边成环
        for (size_t i = 0; i < new_edges.size(); ++i) {
            HalfEdge* he = new_edges[i];
            HalfEdge* next_he = new_edges[(i + 1) % new_edges.size()];
            dcel->ConnectHalfEdges(he, next_he);
        }
        
        // 更新cell指向新的边环
        dcel->SetFaceOfCycle(new_edges[0], cell);
        cell->SetOuterComponent(new_edges[0]);  // 关键：更新外边界指针
    }
}
```

**关键点：**
- 通过`GetOuterBoundaryVertices()`获取当前顶点
- 使用`HalfPlaneClipper`进行裁剪
- 创建新的顶点和边（保留旧的，避免复杂的指针管理）
- 更新cell的`outer_component_`指针

### 3. 合并共享边

```cpp
void MergeSharedEdges(DCEL* dcel) {
    // 使用map识别共享边
    std::map<std::pair<Point2D, Point2D>, std::vector<HalfEdge*>> edge_map;
    
    for (size_t i = 0; i < dcel->GetHalfEdgeCount(); ++i) {
        HalfEdge* edge = dcel->GetHalfEdge(i);
        if (!edge || !edge->GetOrigin() || !edge->GetDestination()) continue;
        
        Point2D p1 = edge->GetOrigin()->GetCoordinates();
        Point2D p2 = edge->GetDestination()->GetCoordinates();
        
        // 标准化边的方向（字典序）
        if (p1.x > p2.x || (p1.x == p2.x && p1.y > p2.y)) {
            std::swap(p1, p2);
        }
        
        edge_map[{p1, p2}].push_back(edge);
    }
    
    // 为共享边设置twin关系
    for (auto& [key, edges] : edge_map) {
        if (edges.size() == 2) {
            edges[0]->SetTwin(edges[1]);
            edges[1]->SetTwin(edges[0]);
        }
    }
}
```

**关键点：**
- 使用map识别具有相同端点的边
- 标准化边的方向（字典序）以确保一致性
- 为共享边设置twin关系

### 4. 提取Voronoi图

```cpp
VoronoiDiagramResult ConvertDCELToResult(const DCEL* dcel, const std::vector<Point2D>& sites) {
    VoronoiDiagramResult result;
    result.sites = sites;
    
    std::set<Point2D> vertex_set;
    std::set<Edge2D> edge_set;
    
    // 从有效的Face中提取顶点和边
    for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
        Face* face = dcel->GetFace(i);
        if (face == nullptr || face->IsUnbounded()) continue;
        
        auto boundary_vertices = face->GetOuterBoundaryVertices();
        if (boundary_vertices.size() < 3) continue;
        
        // 收集顶点
        for (auto* v : boundary_vertices) {
            if (v) vertex_set.insert(v->GetCoordinates());
        }
        
        // 收集边
        for (size_t j = 0; j < boundary_vertices.size(); ++j) {
            Vertex* v1 = boundary_vertices[j];
            Vertex* v2 = boundary_vertices[(j + 1) % boundary_vertices.size()];
            if (v1 && v2) edge_set.emplace(v1->GetCoordinates(), v2->GetCoordinates());
        }
    }
    
    // 转换为vector
    for (const auto& v : vertex_set) result.vertices.push_back(v);
    for (const auto& e : edge_set) result.edges.push_back(e);
    
    // 收集cells
    for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
        Face* face = dcel->GetFace(i);
        if (face && !face->IsUnbounded()) {
            VoronoiCell cell;
            auto boundary_vertices = face->GetOuterBoundaryVertices();
            
            for (auto* v : boundary_vertices) {
                cell.vertices.push_back(v->GetCoordinates());
            }
            
            for (size_t j = 0; j < cell.vertices.size(); ++j) {
                size_t next = (j + 1) % cell.vertices.size();
                cell.edges.emplace_back(cell.vertices[j], cell.vertices[next]);
            }
            
            result.cells.push_back(cell);
        }
    }
    
    return result;
}
```

**关键点：**
- 只从有效的Face中提取数据
- 使用set去重
- 分别收集顶点、边和cell

---

## 时间复杂度分析

### 算法步骤的时间复杂度

1. **创建初始cell**：O(n)
   - 为n个site创建n个bounding box cell
   - 每个cell创建4个顶点和4条边

2. **裁剪cell**：O(n²)
   - 外层循环：n个cell
   - 内层循环：每个cell需要与其他n-1个site比较
   - 每次裁剪操作：O(m)，其中m是当前cell的顶点数
   - 总时间：O(n² × m)

3. **合并共享边**：O(n²)
   - 遍历所有半边：O(n²)
   - 每条边插入map：O(log n)
   - 总时间：O(n² log n)

4. **提取结果**：O(n²)
   - 遍历所有Face：O(n)
   - 每个Face的顶点数：O(n)
   - 总时间：O(n²)

### 总时间复杂度

**最坏情况**：O(n²)
- 裁剪步骤占主导地位
- 每个cell可能需要O(n)次裁剪
- 每次裁剪可能产生O(n)个新顶点

**平均情况**：O(n²)
- 大多数情况下，每个cell的顶点数不会超过O(n)
- 裁剪操作的时间复杂度为O(m)，其中m是顶点数

**最好情况**：O(n²)
- 即使所有点共线，仍然需要进行O(n²)次比较

---

## 空间复杂度分析

### DCEL结构的存储需求

1. **顶点**：O(n²)
   - 每次裁剪可能产生新的顶点
   - 最坏情况下，每个cell有O(n)个顶点
   - 总顶点数：O(n²)

2. **半边**：O(n²)
   - 每条无向边由2条半边表示
   - 边数与顶点数同阶
   - 总半边数：O(n²)

3. **面**：O(n)
   - 每个site对应一个cell
   - 总面数：O(n)

### 总空间复杂度

**O(n²)**
- 顶点和边占主导地位
- DCEL结构需要存储完整的拓扑信息

---

## 优缺点分析

### 优点

1. **维护拓扑关系**
   - 保存了完整的邻接信息
   - 支持高效的邻接查询
   - 可以快速访问相邻的cell、边和顶点

2. **可扩展性强**
   - 可以支持动态添加/删除site
   - 可以扩展到其他平面细分问题
   - 可以支持带洞的多边形

3. **算法清晰**
   - 增量式方法易于理解
   - 每个步骤都有明确的几何意义
   - 便于调试和验证

4. **支持高级操作**
   - 可以进行点定位查询
   - 可以计算cell的面积和周长
   - 可以进行路径规划等操作

### 缺点

1. **实现复杂**
   - 需要维护复杂的指针关系
   - 容易出现指针错误
   - 调试困难

2. **空间开销大**
   - 需要存储O(n²)个顶点和边
   - 每个元素需要额外的指针信息
   - 内存占用较高

3. **时间复杂度不够优**
   - O(n²)的时间复杂度
   - 对于大规模数据集不够高效
   - 存在更快的算法（如Fortune算法：O(n log n)）

4. **数值稳定性问题**
   - 半平面裁剪涉及浮点运算
   - 可能出现数值精度问题
   - 需要谨慎处理边界情况

---

## 代码示例

### 完整的DCEL Voronoi算法实现

```cpp
DCEL* DCELVoronoi::GenerateDCEL(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) {
    
    DCEL* dcel = new DCEL();
    
    if (sites.empty()) return dcel;
    if (sites.size() == 1) {
        CreateBoundingBoxCell(dcel, sites[0], bounds);
        return dcel;
    }
    
    // 步骤1：为每个site创建初始cell
    std::vector<Face*> cells;
    for (const auto& site : sites) {
        Face* cell = CreateBoundingBoxCell(dcel, site, bounds);
        cells.push_back(cell);
    }
    
    // 步骤2：裁剪每个cell
    for (size_t i = 0; i < sites.size(); ++i) {
        for (size_t j = 0; j < sites.size(); ++j) {
            if (i == j) continue;
            
            // 计算垂直平分线
            Point2D midpoint;
            Vector2D normal;
            HalfPlaneClipper::ComputeBisector(sites[i], sites[j], midpoint, normal);
            
            // 获取当前顶点
            auto boundary_vertices = cells[i]->GetOuterBoundaryVertices();
            std::vector<Point2D> polygon;
            for (auto* v : boundary_vertices) {
                polygon.push_back(v->GetCoordinates());
            }
            
            // 裁剪
            std::vector<Point2D> clipped = 
                HalfPlaneClipper::ClipPolygon(polygon, midpoint, normal);
            
            // 更新DCEL
            if (clipped.size() >= 3) {
                std::vector<Vertex*> new_vertices;
                std::vector<HalfEdge*> new_edges;
                
                for (const auto& p : clipped) {
                    new_vertices.push_back(dcel->CreateVertex(p));
                }
                
                for (size_t k = 0; k < new_vertices.size(); ++k) {
                    Vertex* v1 = new_vertices[k];
                    Vertex* v2 = new_vertices[(k + 1) % new_vertices.size()];
                    HalfEdge* he = dcel->CreateEdge(v1, v2);
                    new_edges.push_back(he);
                }
                
                for (size_t k = 0; k < new_edges.size(); ++k) {
                    HalfEdge* he = new_edges[k];
                    HalfEdge* next_he = new_edges[(k + 1) % new_edges.size()];
                    dcel->ConnectHalfEdges(he, next_he);
                }
                
                dcel->SetFaceOfCycle(new_edges[0], cells[i]);
                cells[i]->SetOuterComponent(new_edges[0]);
            }
        }
    }
    
    // 步骤3：合并共享边
    MergeSharedEdges(dcel);
    
    return dcel;
}
```

---

## 与其他算法对比

### 1. 与简单增量算法对比

| 特性 | 简单增量算法 | DCEL增量算法 |
|------|-------------|-------------|
| 时间复杂度 | O(n²)~O(n³) | O(n²) |
| 空间复杂度 | O(n²) | O(n²) |
| 拓扑信息 | ❌ 不维护 | ✅ 维护 |
| 邻接查询 | ❌ 不支持 | ✅ 支持 |
| 实现难度 | 简单 | 复杂 |
| 可扩展性 | 一般 | 强 |

### 2. 与Fortune算法对比

| 特性 | Fortune算法 | DCEL增量算法 |
|------|------------|-------------|
| 时间复杂度 | O(n log n) | O(n²) |
| 空间复杂度 | O(n) | O(n²) |
| 拓扑信息 | ✅ 维护 | ✅ 维护 |
| 实现难度 | 非常复杂 | 复杂 |
| 适用场景 | 大规模数据 | 中小规模数据 |
| 动态更新 | ❌ 不支持 | ✅ 支持 |

### 3. 算法选择建议

**选择DCEL增量算法的场景：**
- 需要维护拓扑关系
- 需要支持动态更新
- 数据规模中等（n < 10000）
- 需要进行邻接查询
- 需要扩展到其他平面细分问题

**选择其他算法的场景：**
- 大规模数据集（n > 10000）→ Fortune算法
- 不需要拓扑信息 → 简单增量算法
- 只需要一次性计算 → Fortune算法
- 内存受限 → Fortune算法

---

## 总结

DCEL Voronoi算法是一种功能强大的增量式算法，通过维护完整的拓扑关系信息，支持高效的邻接查询和动态更新。虽然实现复杂且时间复杂度为O(n²)，但对于中等规模的数据集和需要拓扑信息的应用场景，它是一个很好的选择。

**关键要点：**
1. 理解DCEL数据结构的三个核心组件：Vertex、Half-edge、Face
2. 掌握半平面裁剪的原理和实现
3. 注意维护`outer_component_`指针的正确性
4. 理解算法的时间复杂度和空间复杂度
5. 根据应用场景选择合适的算法

**进一步学习：**
- Fortune扫描线算法（O(n log n)）
- Divide and Conquer算法（O(n log n)）
- 动态Voronoi图
- 加权Voronoi图
- 高阶Voronoi图

---

## 参考资料

1. **Computational Geometry: Algorithms and Applications**  
   by Mark de Berg, Otfried Cheong, Marc van Kreveld, Mark Overmars

2. **Voronoi Diagrams**  
   by Franz Aurenhammer, Rolf Klein

3. **Computational Geometry in C**  
   by Joseph O'Rourke

4. **Halfplane Intersection**  
   CGAL Documentation: https://doc.cgal.org/

5. **DCEL Data Structure**  
   Wikipedia: https://en.wikipedia.org/wiki/Doubly_connected_edge_list
