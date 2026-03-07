# 耳切法三角剖分算法

## 1. 算法概述

耳切法（Ear Clipping）是一种简单直观的多边形三角剖分算法，适用于任意简单多边形（不自交的多边形）。算法的核心思想是不断寻找并"剪掉"多边形中的"耳朵"，直到多边形被完全剖分为三角形。

### 1.1 基本概念

**耳朵（Ear）**：对于一个简单多边形，如果三个连续顶点 $(v_{i-1}, v_i, v_{i+1})$ 满足以下条件，则称顶点 $v_i$ 是一个"耳朵"：

1. **凸性条件**：$(v_{i-1}, v_i, v_{i+1})$ 构成一个凸角（内角 < 180°）
2. **内部条件**：三角形 $(v_{i-1}, v_i, v_{i+1})$ 内部不包含多边形的任何其他顶点
3. **对角线条件**：对角线 $(v_{i-1}, v_{i+1})$ 完全在多边形内部

### 1.2 算法特点

| 特性 | 说明 |
|------|------|
| **时间复杂度** | O(n²) - 外层循环 O(n)，每次查找耳朵 O(n) |
| **空间复杂度** | O(n) - 存储顶点索引和结果 |
| **适用范围** | 任意简单多边形（不自交） |
| **优点** | 实现简单，鲁棒性好，易于理解 |
| **缺点** | 效率较低，不适合大规模多边形 |

## 2. 算法流程

### 2.1 总体流程

```
输入：简单多边形 P = {v₀, v₁, ..., v_{n-1}}
输出：三角形集合

步骤 1：预处理
├─ 确保多边形顶点按逆时针（CCW）顺序排列
└─ 移除共线顶点（避免退化情况）

步骤 2：主循环（当顶点数 > 3 时）
├─ 遍历每个顶点 vᵢ
│   ├─ 获取前一个顶点 v_{i-1} 和后一个顶点 v_{i+1}
│   ├─ 检查 (v_{i-1}, vᵢ, v_{i+1}) 是否构成凸角
│   │   └─ 使用叉积：Cross(v_{i-1}, vᵢ, v_{i+1}) > 0
│   ├─ 检查三角形内是否包含其他顶点
│   │   └─ 对每个其他顶点 vⱼ，检查是否严格在三角形内
│   └─ 如果都满足，则 vᵢ 是一个耳朵
│       ├─ 将三角形 (v_{i-1}, vᵢ, v_{i+1}) 加入结果
│       ├─ 从多边形中移除顶点 vᵢ
│       └─ 跳出当前循环，重新开始寻找
└─ 如果找不到耳朵，说明多边形无效，退出

步骤 3：处理最后三个顶点
└─ 将剩余的三个顶点构成的三角形加入结果

步骤 4：返回所有三角形
```

### 2.2 详细步骤

#### 步骤 1：预处理

```cpp
// 1. 确保 CCW 顺序
if (!IsCCW(polygon)) {
    reverse(polygon.begin(), polygon.end());
}

// 2. 移除共线顶点
RemoveCollinear(polygon);
```

**为什么要确保 CCW 顺序？**
- 凸性检查依赖于顶点的顺序
- 对于 CCW 多边形，凸角的叉积为正
- 统一的顺序可以简化判断逻辑

**为什么要移除共线顶点？**
- 共线顶点会导致退化三角形（面积为0）
- 可能引起数值不稳定
- 移除后可以简化后续处理

#### 步骤 2：主循环

```cpp
while (vertices.size() > 3) {
    bool ear_found = false;
    
    for (size_t i = 0; i < vertices.size(); ++i) {
        // 获取三个连续顶点
        size_t prev_idx = (i + vertices.size() - 1) % vertices.size();
        size_t curr_idx = i;
        size_t next_idx = (i + 1) % vertices.size();
        
        const Point2D& p_prev = polygon[vertices[prev_idx]];
        const Point2D& p_curr = polygon[vertices[curr_idx]];
        const Point2D& p_next = polygon[vertices[next_idx]];
        
        // 检查凸性
        if (!IsConvex(p_prev, p_curr, p_next)) {
            continue;
        }
        
        // 检查三角形内是否包含其他顶点
        bool contains = false;
        for (size_t v : vertices) {
            if (v == vertices[prev_idx] || v == vertices[curr_idx] || v == vertices[next_idx]) {
                continue;
            }
            if (PointStrictlyInTriangle(polygon[v], p_prev, p_curr, p_next)) {
                contains = true;
                break;
            }
        }
        
        if (contains) {
            continue;
        }
        
        // 找到耳朵，剪掉它
        result.triangles.emplace_back(p_prev, p_curr, p_next);
        vertices.erase(vertices.begin() + curr_idx);
        
        ear_found = true;
        break;  // 重新开始寻找
    }
    
    if (!ear_found) {
        break;  // 找不到耳朵，多边形可能无效
    }
}
```

