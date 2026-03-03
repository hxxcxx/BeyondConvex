# 库导出架构设计

## 设计目标

为了支持将几何库作为独立库导出，同时保持 ABI 稳定性和接口控制，我们采用了 **Facade 模式 + 内部实现** 的架构。

## 架构概览

```
┌─────────────────────────────────────────────────────────┐
│         GeometryUtils (对外接口 - 导出)                  │
│                                                         │
│ 职责：稳定的公共接口，控制导出符号                        │
│                                                         │
│ + ToLeftTest(p, q, r): bool                            │
│ + ToLeftTest(u, v): bool                               │
│                                                         │
│ 内部实现：                                               │
│ - 参数验证                                              │
│ - 错误处理                                              │
│ - 调用 GeometryCore                                     │
└─────────────────────────────────────────────────────────┘
                          ▲
                          │ 调用
                          │
┌─────────────────────────────────────────────────────────┐
│          GeometryCore (内部实现 - 不导出)                 │
│                                                         │
│ 职责：实际的算法实现，可以自由修改                          │
│                                                         │
│ namespace internal {                                    │
│   class GeometryCore {                                  │
│     + ToLeftTestImpl(...)                               │
│     + FindLeftmostPointImpl(...)                        │
│     + FindMostCounterClockwisePointImpl(...)            │
│     + IsMoreCounterClockwiseImpl(...)                   │
│   };                                                    │
│ }                                                       │
└─────────────────────────────────────────────────────────┘
```

## 文件组织

```
src/
├── core/
│   ├── point2d.h/cc              # 基础数据结构
│   ├── vector2d.h/cc             # 基础数据结构
│   ├── edge2d.h                  # 基础数据结构
│   ├── geometry_utils.h/cc       # 对外接口（Facade）
│   └── geometry_core.h/cc        # 内部实现（Core）
│
└── convex_hull/                  # 凸包模块
    ├── convex_hull.h/cc
    ├── convex_hull_builder.h/cc
    └── convex_hull_factory.h/cc
```

## 设计优势

### 1. ABI 稳定性

**问题**：如果直接导出实现类，修改内部实现会破坏 ABI 兼容性。

**解决**：
- `GeometryUtils` 作为稳定的 Facade 接口
- `GeometryCore` 作为内部实现，可以自由修改
- 修改 `GeometryCore` 不影响 ABI

### 2. 接口控制

**导出的接口**：
```cpp
// geometry_utils.h (对外)
namespace geometry {
  class GeometryUtils {
   public:
    static bool ToLeftTest(const Point2D& p, const Point2D& q, const Point2D& r);
  };
}
```

**不导出的实现**：
```cpp
// geometry_core.h (内部)
namespace geometry {
namespace internal {
  class GeometryCore {
   public:
    static bool ToLeftTestImpl(const Point2D& p, const Point2D& q, const Point2D& r);
  };
}
}
```

### 3. 版本管理

**场景 1：修改内部实现**
```cpp
// geometry_core.cc
bool GeometryCore::ToLeftTestImpl(...) {
  // 修改实现，不影响 ABI
  return improved_algorithm(...);
}
```
✅ 不需要重新编译客户端代码

**场景 2：添加新功能**
```cpp
// geometry_utils.h
class GeometryUtils {
 public:
  static bool ToLeftTest(...) { /* 旧接口 */ }
  static bool ToLeftTestEx(...) { /* 新接口 */ }  // 新增
};
```
✅ 向后兼容

### 4. 测试友好

**单元测试**：
```cpp
// 测试内部实现
TEST(GeometryCore, ToLeftTestImpl) {
  EXPECT_TRUE(internal::GeometryCore::ToLeftTestImpl(p, q, r));
}

// 测试公共接口
TEST(GeometryUtils, ToLeftTest) {
  EXPECT_TRUE(GeometryUtils::ToLeftTest(p, q, r));
}
```

## 使用方式

### 作为静态库（当前）

```cpp
// 客户端代码
#include "geometry_utils.h"

bool result = geometry::GeometryUtils::ToLeftTest(p, q, r);
```

### 作为动态库（未来）

```cpp
// geometry_utils.h
#ifdef GEOMETRY_EXPORTS
  #define GEOMETRY_API __declspec(dllexport)
#else
  #define GEOMETRY_API __declspec(dllimport)
#endif

class GEOMETRY_API GeometryUtils {
  // ...
};
```

```bash
# 编译 DLL
cl /DGEOMETRY_EXPORTS /LD geometry_lib.cc

# 客户端使用
cl client.cpp geometry_lib.lib
```

## 扩展性

### 添加新的基础操作

```cpp
// 1. 在 GeometryCore 中添加实现
namespace internal {
  class GeometryCore {
   public:
    static bool IsCollinearImpl(...) { /* 实现 */ }
  };
}

// 2. 在 GeometryUtils 中添加接口
class GeometryUtils {
 public:
  static bool IsCollinear(...) {
    return internal::GeometryCore::IsCollinearImpl(...);
  }
};
```

### 添加新的算法模块

```
src/
├── core/              # 核心层（保持稳定）
├── convex_hull/       # 凸包模块
├── triangulation/     # 三角剖分模块（新增）
│   ├── triangulation.h/cc
│   ├── triangulation_builder.h/cc
│   └── triangulation_factory.h/cc
└── voronoi/          # Voronoi 模块（新增）
```

## 依赖关系

```
客户端代码
    ↓ 依赖
GeometryUtils (Facade)
    ↓ 调用
GeometryCore (内部实现)
    ↑ 被调用
ConvexHullBuilder (算法实现)
```

## 最佳实践

### 1. 接口设计原则

**GeometryUtils（对外接口）**：
- ✅ 简单、稳定、向后兼容
- ✅ 参数验证
- ✅ 错误处理
- ❌ 不包含实现细节

**GeometryCore（内部实现）**：
- ✅ 可以自由修改
- ✅ 专注于算法逻辑
- ❌ 不对外暴露

### 2. 命名约定

```cpp
// 公共接口：清晰、简洁
GeometryUtils::ToLeftTest(...)

// 内部实现：添加 Impl 后缀
internal::GeometryCore::ToLeftTestImpl(...)
```

### 3. 文档注释

```cpp
// geometry_utils.h (对外文档）
/// <summary>
/// Check if point r is to the left of the directed line pq
/// </summary>
/// <param name="p">Start point of the line</param>
/// <param name="q">End point of the line</param>
/// <param name="r">Point to test</param>
/// <returns>true if r is to the left of pq</returns>
static bool ToLeftTest(const Point2D& p, const Point2D& q, const Point2D& r);

// geometry_core.h (内部注释）
// Implementation: Uses cross product to determine orientation
static bool ToLeftTestImpl(const Point2D& p, const Point2D& q, const Point2D& r);
```

## 总结

这个架构设计提供了：

1. ✅ **清晰的分层**：接口层 vs 实现层
2. ✅ **ABI 稳定性**：修改实现不影响接口
3. ✅ **灵活性**：内部可以自由重构
4. ✅ **可测试性**：每层都可以独立测试
5. ✅ **可扩展性**：易于添加新功能
6. ✅ **库导出友好**：支持静态库和动态库

这是一个专业的、可维护的、适合作为库导出的架构设计！
