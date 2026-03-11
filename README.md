# BeyondConvex - Computational Geometry Library

<div align="center">

A modern C++17 computational geometry library with interactive visualization using Dear ImGui.

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()

**Features:** Convex Hull • Intersection • Triangulation • Voronoi • DCEL

</div>

## Overview

BeyondConvex is a comprehensive computational geometry library that implements fundamental algorithms with interactive visualization. It provides a clean, modern C++17 interface following the Google C++ Style Guide, making it ideal for learning, research, and practical applications.

## Algorithm Summary

| Category | Algorithm | Time Complexity | Space Complexity | Status |
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

## Features

- ✅ Modern C++17 code following Google C++ Style Guide
- ✅ Interactive ImGui-based visualization with 8 demo scenes
- ✅ Modular architecture with facade pattern
- ✅ Complete computational geometry algorithms suite
- ✅ DCEL (Doubly Connected Edge List) data structure
- ✅ Cross-platform support (Windows, Linux, macOS)

## Project Structure

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
│   │   └── convex_polygon_intersection.h/cc  # Convex polygon
│   ├── triangulation/          # ✅ Triangulation algorithms
│   │   └── triangulation.h/cc  # Sweep line, Ear clipping, Delaunay
│   ├── dcel/                   # ✅ DCEL data structure
│   │   ├── dcel.h/cc           # Main DCEL class
│   │   ├── vertex.h/cc         # Vertex records
│   │   ├── half_edge.h/cc      # Half-edge records
│   │   ├── face.h/cc           # Face records
│   │   └── dcel_builder.h/cc   # DCEL builder
│   └── voronoi/               # ✅ Voronoi diagrams
│       └── voronoi_diagram.h/cc  # Voronoi generation
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
│       └── dcel/              # ✅ DCEL test scene (1)
│           └── dcel_test_scene.h/cc
├── docs/                       # ✅ Comprehensive documentation
│   ├── convex_hull_algorithms.md
│   ├── bentley_ottmann_algorithm.md
│   ├── sweep_line_bentley_ottmann.md
│   ├── ear_clipping_triangulation.md
│   ├── sweep_line_triangulation.md
│   ├── convex_polygon_intersection.md
│   └── library_export_architecture.md
└── third_party/               # External dependencies
    ├── glfw/                  # GLFW framework
    └── imgui/                 # Dear ImGui framework
```

## Building

### Requirements
- CMake 3.15+
- C++17 compatible compiler (MSVC, GCC, Clang)
- OpenGL 3.0+

### Build Steps

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

## Usage

### Running the Viewer

```bash
# Build the project
cd build
cmake --build . --config Release

# Run the viewer
./bin/Release/geometry_viewer.exe  # Windows
./bin/Release/geometry_viewer      # Linux/macOS
```

### Viewer Controls

- **Scenes Menu** (top menu bar): Switch between 8 different algorithm demonstrations
- **Scene Control** (left panel): Select scene and reset
- **Scene Info** (left panel): View scene details and adjust parameters
- **Canvas** (right panel): Click to add points and visualize algorithms

### Available Demo Scenes

1. **To-Left Test** - Interactive geometric predicate testing
2. **Incremental Construction** - Step-by-step convex hull building
3. **Convex Hull** - Compare three convex hull algorithms
4. **Line Segment Intersection** - Bentley-Ottmann sweep line visualization
5. **Convex Polygon Intersection** - Polygon intersection algorithms
6. **Triangulation** - Sweep line, ear clipping, and Delaunay methods
7. **Voronoi Diagram** - Voronoi cell generation from sites
8. **DCEL Test** - Doubly Connected Edge List data structure

### Code Example

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

// ========== To-Left Test ==========
Point2D p(0.0, 0.0), q(1.0, 0.0), r(0.5, 0.5);
bool is_left = GeometryUtils::ToLeftTest(p, q, r);  // true
```

## Learning Path

### ✅ Stage 1: Foundation (Complete)
- Point2D, Vector2D, Edge2D classes
- To-Left Test (fundamental geometric predicate)
- Facade pattern architecture (GeometryUtils/GeometryCore)
- ImGui visualization setup