#### 步骤 3：处理最后三个顶点

```cpp
if (vertices.size() == 3) {
    const Point2D& p0 = polygon[vertices[0]];
    const Point2D& p1 = polygon[vertices[1]];
    const Point2D& p2 = polygon[vertices[2]];
    
    result.triangles.emplace_back(p0, p1, p2);
}
```

## 3. 关键判断函数

### 3.1 凸性检查（IsConvex）

判断三个点是否构成凸角（对于 CCW 多边形）。

```cpp
bool IsConvex(const Point2D& prev, const Point2D& curr, const Point2D& next) {
    return Cross(prev, curr, next) > 0;
}
```

**原理**：
- 使用叉积判断转向
- 对于 CCW 多边形，凸角的叉积为正
- 叉积公式：$(curr - prev) × (next - curr)$

**叉积计算**：
```cpp
double Cross(const Point2D& a, const Point2D& b, const Point2D& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}
```

### 3.2 点在三角形内判断（PointStrictlyInTriangle）

判断一个点是否严格在三角形内部（不包括边界）。

```cpp
bool PointStrictlyInTriangle(
    const Point2D& p,
    const Point2D& a,
    const Point2D& b,
    const Point2D& c) {
    
    double d1 = Cross(p, a, b);
    double d2 = Cross(p, b, c);
    double d3 = Cross(p, c, a);
    
    return (d1 > 0 && d2 > 0 && d3 > 0);
}
```

**原理**：
- 使用同侧法：点 p 在三角形内当且仅当它在三条边的同一侧
- 对于 CCW 三角形，内部点对所有边的叉积都为正
- 严格在内部要求所有叉积都 > 0（不包括边界）

**为什么使用严格判断？**
- 耳朵的定义要求三角形内部不包含其他顶点
- 边界上的顶点不影响耳朵的判定
- 严格判断可以避免误判

### 3.3 共线点移除（RemoveCollinear）

移除多边形中的共线顶点。

```cpp
void RemoveCollinear(std::vector<Point2D>& poly) {
    if (poly.size() < 3) return;
    
    std::vector<Point2D> result;
    
    for (size_t i = 0; i < poly.size(); ++i) {
        const Point2D& prev = poly[(i + poly.size() - 1) % poly.size()];
        const Point2D& curr = poly[i];
        const Point2D& next = poly[(i + 1) % poly.size()];
        
        // 如果叉积接近0，说明三点共线
        if (std::abs(Cross(prev, curr, next)) > 1e-12) {
            result.push_back(curr);
        }
    }
    
    poly.swap(result);
}
```

**原理**：
- 如果三个点共线，它们的叉积为0
- 使用小的容差值（1e-12）来判断是否共线
- 只保留非共线的顶点

### 3.4 CCW 顺序判断（IsCCW）

判断多边形顶点是否按逆时针顺序排列。

```cpp
bool IsCCW(const std::vector<Point2D>& polygon) {
    double area = 0;
    
    for (size_t i = 0; i < polygon.size(); ++i) {
        const Point2D& p1 = polygon[i];
        const Point2D& p2 = polygon[(i + 1) % polygon.size()];
        
        area += (p2.x - p1.x) * (p2.y + p1.y);
    }
    
    return area < 0;  // 屏幕坐标系下，area < 0 表示 CCW
}
```

**原理**：
- 使用有向面积判断
- 屏幕坐标系（Y轴向下）下，area < 0 表示逆时针
- 数学坐标系（Y轴向上）下，area > 0 表示逆时针

## 4. 算法示例

### 4.1 图示示例

```
初始多边形（逆时针）：
    v₀ ───── v₁
    │         │
    │    v₃   │
    │   / \   │
    │  /   \  │
    v₂       v₄

第1步：找到耳朵 v₃
  - 检查 (v₂, v₃, v₄)：凸角 ✓
  - 三角形内无其他顶点 ✓
  - 剪掉 v₃，得到三角形 (v₂, v₃, v₄)

第2步：多边形变为 {v₀, v₁, v₂, v₄}
  - 找到耳朵 v₂
  - 剪掉 v₂，得到三角形 (v₁, v₂, v₄)

第3步：多边形变为 {v₀, v₁, v₄}
  - 只剩3个顶点，得到三角形 (v₀, v₁, v₄)

结果：3个三角形完成剖分
```

