# 扫描线三角剖分算法

## 概述

扫描线三角剖分算法是一种用于剖分 y-单调多边形的方法，通过从上到下处理顶点并使用基于栈的方法来实现。

## 算法要求

### Y-单调多边形

如果一个多边形可以分解为两条单调链（左链和右链），且每条链在 y 方向上都是单调的，则该多边形是 **y-单调的**。

**性质：**
- 多边形可以分成左链和右链
- 每条链的顶点 y 坐标严格递减（或递增）
- 水平线与多边形最多相交两次

**示例：**
```
    顶部
     /\
    /  \
   /    \
  /      \
 /        \
底部----底部
```

### 为什么需要 Y-单调？

扫描线算法依赖于 y-单调性质来：
1. 正确识别左链和右链
2. 在扫描过程中保持正确的顶点顺序
3. 形成有效的三角形而不产生交叉边

**对于非 y-单调多边形**，算法可能会：
- 错误地识别链的归属
- 创建无效的对角线
- 生成在多边形外部的三角形
- 产生相交的边

## 算法步骤

### 步骤 1：按 Y 坐标排序顶点

```cpp
std::vector<size_t> order(n);
for (size_t i = 0; i < n; ++i)
    order[i] = i;

std::sort(order.begin(), order.end(),
    [&](size_t a, size_t b) {
        if (fabs(vertices[a].y - vertices[b].y) > 1e-10)
            return vertices[a].y > vertices[b].y;  // y 坐标大的优先（从上到下）
        return vertices[a].x < vertices[b].x;  // 平局时：从左到右
    });
```

### 步骤 2：识别左链和右链

```cpp
size_t top = order.front();
size_t bottom = order.back();

std::vector<char> isLeftChain(n, 0);

size_t v = top;
while (v != bottom) {
    isLeftChain[v] = 1;  // 标记左链上的顶点
    v = (v + 1) % n;
}
```

### 步骤 3：使用栈进行三角剖分

```cpp
std::vector<size_t> stack;
stack.push_back(order[0]);  // 顶部顶点
stack.push_back(order[1]);  // 第二个顶点

for (size_t i = 2; i < n - 1; ++i) {
    size_t curr = order[i];
    
    if (isLeftChain[curr] != isLeftChain[stack.back()]) {
        // 不同链：弹出所有顶点并形成三角形
        // ...
    } else {
        // 同一链：当对角线有效时弹出
        // ...
    }
}
```

### 步骤 4：将剩余顶点连接到底部

```cpp
size_t bottom_idx = order.back();
// 与栈中剩余顶点形成三角形
```

## 时间复杂度

- **排序顶点**：O(n log n)
- **三角剖分**：O(n)
- **总计**：O(n log n)

## 空间复杂度

- O(n) 用于栈和辅助数组

## 实现细节

### 三角形方向

对于 CCW（逆时针）多边形，三角形必须保持 CCW 方向：

```cpp
if (isLeftChain[curr]) {
    // 左链：
    result.triangles.emplace_back(
        vertices[curr], vertices[v_prev], vertices[v_top]);
} else {
    // 右链：
    result.triangles.emplace_back(
        vertices[curr], vertices[v_top], vertices[v_prev]);
}
```

### 凸性检查

当处理同一链上的顶点时，检查对角线是否是凸的：

```cpp
Vector2D v1 = vertices[curr] - vertices[v_last];
Vector2D v2 = vertices[v_top] - vertices[v_last];
double cross = v1.Cross(v2);

bool is_convex = false;
if (isLeftChain[curr]) {
    // 在左链上，需要 cross < 0（右转）
    is_convex = (cross < -1e-10);
} else {
    // 在右链上，需要 cross > 0（左转）
    is_convex = (cross > 1e-10);
}
```

## 局限性

### 1. Y-单调要求

**问题**：该算法仅对 y-单调多边形正确工作。

**症状**：
- 三角形数量不正确
- 三角形在多边形外部
- 边相交
- 某些区域缺少三角形

**解决方案**：对于一般简单多边形，使用耳切法算法。

### 2. 顶点排序

**问题**：顶点必须按严格的 y 顺序处理。

**解决方案**：在处理前按 y 坐标排序顶点。

### 3. 数值精度

**问题**：浮点精度问题可能导致错误的比较。

**解决方案**：使用 epsilon 容差（1e-10）进行比较。

## 与其他算法的对比

| 算法 | 时间复杂度 | 要求 | 稳定性 |
|------|-----------|------|--------|
| 扫描线 | O(n log n) | Y-单调多边形 | 高（对于有效输入） |
| 耳切法 | O(n²) | 简单多边形 | 非常高 |
| Delaunay | O(n log n) | 点集 | 高 |

## 使用示例

```cpp
// 生成 y-单调多边形
std::vector<Point2D> polygon = GenerateYMonotonePolygon();

// 使用扫描线进行三角剖分
TriangulationResult result = Triangulation::SweepLineTriangulation(polygon);

// 检查结果
if (result.IsValid()) {
    std::cout << "三角形数量: " << result.TriangleCount() << std::endl;
    std::cout << "总面积: " << result.TotalArea() << std::endl;
}
```

## 参考资料

- Computational Geometry: Algorithms and Applications
- de Berg et al., Chapter 3: Polygon Triangulation
- G. J. E. Balogh, "Monotone Polygon Triangulation"

## 参见

- [耳切法算法](./ear_clipping.md)
- [Delaunay 三角剖分](./delaunay.md)
- [多边形性质](./polygon_properties.md)
