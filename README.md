# BeyondConvex

<div align="center">

一个现代化的 C++17 计算几何库，使用 Dear ImGui 提供交互式可视化。

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()

**功能特性：** 凸包 • 交集 • 三角剖分 • Voronoi • DCEL • 空间索引

</div>

## 概述

BeyondConvex 通过学习优质计算几何资源（邓俊辉教授课程、B站教程），结合 AI 辅助编程工具（VS Code Copilot + GLM 4.7）完成开发。

## 学习资源

### 视频教程
- **[德劳内三角形生成算法 Delaunay triangle generation algorithm（I）](https://www.bilibili.com/video/BV1QB4y1S7RK/?share_source=copy_web&vd_source=6ebb0754dc3f18fc0e61fca680a38c61)** - B站视频教程

### 在线课程
- **[计算几何（邓俊辉）](https://www.xuetangx.com/course/thu08091000327intl/3995175?channel=i.area.learn_title)** - 清华大学邓俊辉教授的计算几何课程

## 算法概览

| 类别 | 算法 | 时间复杂度 | 空间复杂度 | 状态 |
|----------|-----------|-----------------|------------------|--------|
| **Convex Hull** | Jarvis March | O(nh) | O(n) | ✅ |
| | Graham Scan | O(n log n) | O(n) | ✅ |
| | Monotone Chain | O(n log n) | O(n) | ✅ |
| **Intersection** | Bentley-Ottmann | O((n+k) log n) | O(n+k) | ✅ |
| | Convex Polygon (Linear) | O(n+m) | O(n+m) | ✅ |
| | Convex Polygon (Binary) | O(log n + log m) | O(1) | ✅ |
| **Triangulation** | Sweep Line | O(n log n) | O(n) | ✅ |
| | Ear Clipping | O(n²) | O(n) | ✅ |
| | Delaunay | O(n log n) | O(n) | ✅ |
| **Voronoi** | Site-based | O(n²) | O(n²) | ✅ |
| **Data Structure** | DCEL | - | O(V+E+F) | ✅ |
| **Spatial Index** | KD-Tree | O(log n) | O(n) | ✅ |
| | Quadtree | O(log n) | O(n) | ✅ |
| | R-Tree | O(log n) | O(n) | ✅ |
| | BSP-Tree | O(log n) | O(n) | ✅ |
| | BVH | O(log n) | O(n) | ✅ |

## 功能特性

- ✅ 遵循 Google C++ Style Guide 的现代 C++17 代码
- ✅ 基于 ImGui 的交互式可视化，包含 13 个 demo scenes
- ✅ 使用 Facade Pattern 的模块化架构
- ✅ 完整的 computational geometry algorithms 套件
- ✅ DCEL (Doubly Connected Edge List) 数据结构
- ✅ 跨平台支持（Windows、Linux、macOS）

## 项目结构

```
BeyondConvex/
├── CMakeLists.txt              # Main build configuration
├── src/                        # Core library
│   ├── geometry_utils.h/cc     # Public facade interface
│   ├── core/                   # Internal implementation
│   │   ├── geometry_core.h/cc  # Internal algorithms
│   │   ├── point2d.h/cc        # Point2D class
│   │   ├── vector2d.h/cc       # Vector2D class
│   │   └── edge2d.h            # Edge2D class
│   ├── convex_hull/            # ✅ Convex hull module
│   │   ├── convex_hull.h/cc    # ConvexHull class
│   │   ├── convex_hull_builder.h/cc  # Algorithm implementations
│   │   └── convex_hull_factory.h/cc  # Factory pattern
│   ├── intersection/           # ✅ Intersection algorithms
│   │   ├── line_segment_intersection.h/cc  # Bentley-Ottmann
│   │   └── convex_polygon_intersection.h/cc  # Convex polygon intersection
│   ├── triangulation/          # ✅ Triangulation algorithms
│   │   ├── triangulation.h/cc  # Sweep line, Ear clipping, Delaunay
│   │   ├── delaunay_triangulation.h/cc  # Delaunay (Bowyer-Watson)
│   │   └── spatial_grid.h      # Spatial grid optimization
│   ├── dcel/                   # ✅ DCEL data structure
│   │   ├── dcel.h/cc           # Main DCEL class
│   │   ├── vertex.h/cc         # Vertex records
│   │   ├── half_edge.h/cc      # Half-edge records
│   │   ├── face.h/cc           # Face records
│   │   └── dcel_builder.h/cc   # DCEL builder
│   └── voronoi/               # ✅ Voronoi diagrams
│       └── voronoi_diagram.h/cc  # Voronoi generation
│   └── spatial_index/         # ✅ Spatial indexing structures
│       ├── kdtree/            # KD-Tree implementation
│       │   ├── kdtree.h/cc    # KD-Tree class
│       │   └── kdtree_node.h  # KD-Tree node
│       ├── quadtree/          # Quadtree implementation
│       │   ├── quadtree.h/cc  # Quadtree class
│       │   └── quadtree_node.h # Quadtree node
│       ├── rtree/             # R-Tree implementation
│       │   ├── rtree.h/cc     # R-Tree class
│       │   └── rtree_node.h   # R-Tree node
│       ├── bsptree/           # BSP-Tree implementation
│       │   └── bsptree.h/cc   # BSP-Tree class
│       ├── bvhtree/           # BVH implementation
│       │   └── bvhtree.h/cc   # BVH class
│       └── spatial_index_common.h # Common structures
├── viewer/                     # ImGui visualization
│   ├── main.cc                # Entry point
│   ├── scene_manager.h/cc     # Scene management
│   ├── geometry_canvas.h/cc   # Rendering canvas
│   └── demos/                 # Algorithm demonstrations (8 scenes)
│       ├── scene_base.h       # Base scene class
│       ├── convex_hull/       # ✅ Convex hull scenes (3)
│       │   ├── to_left_test_scene.h/cc
│       │   ├── incremental_construction_scene.h/cc
│       │   └── convex_hull_scene.h/cc
│       ├── intersection/      # ✅ Intersection scenes (2)
│       │   ├── line_segment_intersection_scene.h/cc
│       │   └── convex_polygon_intersection_scene.h/cc
│       ├── triangulation/     # ✅ Triangulation scene (1)
│       │   └── triangulation_scene.h/cc
│       ├── voronoi/           # ✅ Voronoi scene (1)
│       │   └── voronoi_scene.h/cc
│       ├── dcel/              # ✅ DCEL test scene (1)
│       │   └── dcel_test_scene.h/cc
│       └── spatial_index/     # ✅ Spatial index scenes (5)
│           ├── kdtree_scene.h/cc
│           ├── quadtree_scene.h/cc
│           ├── rtree_scene.h/cc
│           ├── bsptree_scene.h/cc
│           └── bvh_scene.h/cc
├── docs/                       # ✅ Comprehensive documentation
│   ├── convex_hull_algorithms.md
│   ├── bentley_ottmann_algorithm.md
│   ├── sweep_line_bentley_ottmann.md
│   ├── ear_clipping_triangulation.md
│   ├── sweep_line_triangulation.md
│   ├── convex_polygon_intersection.md
│   ├── delaunay_triangulation.md
│   └── spatial_optimization.md
│   ├── kdtree_algorithm.md
│   ├── quadtree_algorithm.md
│   ├── rtree_algorithm.md
│   ├── bsptree_algorithm.md
│   ├── BVH_Tree_Documentation.md
│   └── spatial_optimization.md
└── third_party/               # External dependencies
    ├── glfw/                  # GLFW framework
    └── imgui/                 # Dear ImGui framework
```

## 构建说明

### 系统要求
- CMake 3.15+
- C++17 compatible compiler (MSVC, GCC, Clang)
- OpenGL 3.0+

### 构建步骤

```bash
# Clone repository
git clone <repo-url>
cd BeyondConvex

# Create build directory
mkdir build
cd build

# Configure with CMake (automatically downloads GLFW and ImGui)
cmake ..

# Build
cmake --build . --config Release

```

## 使用方法

### 运行查看器

```bash
# Build the project
cd build
cmake --build . --config Release

# Run the viewer
./bin/Release/geometry_viewer.exe  # Windows
./bin/Release/geometry_viewer      # Linux/macOS
```

### 查看器控制

- **Scenes Menu** (top menu bar): 在 8 个不同的 algorithm demonstrations 之间切换
- **Scene Control** (left panel): 选择 scene 和 reset
- **Scene Info** (left panel): 查看 scene details 和调整 parameters
- **Canvas** (right panel): 点击添加 points 并可视化 algorithms

### 可用的演示场景

1. **To-Left Test** - 交互式 geometric predicate testing
2. **Incremental Construction** - 逐步构建 convex hull
3. **Convex Hull** - 比较三种 convex hull algorithms
4. **Line Segment Intersection** - Bentley-Ottmann sweep line visualization
5. **Convex Polygon Intersection** - Polygon intersection algorithms
6. **Triangulation** - Sweep line, ear clipping, and Delaunay methods
7. **Voronoi Diagram** - 从 sites 生成 Voronoi cells
8. **DCEL Test** - Doubly Connected Edge List data structure
9. **KD-Tree** - Point-based spatial indexing with nearest neighbor search
10. **Quadtree** - 2D space partitioning for efficient range queries
11. **R-Tree** - Rectangle-based spatial indexing for GIS and games
12. **BSP-Tree** - Binary space partitioning for collision detection
13. **BVH** - Bounding Volume Hierarchy for ray tracing and collision

### 代码示例

```cpp
#include "geometry_utils.h"
#include "convex_hull/convex_hull_factory.h"
#include "intersection/line_segment_intersection.h"
#include "triangulation/triangulation.h"
#include "voronoi/voronoi_diagram.h"
#include "core/point2d.h"

using namespace geometry;

// ========== Convex Hull ==========
std::vector<Point2D> points = {{0.0, 0.0}, {3.0, 4.0}, {1.0, 1.0}};
ConvexHull hull = ConvexHullFactory::Create(
  ConvexHullAlgorithm::MonotoneChain, points
);
std::cout << "Hull area: " << hull.Area() << std::endl;

// ========== Line Segment Intersection ==========
std::vector<Edge2D> segments = {
  {{0.0, 0.0}, {2.0, 2.0}},
  {{0.0, 2.0}, {2.0, 0.0}}
};
auto intersections = LineSegmentIntersection::FindAllIntersections(segments);

// ========== Convex Polygon Intersection ==========
ConvexIntersectionResult result = ConvexPolygonIntersection::Intersect(
  hull1, hull2, ConvexIntersectionAlgorithm::kLinearScan
);

// ========== Triangulation ==========
std::vector<Point2D> polygon = {{0.0, 0.0}, {2.0, 0.0}, {1.0, 1.0}};
TriangulationResult tri = Triangulation::Triangulate(
  polygon, TriangulationAlgorithm::kEarClipping
);

// ========== Voronoi Diagram ==========
std::vector<Point2D> sites = {{0.0, 0.0}, {1.0, 1.0}, {2.0, 0.0}};
VoronoiDiagramResult voronoi = VoronoiDiagram::Generate(sites);

// ========== Spatial Indexing ==========
// KD-Tree for points
KDTree kdtree;
kdtree.Insert({0.0, 0.0});
kdtree.Insert({1.0, 1.0});
auto nearest = kdtree.NearestNeighbor({0.5, 0.5});

// Quadtree for range queries
Quadtree quadtree(BoundingBox(0, 0, 100, 100), 4);
quadtree.Insert({10.0, 10.0});
quadtree.Insert({20.0, 20.0});
auto points_in_range = quadtree.RangeQuery(BoundingBox(5, 5, 15, 15));

// R-Tree for rectangles
RTree rtree(4, 2);
rtree.Insert(BoundingBox(0, 0, 10, 10), 0);
rtree.Insert(BoundingBox(5, 5, 15, 15), 1);
auto intersecting = rtree.RangeQuery(BoundingBox(8, 8, 12, 12));

// ========== To-Left Test ==========
Point2D p(0.0, 0.0), q(1.0, 0.0), r(0.5, 0.5);
bool is_left = GeometryUtils::ToLeftTest(p, q, r);  // true
```

## 学习路径

### ✅ Stage 1: Foundation (已完成)
- Point2D, Vector2D, Edge2D classes
- To-Left Test (fundamental geometric predicate)
- Facade Pattern architecture (GeometryUtils/GeometryCore)
- ImGui visualization setup

### ✅ Stage 2: Convex Hull (已完成)
- **Jarvis March** - O(nh)
- **Graham Scan** - O(n log n)
- **Monotone Chain** - O(n log n) ⭐ 推荐
- Factory Pattern for algorithm selection
- 交互式 visualization scenes (3 demos)

### ✅ Stage 3: Intersection (已完成)
- **Line Segment Intersection** - Bentley-Ottmann O((n+k) log n)
- **Convex Polygon Intersection** - Linear Scan O(n+m) & Binary Search O(log n+log m)
- 交互式 visualization scenes (2 demos)

### ✅ Stage 4: DCEL Data Structure (已完成)
- **DCEL Implementation** - Vertex, HalfEdge, Face records
- **DCEL Builder** - Construct planar subdivisions
- 交互式 test scene (1 demo)

### ✅ Stage 5: Triangulation (已完成)
- **Sweep Line Triangulation** - Top-to-bottom approach
- **Ear Clipping Method** - O(n²) for simple polygons
- **Delaunay Triangulation** - Empty circumcircle property
- 交互式 visualization scene (1 demo)

### ✅ Stage 6: Voronoi Diagrams (已完成)
- **Voronoi Diagram Generation** - Site-based cell construction
- Relationship with Delaunay triangulation (dual graphs)
- 交互式 visualization scene (1 demo)

### ✅ Stage 7: Spatial Indexing (已完成)
- **KD-Tree** - k-dimensional tree for point data
- **Quadtree** - 2D recursive space partitioning
- **R-Tree** - Rectangle-based indexing for spatial objects
- **BSP-Tree** - Binary space partitioning
- **BVH** - Bounding Volume Hierarchy
- 交互式 visualization scenes (5 demos)

### 📋 Future Enhancements
- Fortune's sweep line algorithm for Voronoi (O(n log n))
- Half-plane intersection
- Circle-line intersection
- 更多 geometric predicates

## 依赖项

- **GLFW** - Window and input management (auto-downloaded by CMake)
- **OpenGL** - Graphics rendering
- **Dear ImGui** - Immediate mode GUI framework (included in third_party)

## 当前状态

### ✅ 已完成（所有主要功能）

**Core Infrastructure:**
- ✅ Modern C++17 project with CMake build system
- ✅ Point2D, Vector2D, Edge2D geometric primitives
- ✅ Facade Pattern architecture (GeometryUtils/GeometryCore)
- ✅ Google C++ Style Guide compliance

**Convex Hull Algorithms:**
- ✅ Jarvis March - O(nh)
- ✅ Graham Scan - O(n log n)
- ✅ Monotone Chain - O(n log n)
- ✅ Factory Pattern for algorithm selection
- ✅ Area, perimeter, and point-in-polygon tests

**Intersection Algorithms:**
- ✅ Bentley-Ottmann sweep line for line segments - O((n+k) log n)
- ✅ Convex polygon intersection (Linear Scan & Binary Search)
- ✅ Event queue and status line data structures

**Triangulation Algorithms:**
- ✅ Sweep line triangulation (top-to-bottom)
- ✅ Ear clipping method for simple polygons
- ✅ Delaunay triangulation with empty circumcircle property

**Voronoi Diagrams:**
- ✅ Voronoi cell generation from sites
- ✅ Dual relationship with Delaunay triangulation

**DCEL Data Structure:**
- ✅ Complete DCEL implementation (Vertex, HalfEdge, Face)
- ✅ DCEL Builder for planar subdivisions
- ✅ Efficient adjacency queries

**Spatial Indexing:**
- ✅ KD-Tree implementation for point data
- ✅ Quadtree for 2D space partitioning
- ✅ R-Tree for rectangle objects (GIS, games)
- ✅ BSP-Tree for binary space partitioning
- ✅ BVH for bounding volume hierarchy
- ✅ Range queries, nearest neighbor search
- ✅ Interactive visualization with multiple test patterns

**Visualization & Demos:**
- ✅ Interactive ImGui viewer with 8 demo scenes
- ✅ Real-time algorithm visualization
- ✅ Scene management system

**Documentation:**
- ✅ 8 comprehensive documentation files (algorithms, data structures)
- ✅ Code examples and usage patterns
- ✅ Architecture design documents

### 🚧 In Progress
- None

### 📋 Future Enhancements
- Fortune's sweep line algorithm for Voronoi (O(n log n))
- Half-plane intersection
- Circle-line intersection
- Additional geometric predicates
- Performance optimizations and benchmarks

## 文档

### Algorithm Documentation
- [Convex Hull Algorithms Comparison](docs/convex_hull_algorithms.md) - Jarvis March, Graham Scan, Monotone Chain
- [Bentley-Ottmann Algorithm](docs/bentley_ottmann_algorithm.md) - Line segment intersection with sweep line
- [Sweep Line: Bentley-Ottmann](docs/sweep_line_bentley_ottmann.md) - Detailed sweep line implementation
- [Ear Clipping Triangulation](docs/ear_clipping_triangulation.md) - Polygon triangulation algorithm
- [Sweep Line Triangulation](docs/sweep_line_triangulation.md) - Top-to-bottom triangulation approach
- [Convex Polygon Intersection](docs/convex_polygon_intersection.md) - Linear scan and binary search methods
- [Delaunay Triangulation](docs/delaunay_triangulation.md) - Bowyer-Watson algorithm with spatial optimization
- [Spatial Grid Optimization](docs/spatial_optimization.md) - Bucket-based acceleration for Delaunay

### Spatial Index Documentation
- [KD-Tree Algorithm](docs/kdtree_algorithm.md) - k-dimensional tree for spatial indexing
- [Quadtree Algorithm](docs/quadtree_algorithm.md) - 2D space partitioning tree
- [R-Tree Algorithm](docs/rtree_algorithm.md) - Rectangle-based spatial indexing
- [BSP-Tree Algorithm](docs/bsptree_algorithm.md) - Binary space partitioning
- [BVH Documentation](docs/BVH_Tree_Documentation.md) - Bounding Volume Hierarchy

### Data Structure Documentation
- [DCEL Data Structure](docs/dcel_data_structure.md) - Doubly Connected Edge List

## 代码风格

本项目遵循 **Google C++ Style Guide**：
- File naming: `snake_case.cc` and `snake_case.h`
- Class naming: `PascalCase`
- Function naming: `PascalCase`
- Member variables: `snake_case_` (with trailing underscore)

## 许可证

MIT License - Feel free to use for learning and projects.

---