### 4.2 代码示例

```cpp
// 完整的耳切法实现
TriangulationResult EarClippingTriangulation(
    const std::vector<Point2D>& polygon) {
    
    TriangulationResult result;
    
    if (polygon.size() < 3) {
        return result;
    }
    
    // 步骤 1：预处理
    std::vector<Point2D> poly = polygon;
    
    if (!IsCCW(poly)) {
        std::reverse(poly.begin(), poly.end());
    }
    
    RemoveCollinear(poly);
    
    // 创建顶点索引
    std::vector<size_t> vertices;
    for (size_t i = 0; i < poly.size(); ++i) {
        vertices.push_back(i);
    }
    
    // 步骤 2：主循环
    int guard = 0;
    while (vertices.size() > 3 && guard < 10000) {
        bool ear_found = false;
        
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t prev_idx = (i + vertices.size() - 1) % vertices.size();
            size_t curr_idx = i;
            size_t next_idx = (i + 1) % vertices.size();
            
            const Point2D& p_prev = poly[vertices[prev_idx]];
            const Point2D& p_curr = poly[vertices[curr_idx]];
            const Point2D& p_next = poly[vertices[next_idx]];
            
            // 检查凸性
            if (!IsConvex(p_prev, p_curr, p_next)) {
                continue;
            }
            
            // 检查三角形内是否包含其他顶点
            bool contains = false;
            for (size_t v : vertices) {
                if (v == vertices[prev_idx] || 
                    v == vertices[curr_idx] || 
                    v == vertices[next_idx]) {
                    continue;
                }
                
                if (PointStrictlyInTriangle(
                        poly[v], p_prev, p_curr, p_next)) {
                    contains = true;
                    break;
                }
            }
            
            if (contains) {
                continue;
            }
            
            // 找到耳朵，剪掉它
            result.triangles.emplace_back(p_prev, p_curr, p_next);
            result.edges.emplace_back(p_prev, p_curr);
            result.edges.emplace_back(p_curr, p_next);
            result.edges.emplace_back(p_next, p_prev);
            
            vertices.erase(vertices.begin() + curr_idx);
            
            ear_found = true;
            break;
        }
        
        if (!ear_found) {
            break;
        }
        
        guard++;
    }
    
    // 步骤 3：处理最后三个顶点
    if (vertices.size() == 3) {
        const Point2D& p0 = poly[vertices[0]];
        const Point2D& p1 = poly[vertices[1]];
        const Point2D& p2 = poly[vertices[2]];
        
        result.triangles.emplace_back(p0, p1, p2);
        result.edges.emplace_back(p0, p1);
        result.edges.emplace_back(p1, p2);
        result.edges.emplace_back(p2, p0);
    }
    
    return result;
}
```

## 5. 算法优化

### 5.1 优化思路

1. **耳朵列表维护**：预先计算所有可能的耳朵，维护一个耳朵列表
2. **局部更新**：剪掉耳朵后，只更新受影响的顶点的耳朵状态
3. **空间索引**：使用空间索引结构加速点在三角形内的判断

### 5.2 优化后的复杂度

- **时间复杂度**：可以优化到 O(n log n)
- **空间复杂度**：O(n)

## 6. 与其他算法比较

| 算法 | 时间复杂度 | 适用范围 | 实现难度 |
|------|-----------|---------|---------|
| 耳切法 | O(n²) | 任意简单多边形 | 简单 |
| 扫描线法 | O(n log n) | Y单调多边形 | 中等 |
| Delaunay三角剖分 | O(n log n) | 点集 | 复杂 |

## 7. 注意事项

1. **多边形有效性**：算法假设输入是简单多边形（不自交）
2. **数值稳定性**：使用适当的容差值（如 1e-12）处理浮点误差
3. **退化情况**：共线顶点需要在预处理阶段移除
4. **无限循环防护**：添加 guard 计数器防止异常情况

## 8. 参考资料

1. "Computational Geometry: Algorithms and Applications" - de Berg et al.
2. "Triangulation of Simple Polygons" - ElGindy et al.
3. "Ear Clipping for Polygon Triangulation" - Eric Haines
