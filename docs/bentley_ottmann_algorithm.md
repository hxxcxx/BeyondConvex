# Bentley-Ottmann 算法

## 概述

Bentley-Ottmann 算法是一种经典的**扫描线算法**，用于高效地找到平面上所有线段的交点。

### 算法特点

| 特性 | 描述 |
|------|------|
| **时间复杂度** | O((n + k) log n) |
| **空间复杂度** | O(n + k) |
| **n** | 线段数量 |
| **k** | 交点数量 |

### 与暴力算法对比

| 算法 | 时间复杂度 | 适用场景 |
|------|-----------|---------|
| **暴力算法** | O(n²) | 线段数量少（n < 100） |
| **Bentley-Ottmann** | O((n + k) log n) | 线段数量多，交点少 |

---

## 算法原理

### 核心思想

使用一条**垂直扫描线**从上到下扫描平面，在扫描过程中维护与扫描线相交的线段集合。

### 数据结构

```
1. 事件队列（Event Queue）
   - 优先队列（最小堆）
   - 存储所有端点和交点
   - 按 y 坐标排序（y 大的优先级高）

2. 扫描线状态（Sweep Line Status）
   - 平衡二叉搜索树
   - 存储与扫描线相交的线段
   - 按线段在扫描线处的 y 坐标排序
```

### 事件类型

```
1. 左端点事件
   - 线段的左端点（x 较小的端点）
   - 动作：将线段插入扫描线状态

2. 右端点事件
   - 线段的右端点（x 较大的端点）
   - 动作：将线段从扫描线状态移除

3. 交点事件
   - 两条线段的交点
   - 动作：重新排序相关线段
```

---

## 算法步骤

### 初始化

```
1. 将所有线段的端点插入事件队列
2. 初始化空的扫描线状态
3. 初始化空的交点集合
```

### 主循环

```
while (事件队列不为空) {
    取出优先级最高的事件点
    
    if (是左端点事件) {
        将线段插入扫描线状态
        检查与相邻线段的交点
    }
    else if (是右端点事件) {
        将线段从扫描线状态移除
        检查相邻线段是否产生新交点
    }
    else if (是交点事件) {
        记录交点
        重新排序相关线段
        检查新产生的交点
    }
}
```

---

## 可视化示例

### 场景 1：两条相交线段

```
初始状态：
    A
   / \
  /   \
 /     \
B-------C

扫描线从上到下移动：

步骤 1: 扫描线在 A 点
        - 插入线段 AB
        - 插入线段 AC

步骤 2: 扫描线在交点
        - 发现交点
        - 记录交点位置

步骤 3: 扫描线在 B 点
        - 移除线段 AB

步骤 4: 扫描线在 C 点
        - 移除线段 AC
```

### 场景 2：多条线段

```
        D
       /|\
      / | \
     /  |  \
    A---+---B
         \
          \
           C

扫描线状态变化：
1. D 点: 插入 DA, DB, DC
2. 交点: 发现 DA 与 DB 的交点
3. A 点: 移除 DA
4. B 点: 移除 DB
5. C 点: 移除 DC
```

---

## 代码实现

### 核心数据结构

```cpp
// 事件点
struct EventPoint {
  Point2D point;
  bool is_left_endpoint;
  int segment_index;
};

// 交点
struct IntersectionPoint {
  Point2D point;
  int segment1_index;
  int segment2_index;
};
```

### 主算法

```cpp
std::vector<IntersectionPoint> FindAllIntersections(
    const std::vector<Edge2D>& segments) {
  
  // 1. 初始化事件队列
  std::priority_queue<EventPoint> event_queue;
  for (const auto& seg : segments) {
    event_queue.emplace(seg.p1, true, index);
    event_queue.emplace(seg.p2, false, index);
  }
  
  // 2. 初始化扫描线状态
  std::set<SegmentInfo> sweep_line;
  
  // 3. 处理事件
  while (!event_queue.empty()) {
    EventPoint event = event_queue.top();
    event_queue.pop();
    
    if (event.is_left_endpoint) {
      // 左端点：插入线段
      sweep_line.insert(segment);
      // 检查与相邻线段的交点
    } else {
      // 右端点：移除线段
      sweep_line.erase(segment);
      // 检查相邻线段的新交点
    }
  }
  
  return intersections;
}
```

