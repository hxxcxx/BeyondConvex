# Divide & Conquer Voronoi 算法详解

## 目录

1. [算法概述](#算法概述)
2. [算法原理](#算法原理)
3. [实现细节](#实现细节)
4. [时间复杂度分析](#时间复杂度分析)
5. [空间复杂度分析](#空间复杂度分析)
6. [优缺点分析](#优缺点分析)
7. [代码示例](#代码示例)
8. [与其他算法对比](#与其他算法对比)

---

## 算法概述

### 什么是分治法？

分治法（Divide and Conquer）是一种经典的算法设计范式：
- **Divide（分）**：将问题分解为若干个较小的子问题
- **Conquer（治）**：递归地解决这些子问题
- **Combine（合）**：将子问题的解合并成原问题的解

### Divide & Conquer Voronoi算法

Divide & Conquer Voronoi算法是一种高效的Voronoi图生成算法，通过递归地将点集分成两半，分别计算Voronoi图，然后合并结果。

**算法特点：**
- 时间复杂度：O(n log n)
- 空间复杂度：O(n)
- 递归实现
- 适合大规模数据集

---

## 算法原理

### 核心思想

```
DivideAndConquer(sites):
    if |sites| <= 3:
        return BruteForce(sites)
    
    mid = |sites| / 2
    left_sites = sites[0:mid]
    right_sites = sites[mid:end]
    
    left_voronoi = DivideAndConquer(left_sites)
    right_voronoi = DivideAndConquer(right_sites)
    
    return Merge(left_voronoi, right_voronoi)
```

### 算法步骤

#### 1. 排序（预处理）

首先将所有site按x坐标排序：
```cpp
std::sort(sites.begin(), sites.end(),
    [](const Point2D& a, const Point2D& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });
```

**目的：**
- 确保分割线是垂直的
- 简化合并过程
- 保证递归的平衡性

#### 2. 分割（Divide）

将排序后的site集分成两半：
```cpp
size_t mid = n / 2;
std::vector<Point2D> left_sites(sites.begin(), sites.begin() + mid);
std::vector<Point2D> right_sites(sites.begin() + mid, sites.end());
```

**特点：**
- 左半部分：sites[0 : n/2]
- 右半部分：sites[n/2 : n]
- 分割线是垂直的，x = (sites[mid-1].x + sites[mid].x) / 2

#### 3. 递归求解（Conquer）

递归地计算左右两半的Voronoi图：
```cpp
DCEL* left_dcel = DivideAndConquer(left_sites, bounds);
DCEL* right_dcel = DivideAndConquer(right_sites, bounds);
```

**基准情况（Base Case）：**
- 当site数量 ≤ 3时，使用增量算法直接计算
- 避免过深的递归
- 简化小规模问题的处理

#### 4. 合并（Merge）

合并两个子Voronoi图是最复杂的步骤：

**合并过程：**
1. 计算分割曲线（dividing curve）
2. 用分割曲线裁剪左右两边的cell
3. 合并共享边
4. 构建最终的Voronoi图

### 算法流程图

```
开始
  ↓
排序sites（按x坐标）
  ↓
DivideAndConquer(sites)
  ↓
if |sites| <= 3:
  使用增量算法
  ↓
else:
  分割成left_sites和right_sites
  ↓
  left_voronoi = DivideAndConquer(left_sites)
  right_voronoi = DivideAndConquer(right_sites)
  ↓
  merged = Merge(left_voronoi, right_voronoi)
  ↓
返回结果
```

---

## 实现细节

### 1. 主函数

```cpp
DCEL* DivideConquerVoronoi::GenerateDCEL(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) {
    
    DCEL* dcel = new DCEL();
    
    if (sites.empty()) return dcel;
    if (sites.size() == 1) {
        // 单点情况：使用增量算法
        // ...
        return dcel;
    }
    
    // 排序
    std::vector<Point2D> sorted_sites = sites;
    std::sort(sorted_sites.begin(), sorted_sites.end(),
        [](const Point2D& a, const Point2D& b) {
            return a.x < b.x || (a.x == b.x && a.y < b.y);
        });
    
    // 分治
    dcel = DivideAndConquer(sorted_sites, bounds);
    
    return dcel;
}
```

### 2. 递归函数

```cpp
DCEL* DivideConquerVoronoi::DivideAndConquer(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) const {
    
    size_t n = sites.size();
    
    // 基准情况
    if (n <= 3) {
        // 使用增量算法
        IncrementalVoronoi incremental;
        VoronoiDiagramResult result = incremental.Generate(sites, bounds);
        
        // 转换为DCEL
        // ...
        return dcel;
    }
    
    // 分割
    size_t mid = n / 2;
    std::vector<Point2D> left_sites(sites.begin(), sites.begin() + mid);
    std::vector<Point2D> right_sites(sites.begin() + mid, sites.end());
    
    // 递归
    DCEL* left_dcel = DivideAndConquer(left_sites, bounds);
    DCEL* right_dcel = DivideAndConquer(right_sites, bounds);
    
    // 合并
    DCEL* merged_dcel = MergeDiagrams(
        left_dcel, right_dcel, 
        left_sites, right_sites, 
        bounds);
    
    // 清理
    delete left_dcel;
    delete right_dcel;
    
    return merged_dcel;
}
```

### 3. 合并函数

```cpp
DCEL* DivideConquerVoronoi::MergeDiagrams(
    DCEL* left_dcel,
    DCEL* right_dcel,
    const std::vector<Point2D>& left_sites,
    const std::vector<Point2D>& right_sites,
    const VoronoiBounds& bounds) const {
    
    DCEL* merged_dcel = new DCEL();
    
    // 1. 计算分割曲线
    std::vector<Point2D> dividing_curve = 
        ComputeDividingCurve(left_sites, right_sites, bounds);
    
    // 2. 复制左边的faces
    for (size_t i = 0; i < left_dcel->GetFaceCount(); ++i) {
        Face* face = left_dcel->GetFace(i);
        // 复制顶点和边
        // ...
    }
    
    // 3. 复制右边的faces
    for (size_t i = 0; i < right_dcel->GetFaceCount(); ++i) {
        Face* face = right_dcel->GetFace(i);
        // 复制顶点和边
        // ...
    }
    
    // 4. 用分割曲线裁剪faces
    ClipFacesByDividingCurve(
        merged_dcel, dividing_curve, 
        left_sites, right_sites);
    
    // 5. 合并共享边
    // ...
    
    return merged_dcel;
}
```

### 4. 计算分割曲线

```cpp
std::vector<Point2D> DivideConquerVoronoi::ComputeDividingCurve(
    const std::vector<Point2D>& left_sites,
    const std::vector<Point2D>& right_sites,
    const VoronoiBounds& bounds) const {
    
    // 简化版本：计算中位点的垂直平分线
    Point2D left_median = left_sites[left_sites.size() / 2];
    Point2D right_median = right_sites[right_sites.size() / 2];
    
    Point2D midpoint;
    Vector2D normal;
    HalfPlaneClipper::ComputeBisector(
        left_median, right_median, midpoint, normal);
    
    std::vector<Point2D> curve;
    curve.push_back(midpoint);
    
    return curve;
}
```

### 5. 裁剪Faces

```cpp
void DivideConquerVoronoi::ClipFacesByDividingCurve(
    DCEL* dcel,
    const std::vector<Point2D>& dividing_curve,
    const std::vector<Point2D>& left_sites,
    const std::vector<Point2D>& right_sites) const {
    
    // 对每个face，用对侧的sites进行裁剪
    for (size_t i = 0; i < dcel->GetFaceCount(); ++i) {
        Face* face = dcel->GetFace(i);
        if (face == nullptr || face->IsUnbounded()) continue;
        
        // 确定这个face属于哪个site
        // ...
        
        // 用对侧的sites裁剪
        for (const auto& site : opposite_sites) {
            // 计算垂直平分线
            // 裁剪face
            // 更新DCEL
        }
    }
}
```

---

## 时间复杂度分析

### 递归关系

设T(n)为处理n个site的时间复杂度：

```
T(n) = 2T(n/2) + O(n)
```

**解释：**
- `2T(n/2)`：递归处理左右两半
- `O(n)`：合并两个子图

### 主定理求解

根据主定理（Master Theorem）：
```
T(n) = aT(n/b) + f(n)
其中：a = 2, b = 2, f(n) = O(n)

计算 n^(log_b(a)) = n^(log_2(2)) = n

由于 f(n) = O(n) = O(n^(log_b(a)))
根据主定理情况2：
T(n) = O(n^(log_b(a)) * log n) = O(n log n)
```

### 各步骤的时间复杂度

1. **排序**：O(n log n)
   - 只在开始时排序一次

2. **递归分割**：O(n log n)
   - 每层递归：O(n)
   - 递归深度：O(log n)
   - 总时间：O(n log n)

3. **合并**：O(n)
   - 复制faces：O(n)
   - 裁剪faces：O(n)
   - 合并共享边：O(n)

### 总时间复杂度

**O(n log n)**

这是Voronoi图算法的最优时间复杂度（比较模型下）。

---

## 空间复杂度分析

### 递归栈空间

- 递归深度：O(log n)
- 每层需要存储：O(n)
- 总空间：O(n log n)

### DCEL存储空间

- 顶点：O(n)
- 边：O(n)
- 面：O(n)
- 总空间：O(n)

### 总空间复杂度

**O(n)**（不包括递归栈）
**O(n log n)**（包括递归栈）

---

## 优缺点分析

### 优点

1. **最优时间复杂度**
   - O(n log n)是最优的（比较模型下）
   - 比增量算法O(n²)更快
   - 适合大规模数据集

2. **良好的缓存性能**
   - 分治法具有良好的局部性
   - 缓存命中率高
   - 实际运行速度快

3. **可并行化**
   - 左右两半可以并行计算
   - 适合多核处理器
   - 可扩展性强

4. **实现相对简单**
   - 比Fortune算法简单
   - 递归结构清晰
   - 易于理解和调试

### 缺点

1. **需要排序**
   - 预处理需要O(n log n)时间
   - 不能利用输入的顺序
   - 动态更新困难

2. **合并复杂**
   - 合并步骤实现复杂
   - 容易出现边界情况
   - 需要仔细处理

3. **递归开销**
   - 递归调用有开销
   - 栈空间占用
   - 可能导致栈溢出（极深递归）

4. **不适合动态更新**
   - 添加/删除site需要重新计算
   - 不支持增量更新
   - 不适合动态场景

---

## 代码示例

### 完整实现

```cpp
DCEL* DivideConquerVoronoi::DivideAndConquer(
    const std::vector<Point2D>& sites,
    const VoronoiBounds& bounds) const {
    
    DCEL* dcel = new DCEL();
    size_t n = sites.size();
    
    // 基准情况
    if (n <= 3) {
        IncrementalVoronoi incremental;
        VoronoiDiagramResult result = incremental.Generate(sites, bounds);
        
        // 转换为DCEL
        std::map<Point2D, Vertex*> vertex_map;
        for (const auto& p : result.vertices) {
            vertex_map[p] = dcel->CreateVertex(p);
        }
        
        for (const auto& cell : result.cells) {
            if (!cell.IsValid()) continue;
            
            std::vector<Vertex*> cell_vertices;
            for (const auto& p : cell.vertices) {
                if (vertex_map.find(p) == vertex_map.end()) {
                    vertex_map[p] = dcel->CreateVertex(p);
                }
                cell_vertices.push_back(vertex_map[p]);
            }
            
            if (cell_vertices.size() >= 3) {
                std::vector<HalfEdge*> edges;
                for (size_t i = 0; i < cell_vertices.size(); ++i) {
                    Vertex* v1 = cell_vertices[i];
                    Vertex* v2 = cell_vertices[(i + 1) % cell_vertices.size()];
                    HalfEdge* he = dcel->CreateEdge(v1, v2);
                    edges.push_back(he);
                }
                
                for (size_t i = 0; i < edges.size(); ++i) {
                    HalfEdge* he = edges[i];
                    HalfEdge* next_he = edges[(i + 1) % edges.size()];
                    dcel->ConnectHalfEdges(he, next_he);
                }
                
                Face* face = dcel->CreateFace();
                dcel->SetFaceOfCycle(edges[0], face);
                face->SetOuterComponent(edges[0]);
            }
        }
        
        return dcel;
    }
    
    // 分割
    size_t mid = n / 2;
    std::vector<Point2D> left_sites(sites.begin(), sites.begin() + mid);
    std::vector<Point2D> right_sites(sites.begin() + mid, sites.end());
    
    // 递归
    DCEL* left_dcel = DivideAndConquer(left_sites, bounds);
    DCEL* right_dcel = DivideAndConquer(right_sites, bounds);
    
    // 合并
    dcel = MergeDiagrams(left_dcel, right_dcel, left_sites, right_sites, bounds);
    
    // 清理
    delete left_dcel;
    delete right_dcel;
    
    return dcel;
}
```

---

## 与其他算法对比

### 1. 与增量算法对比

| 特性 | 增量算法 | 分治算法 |
|------|---------|---------|
| 时间复杂度 | O(n²) | O(n log n) |
| 空间复杂度 | O(n²) | O(n) |
| 是否需要排序 | ❌ | ✅ |
| 动态更新 | ✅ 支持 | ❌ 不支持 |
| 实现难度 | 简单 | 中等 |
| 大规模数据 | 慢 | 快 |

### 2. 与Fortune算法对比

| 特性 | Fortune算法 | 分治算法 |
|------|------------|---------|
| 时间复杂度 | O(n log n) | O(n log n) |
| 空间复杂度 | O(n) | O(n) |
| 是否需要排序 | ❌ | ✅ |
| 实现难度 | 非常复杂 | 中等 |
| 稳定性 | 高 | 中等 |
| 实际性能 | 最快 | 快 |

### 3. 算法选择建议

**选择分治算法的场景：**
- 大规模静态数据集（n > 1000）
- 需要最优时间复杂度
- 可以接受预处理排序
- 不需要动态更新
- 实现复杂度要求适中

**选择其他算法的场景：**
- 需要动态更新 → 增量算法
- 极大规模数据（n > 100000）→ Fortune算法
- 小规模数据（n < 100）→ 增量算法
- 实现简单优先 → 增量算法

---

## 总结

Divide & Conquer Voronoi算法是一种高效的Voronoi图生成算法，通过递归地将点集分成两半，分别计算Voronoi图，然后合并结果。它具有O(n log n)的最优时间复杂度，适合大规模静态数据集。

**关键要点：**
1. 理解分治法的三个步骤：Divide、Conquer、Combine
2. 掌握递归实现和基准情况处理
3. 理解合并步骤的复杂性
4. 了解算法的时间复杂度和空间复杂度
5. 根据应用场景选择合适的算法

**进一步优化：**
- 使用迭代代替递归（避免栈溢出）
- 优化合并步骤（减少裁剪操作）
- 并行化递归调用（利用多核）
- 使用更高效的分割曲线计算方法

**参考资料：**
1. **Computational Geometry: Algorithms and Applications**  
   by Mark de Berg, Otfried Cheong, Marc van Kreveld, Mark Overmars

2. **Voronoi Diagrams**  
   by Franz Aurenhammer, Rolf Klein

3. **Divide and Conquer Algorithms**  
   Wikipedia: https://en.wikipedia.org/wiki/Divide-and-conquer_algorithm