### ✅ Stage 2: Convex Hull (Complete)
- **Jarvis March** (Gift Wrapping) - O(nh)
- **Graham Scan** - O(n log n)
- **Monotone Chain** - O(n log n) ⭐ Recommended
- Factory pattern for algorithm selection
- Interactive visualization scenes (3 demos)

### ✅ Stage 3: Intersection (Complete)
- **Line Segment Intersection** - Bentley-Ottmann O((n+k) log n)
- **Convex Polygon Intersection** - Linear Scan O(n+m) & Binary Search O(log n+log m)
- Interactive visualization scenes (2 demos)

### ✅ Stage 4: Triangulation (Complete)
- **Sweep Line Triangulation** - Top-to-bottom approach
- **Ear Clipping Method** - O(n²) for simple polygons
- **Delaunay Triangulation** - Empty circumcircle property
- Interactive visualization scene (1 demo)

### ✅ Stage 5: Voronoi Diagrams (Complete)
- **Voronoi Diagram Generation** - Site-based cell construction
- Relationship with Delaunay triangulation (dual graphs)
- Interactive visualization scene (1 demo)

### ✅ Stage 6: DCEL Data Structure (Complete)
- **DCEL Implementation** - Vertex, HalfEdge, Face records
- **DCEL Builder** - Construct planar subdivisions
- Interactive test scene (1 demo)

### 📋 Future Enhancements
- Fortune's sweep line algorithm for Voronoi (O(n log n))
- Half-plane intersection
- Circle-line intersection
- More geometric predicates

## Dependencies

- **GLFW** - Window and input management (auto-downloaded by CMake)
- **OpenGL** - Graphics rendering
- **Dear ImGui** - Immediate mode GUI framework (included in third_party)

## Current Status

### ✅ Completed (All Major Features)

**Core Infrastructure:**
- ✅ Modern C++17 project with CMake build system
- ✅ Point2D, Vector2D, Edge2D geometric primitives
- ✅ Facade pattern architecture (GeometryUtils/GeometryCore)
- ✅ Google C++ Style Guide compliance

**Convex Hull Algorithms:**
- ✅ Jarvis March (Gift Wrapping) - O(nh)
- ✅ Graham Scan - O(n log n)
- ✅ Monotone Chain - O(n log n)
- ✅ Factory pattern for algorithm selection
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

**Visualization & Demos:**
- ✅ Interactive ImGui viewer with 8 demo scenes
- ✅ Real-time algorithm visualization
- ✅ Scene management system

**Documentation:**
- ✅ 8 comprehensive documentation files (algorithms, data structures, architecture)
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

## Documentation

### Algorithm Documentation
- [Convex Hull Algorithms Comparison](docs/convex_hull_algorithms.md) - Jarvis March, Graham Scan, Monotone Chain
- [Bentley-Ottmann Algorithm](docs/bentley_ottmann_algorithm.md) - Line segment intersection with sweep line
- [Sweep Line: Bentley-Ottmann](docs/sweep_line_bentley_ottmann.md) - Detailed sweep line implementation
- [Ear Clipping Triangulation](docs/ear_clipping_triangulation.md) - Polygon triangulation algorithm
- [Sweep Line Triangulation](docs/sweep_line_triangulation.md) - Top-to-bottom triangulation approach
- [Convex Polygon Intersection](docs/convex_polygon_intersection.md) - Linear scan and binary search methods

### Data Structure Documentation
- [DCEL Data Structure](docs/dcel_data_structure.md) - Doubly Connected Edge List (双向连接边表)

### Architecture Documentation
- [Library Export Architecture](docs/library_export_architecture.md) - Facade pattern design documentation

## Code Style

This project follows **Google C++ Style Guide**:
- File naming: `snake_case.cc` and `snake_case.h`
- Class naming: `PascalCase`
- Function naming: `PascalCase`
- Member variables: `snake_case_` (with trailing underscore)

## Project Statistics

- **Total Lines of Code:** ~5,000+
- **Core Algorithms:** 10+ implementations
- **Demo Scenes:** 8 interactive visualizations
- **Documentation Files:** 8 comprehensive guides
- **Supported Platforms:** Windows, Linux, macOS

## License

MIT License - Feel free to use for learning and projects.

---

<div align="center">

**Built with ❤️ for the computational geometry community**

[Report Bug](../../issues) · [Request Feature](../../issues)

</div>