---

## 使用示例

### C++ 代码

```cpp
#include "intersection/line_segment_intersection.h"
#include "core/edge2d.h"
#include <vector>

using namespace geometry;

int main() {
  // 创建线段
  std::vector<Edge2D> segments = {
    Edge2D(Point2D(0, 0), Point2D(10, 10)),   // 对角线
    Edge2D(Point2D(0, 10), Point2D(10, 0)),   // 另一条对角线
    Edge2D(Point2D(2, 0), Point2D(2, 10)),    // 垂直线
    Edge2D(Point2D(0, 5), Point2D(10, 5))     // 水平线
  };
  
  // 使用 Bentley-Ottmann 算法
  auto intersections = LineSegmentIntersection::FindAllIntersections(segments);
  
  // 输出交点
  std::cout << "找到 " << intersections.size() << " 个交点" << std::endl;
  for (const auto& intersection : intersections) {
    std::cout << "交点: (" << intersection.point.x << ", " 
              << intersection.point.y << ")" << std::endl;
    std::cout << "  线段 " << intersection.segment1_index 
              << " 与线段 " << intersection.segment2_index << std::endl;
  }
  
  return 0;
}
```

### 交互式使用

在 BeyondConvex 查看器中：

1. 选择 **"Line Segment Intersection"** 场景
2. 点击画布设置线段的起点
3. 再次点击设置线段的终点
4. 交点会以红色圆圈显示
5. 可以在 UI 中切换算法：
   - **Bentley-Ottmann** (O((n + k) log n))
   - **Brute Force** (O(n²))

---

## 算法分析

### 时间复杂度

```
初始化事件队列: O(n log n)
处理每个事件: O(log n)
交点事件数量: k

总时间复杂度: O((n + k) log n)
```

### 空间复杂度

```
事件队列: O(n + k)
扫描线状态: O(n)
交点集合: O(k)

总空间复杂度: O(n + k)
```

### 优化空间

1. **退化情况处理**
   - 垂直线段
   - 水平线段
   - 共线线段
   - 端点重合

2. **数值稳定性**
   - 使用浮点数容差（epsilon）
   - 避免除零错误

---

## 应用场景

### 计算机图形学

- 多边形裁剪
- 隐藏线消除
- 区域填充

### 地理信息系统 (GIS)

- 地图叠加
- 区域查询
- 空间分析

### 机器人路径规划

- 障碍物检测
- 碰撞检测
- 可视图构建

### VLSI 设计

- 布线检查
- 设计规则验证
- 碰撞检测

---

## 扩展阅读

### 相关算法

1. **Shamos-Hoey 算法**
   - 检测是否存在交点（不报告所有交点）
   - 时间复杂度：O(n log n)

2. **Chazelle 算法**
   - 理论最优算法
   - 时间复杂度：O(n log n + k)

3. **随机化算法**
   - 期望时间复杂度：O(n log n + k)
   - 实现简单，性能好

### 参考资料

- [Computational Geometry: Algorithms and Applications](https://www.cs.uu.nl/geobook/) - Chapter 2
- [Bentley-Ottmann Algorithm on Wikipedia](https://en.wikipedia.org/wiki/Bentley%E2%80%93Ottmann_algorithm)
- [Line Segment Intersection - CGAL](https://doc.cgal.org/latest/Spatial_sorting/index.html)

---

## 实现细节

### 当前实现的限制

1. **不处理共线情况**
   - 共线线段的交点不会报告
   - 端点在线段上不会报告

2. **数值精度**
   - 使用 double 类型
   - 容差设置为 1e-10

3. **性能优化**
   - 使用 std::set 作为扫描线状态
   - 可以进一步优化为平衡树

### 未来改进

1. ✅ 处理共线情况
2. ✅ 处理端点交点
3. ✅ 添加可视化动画
4. ✅ 性能优化和基准测试

---

**最后更新：** 2026-03-04  
**版本：** 1.0.0  
**作者：** BeyondConvex Team
