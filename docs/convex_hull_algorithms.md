# 凸包算法对比

本文档介绍了 BeyondConvex 库中实现的三种凸包算法及其特点。

## 目录

- [算法概述](#算法概述)
- [1. Jarvis March (Gift Wrapping)](#1-jarvis-march-gift-wrapping)
- [2. Graham Scan](#2-graham-scan)
- [3. Monotone Chain](#3-monotone-chain)
- [算法对比](#算法对比)
- [使用示例](#使用示例)

---

## 算法概述

| 算法 | 时间复杂度 | 空间复杂度 | 实现难度 | 推荐场景 |
|------|-----------|-----------|---------|---------|
| Jarvis March | O(nh) | O(h) | ⭐ 简单 | 凸包顶点少 (h << n) |
| Graham Scan | O(n log n) | O(n) | ⭐⭐ 中等 | 通用场景 |
| Monotone Chain | O(n log n) | O(n) | ⭐⭐ 简单 | **推荐默认算法** |

其中：
- **n**：点的总数
- **h**：凸包顶点数

---

## 1. Jarvis March (Gift Wrapping)

### 算法原理

Jarvis March 算法模拟"包装礼物"的过程，从最左边的点开始，每次找到相对于当前点最逆时针的下一个点，直到回到起点。

### 核心步骤

```
1. 找到最左边的点作为起点
2. 对于当前点，找到最逆时针的下一个点
3. 重复步骤 2，直到回到起点
```

### 可视化

```
        B
       / \
      /   \
     /     \
    A-------C
     \     /
      \   /
       \ /
        D

起点 A → 最逆时针的点 B → 最逆时针的点 C → 最逆时针的点 D → 回到 A
```

### 代码实现

```cpp
ConvexHull ConvexHullBuilder::BuildJarvisMarch(const std::vector<Point2D>& points) {
  // Step 1: 找到最左边的点
  int start_index = FindLeftmostPoint(points);
  
  // Step 2: 包装凸包
  int current = start_index;
  do {
    hull_vertices.push_back(points[current]);
    int next = FindMostCounterClockwisePoint(points, current);
    current = next;
  } while (current != start_index);
  
  return ConvexHull(hull_vertices);
}
```

### 优缺点

**优点：**
- ✅ 实现简单，易于理解
- ✅ 不需要排序
- ✅ 空间复杂度低 O(h)
- ✅ 适合凸包顶点很少的情况

**缺点：**
- ❌ 最坏情况 O(n²)（所有点都在凸包上）
- ❌ 对点集分布敏感

### 使用场景

- 点集分布稀疏，凸包顶点很少
- 教学演示，理解凸包概念
- 内存受限的环境

---

## 2. Graham Scan

### 算法原理

Graham Scan 算法通过极角排序和栈扫描来构建凸包。首先找到最低点作为基准，然后按极角排序其他点，最后用栈维护凸包顶点。

### 核心步骤

```
1. 找到最低点 (y 最小，若相等则 x 最小)
2. 按极角排序其他点
3. 预处理：移除与基准点共线的多余点
4. 用栈构建凸包：
   - 对于每个点，检查是否形成左转
   - 如果不是左转，弹出栈顶
   - 将当前点压入栈
```

### 可视化

```
排序后（按极角）：
        C
       /
      /
     B
    / \
   /   \
  A-----D
   \
    \
     E

栈构建过程：
[A] → [A, B] → [A, B, C] → [A, B, D] (C 被弹出) → [A, B, D, E]
```

### 代码实现

```cpp
ConvexHull ConvexHullBuilder::BuildGrahamScan(const std::vector<Point2D>& points) {
  // Step 1: 找到最低点
  int pivot_index = FindLowestPoint(points);
  Point2D pivot = points[pivot_index];
  
  // Step 2: 按极角排序
  std::sort(sorted_points.begin(), sorted_points.end(),
    [&pivot](const Point2D& a, const Point2D& b) {
      return CompareByPolarAngle(pivot, a, b);
    });
  
  // Step 3: 移除共线点
  // ... (保留最远的点)
  
  // Step 4: 栈构建
  std::vector<Point2D> stack;
  stack.push_back(pivot);
  stack.push_back(sorted_points[0]);
  
  for (size_t i = 1; i < sorted_points.size(); ++i) {
    while (stack.size() >= 2) {
      const Point2D& top = stack.back();
      const Point2D& second_top = stack[stack.size() - 2];
      
      if (!ToLeftTest(second_top, top, sorted_points[i])) {
        stack.pop_back();
      } else {
        break;
      }
    }
    stack.push_back(sorted_points[i]);
  }
  
  return ConvexHull(stack);
}
```

### 优缺点

**优点：**
- ✅ 时间复杂度稳定 O(n log n)
- ✅ 经典算法，广泛使用
- ✅ 适合大多数场景

**缺点：**
- ❌ 需要计算极角（或复杂的叉积比较）
- ❌ 需要预处理共线点
- ❌ 实现相对复杂

### 使用场景

- 通用凸包计算
- 需要稳定性能的场景
- 点集分布均匀的情况

---

## 3. Monotone Chain

### 算法原理

Monotone Chain 算法通过分别构建上凸包和下凸包来得到完整凸包。首先按 x 坐标排序，然后从左到右构建下凸包，从右到左构建上凸包。

### 核心步骤

```
1. 按 x, y 坐标排序
2. 构建下凸包（从左到右）：
   - 保持非右转（左转或共线）
3. 构建上凸包（从右到左）：
   - 保持非右转（左转或共线）
4. 合并上下凸包（去掉重复端点）
```

### 可视化

```
排序后（按 x）：
A B C D E F

下凸包（从左到右）：
A —————— B —————— F

上凸包（从右到左）：
F —————— E —————— A

合并：
A → B → F → E → A
```

### 代码实现

```cpp
ConvexHull ConvexHullBuilder::BuildMonotoneChain(const std::vector<Point2D>& points) {
  // Step 1: 按 x, y 排序
  std::vector<Point2D> sorted = points;
  std::sort(sorted.begin(), sorted.end(), [](const Point2D& a, const Point2D& b) {
    return a.x < b.x || (a.x == b.x && a.y < b.y);
  });
  
  // Step 2: 构建下凸包
  std::vector<Point2D> lower;
  for (const auto& p : sorted) {
    while (lower.size() >= 2) {
      const Point2D& top = lower.back();
      const Point2D& second_top = lower[lower.size() - 2];
      Vector2D v1 = top - second_top;
      Vector2D v2 = p - top;
      
      if (v1.Cross(v2) <= 0) {
        lower.pop_back();
      } else {
        break;
      }
    }
    lower.push_back(p);
  }
  
  // Step 3: 构建上凸包
  std::vector<Point2D> upper;
  for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
    while (upper.size() >= 2) {
      const Point2D& top = upper.back();
      const Point2D& second_top = upper[upper.size() - 2];
      Vector2D v1 = top - second_top;
      Vector2D v2 = *it - top;
      
      if (v1.Cross(v2) <= 0) {
        upper.pop_back();
      } else {
        break;
      }
    }
    upper.push_back(*it);
  }
  
  // Step 4: 合并（去掉重复端点）
  std::vector<Point2D> hull_vertices;
  for (size_t i = 0; i < lower.size() - 1; ++i) {
    hull_vertices.push_back(lower[i]);
  }
  for (size_t i = 0; i < upper.size() - 1; ++i) {
    hull_vertices.push_back(upper[i]);
  }
  
  return ConvexHull(hull_vertices);
}
```

### 优缺点

**优点：**
- ✅ 实现最简单，逻辑清晰
- ✅ 不需要极角计算，只需坐标排序
- ✅ 不需要预处理共线点
- ✅ 性能稳定 O(n log n)
- ✅ 代码简洁，易于维护

**缺点：**
- ⚠️ 需要两次扫描（上凸包 + 下凸包）
- ⚠️ 需要额外的空间存储上下凸包

### 使用场景

- **推荐作为默认算法** ✅
- 需要简洁代码的场景
- 需要稳定性能的场景
- 生产环境首选

---

## 算法对比

### 性能对比

| 点集规模 | Jarvis March | Graham Scan | Monotone Chain |
|---------|--------------|-------------|----------------|
| n=100, h=10 | ~1,000 ops | ~1,000 ops | ~1,000 ops |
| n=1000, h=20 | ~20,000 ops | ~10,000 ops | ~10,000 ops |
| n=1000, h=500 | ~500,000 ops | ~10,000 ops | ~10,000 ops |
| n=10000, h=50 | ~500,000 ops | ~130,000 ops | ~130,000 ops |

### 代码复杂度

| 算法 | 代码行数 | 核心逻辑 | 辅助函数 |
|------|---------|---------|---------|
| Jarvis March | ~60 行 | 循环找最逆时针点 | 3 个 |
| Graham Scan | ~80 行 | 排序 + 栈扫描 | 4 个 |
| Monotone Chain | ~50 行 | 两次栈扫描 | 0 个 ✅ |

### 实际应用建议

```
┌─────────────────────────────────────────┐
│  选择算法的决策树                        │
├─────────────────────────────────────────┤
│                                         │
│  凸包顶点很少 (h < log n)?              │
│       ├── 是 → Jarvis March             │
│       └── 否 → 继续                     │
│                                         │
│  需要最简单的实现?                      │
│       ├── 是 → Monotone Chain ✅        │
│       └── 否 → 继续                     │
│                                         │
│  需要经典算法?                          │
│       ├── 是 → Graham Scan              │
│       └── 否 → Monotone Chain ✅        │
│                                         │
└─────────────────────────────────────────┘
```

---

## 使用示例

### C++ 代码示例

```cpp
#include "convex_hull/convex_hull_factory.h"
#include "core/point2d.h"
#include <vector>

using namespace geometry;

int main() {
  // 创建点集
  std::vector<Point2D> points = {
    {0.0, 0.0},
    {1.0, 1.0},
    {2.0, 0.5},
    {1.5, -0.5},
    {0.5, -0.5}
  };
  
  // 使用 Monotone Chain 算法（推荐）
  ConvexHull hull = ConvexHullFactory::Create(
    ConvexHullAlgorithm::MonotoneChain,
    points
  );
  
  // 获取凸包顶点
  std::cout << "凸包顶点数: " << hull.Size() << std::endl;
  for (size_t i = 0; i < hull.Size(); ++i) {
    const Point2D& vertex = hull.GetVertex(i);
    std::cout << "  顶点 " << i << ": (" 
              << vertex.x << ", " << vertex.y << ")" << std::endl;
  }
  
  // 计算凸包面积和周长
  std::cout << "面积: " << hull.Area() << std::endl;
  std::cout << "周长: " << hull.Perimeter() << std::endl;
  
  // 检查点是否在凸包内
  Point2D test_point(1.0, 0.0);
  bool inside = hull.Contains(test_point);
  std::cout << "点 (" << test_point.x << ", " << test_point.y 
            << ") 在凸包内? " << (inside ? "是" : "否") << std::endl;
  
  return 0;
}
```

### 切换算法

```cpp
// Jarvis March (适合凸包顶点很少的情况)
auto hull1 = ConvexHullFactory::Create(
  ConvexHullAlgorithm::JarvisMarch, points
);

// Graham Scan (经典算法)
auto hull2 = ConvexHullFactory::Create(
  ConvexHullAlgorithm::GrahamScan, points
);

// Monotone Chain (推荐默认算法)
auto hull3 = ConvexHullFactory::Create(
  ConvexHullAlgorithm::MonotoneChain, points
);
```

### 获取算法信息

```cpp
// 检查算法是否支持
bool supported = ConvexHullFactory::IsAlgorithmSupported(
  ConvexHullAlgorithm::MonotoneChain
);

// 获取所有支持的算法
std::vector<ConvexHullAlgorithm> algorithms = 
  ConvexHullFactory::GetSupportedAlgorithms();

// 获取算法名称
std::string name = GetAlgorithmName(
  ConvexHullAlgorithm::MonotoneChain
);
// 输出: "Monotone Chain"

// 获取算法复杂度
std::string complexity = GetAlgorithmComplexity(
  ConvexHullAlgorithm::MonotoneChain
);
// 输出: "O(n log n)"
```

---

## 总结

### 推荐使用

| 场景 | 推荐算法 | 原因 |
|------|---------|------|
| **通用场景** | Monotone Chain | 简单、稳定、高效 |
| 教学演示 | Jarvis March | 易于理解 |
| 点集稀疏 | Jarvis March | h << n 时更快 |
| 经典应用 | Graham Scan | 广泛使用 |

### 性能排名

1. **Monotone Chain** - 最稳定，最简单 ✅
2. **Graham Scan** - 经典算法，性能稳定
3. **Jarvis March** - 特定场景下快，但不稳定

### 代码质量排名

1. **Monotone Chain** - 最简洁，50 行 ✅
2. **Jarvis March** - 较简单，60 行
3. **Graham Scan** - 较复杂，80 行

---

## 参考资料

- [Computational Geometry: Algorithms and Applications](https://www.cs.uu.nl/geobook/)
- [Convex Hull Algorithms](https://en.wikipedia.org/wiki/Convex_hull_algorithms)
- [Monotone Chain Algorithm](https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain)
