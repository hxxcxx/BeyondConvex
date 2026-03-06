# 凸包求交算法 (Convex Polygon Intersection)

## 概述

本项目实现了两种高效的凸包求交算法，用于计算两个凸多边形的交集。这些算法基于计算几何的经典理论，包括 Sutherland-Hodgman 裁剪算法和二分查找优化方法。

## 算法实现

### 1. 线性扫描算法 (Linear Scan - Sutherland-Hodgman)

**复杂度**: O(n + m)

**原理**:
- 使用 Sutherland-Hodgman 多边形裁剪算法
- 将一个凸多边形作为"裁剪窗口"
- 逐边裁剪另一个凸多边形
- 保留在裁剪窗口内的部分

**步骤**:
1. 从第一个凸多边形开始
2. 用第二个凸多边形的每一条边作为裁剪线
3. 对于每条裁剪线，保留在其左侧（内部）的所有顶点
4. 计算交点并添加到结果中
5. 重复直到所有边都处理完毕

**优点**:
- 实现简单直观
- 时间复杂度线性
- 适用于大多数实际场景

**缺点**:
- 需要处理所有边
- 对于小规模问题可能不是最优

### 2. 二分查找算法 (Binary Search)

**复杂度**: O(log n + log m)

**原理**:
- 利用凸多边形的有序性质
- 使用二分查找快速定位交点
- 只检查可能相交的边对

**步骤**:
1. 检查一个多边形是否完全包含在另一个内
2. 使用二分查找找到切线点
3. 只检查可能相交的边对
4. 计算所有交点
5. 按角度排序形成交集多边形

**优点**:
- 对数时间复杂度
- 适合大规模凸多边形
- 可以快速判断包含关系

**缺点**:
- 实现较复杂
- 需要额外的排序步骤

## 数据结构

### ConvexIntersectionResult

```cpp
struct ConvexIntersectionResult {
  std::vector<Point2D> vertices;  // 交集顶点（逆时针顺序）
  bool is_empty;                  // 是否无交集
  bool is_point;                  // 交集是否为单点
  bool is_segment;                // 交集是否为线段
};
```

### 交集类型

1. **空集**: 两个凸包不相交
2. **单点**: 两个凸包在一点相切
3. **线段**: 两个凸包在一条线段上相交
4. **多边形**: 两个凸包的交集是一个凸多边形

## 核心函数

### Intersect()

```cpp
static ConvexIntersectionResult Intersect(
    const ConvexHull& convex1,
    const ConvexHull& convex2,
    ConvexIntersectionAlgorithm algorithm);
```

计算两个凸包的交集。

**参数**:
- `convex1`: 第一个凸包
- `convex2`: 第二个凸包
- `algorithm`: 使用的算法

**返回**: 交集结果

### ClipConvexPolygon()

```cpp
static std::vector<Point2D> ClipConvexPolygon(
    const std::vector<Point2D>& polygon,
    const Point2D& clip_p1,
    const Point2D& clip_p2);
```

用一条直线裁剪凸多边形（Sutherland-Hodgman 核心操作）。

## 可视化场景

### ConvexPolygonIntersectionScene

**功能**:
- 随机生成两个重叠的凸包
- 动画展示求交过程
- 支持算法切换
- 显示交集结果和统计信息

**控制**:
- `SPACE`: 生成新的随机凸包
- `A`: 切换算法

**可视化元素**:
- 蓝色半透明多边形：第一个凸包
- 橙色半透明多边形：第二个凸包
- 绿色实心多边形：交集结果

## 使用示例

```cpp
// 创建两个凸包
std::vector<Point2D> points1 = { ... };
std::vector<Point2D> points2 = { ... };

ConvexHull convex1 = ConvexHullFactory::Create(
    ConvexHullAlgorithm::MonotoneChain, points1);
ConvexHull convex2 = ConvexHullFactory::Create(
    ConvexHullAlgorithm::MonotoneChain, points2);

// 计算交集
ConvexIntersectionResult result = ConvexPolygonIntersection::Intersect(
    convex1, convex2, ConvexIntersectionAlgorithm::kLinearScan);

// 检查结果
if (result.is_empty) {
    std::cout << "No intersection" << std::endl;
} else if (result.is_point) {
    std::cout << "Intersection at point: " 
              << result.vertices[0].x << ", " 
              << result.vertices[0].y << std::endl;
} else {
    std::cout << "Intersection polygon with " 
              << result.vertices.size() << " vertices" << std::endl;
    
    // 转换为 ConvexHull 对象
    ConvexHull intersection = result.ToConvexHull();
    std::cout << "Area: " << intersection.Area() << std::endl;
}
```

## 算法比较

| 算法 | 时间复杂度 | 空间复杂度 | 适用场景 |
|------|-----------|-----------|---------|
| Linear Scan | O(n + m) | O(n + m) | 一般用途，实现简单 |
| Binary Search | O(log n + log m) | O(k) | 大规模多边形，需要快速查询 |

其中：
- n, m 是两个凸包的顶点数
- k 是交点的数量

## 数学基础

### 点在直线左侧判断

使用叉积判断点是否在有向直线的左侧：

```
cross(p2 - p1, p - p1) > 0  // p 在 p1->p2 的左侧
```

### 线段交点计算

使用参数方程计算两条线段的交点：

```
p1 + t * (p2 - p1) = q1 + s * (q2 - q1)
```

解这个方程组得到 t 和 s，如果都在 [0, 1] 范围内，则线段相交。

## 扩展：Dobkin-Kirkpatrick 算法

Doblin-Kirkpatrick 算法是一种更高级的凸多边形求交方法，基于层次数据结构：

**特点**:
- 预处理：O(n) 构建层次结构
- 查询：O(log n) 时间复杂度
- 支持动态更新

**应用场景**:
- 需要多次查询同一对多边形
- 动态环境中的碰撞检测
- 计算机图形学中的可见性判断

**未来实现方向**:
1. 构建 DK 层次结构
2. 实现快速查询算法
3. 支持增量更新

## 性能优化

1. **包围盒测试**: 先进行快速排斥测试
2. **包含关系检测**: 检查一个多边形是否完全在另一个内
3. **早期终止**: 发现不相交后立即返回
4. **内存预分配**: 预先分配结果数组

## 应用场景

1. **计算机图形学**: 区域裁剪、可见性判断
2. **机器人学**: 碰撞检测、路径规划
3. **地理信息系统**: 区域重叠分析
4. **CAD/CAM**: 零件干涉检查
5. **游戏开发**: 物理引擎、碰撞检测

## 参考文献

1. Sutherland, I. E., & Hodgman, G. W. (1974). "Reentrant polygon clipping"
2. de Berg, M., et al. "Computational Geometry: Algorithms and Applications"
3. Dobkin, D., & Kirkpatrick, D. (1990). "Determining the separation of convex polyhedra"
4. O'Rourke, J. "Computational Geometry in C"

## 文件结构

```
src/intersection/
├── convex_polygon_intersection.h      # 接口定义
└── convex_polygon_intersection.cc     # 算法实现

viewer/demos/intersection/
├── convex_polygon_intersection_scene.h   # 可视化场景头文件
└── convex_polygon_intersection_scene.cc  # 可视化场景实现
```

## 测试建议

1. **边界情况**:
   - 不相交的凸包
   - 一个包含另一个
   - 边界相切
   - 共享一条边

2. **性能测试**:
   - 小规模（3-5 个顶点）
   - 中等规模（10-20 个顶点）
   - 大规模（100+ 个顶点）

3. **正确性验证**:
   - 与暴力算法对比
   - 检查结果是否为凸包
   - 验证面积计算
