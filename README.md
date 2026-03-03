# BeyondConvex - Computational Geometry Library

A modern C++ computational geometry library with interactive visualization using Dear ImGui.

## Features

- ✅ Modern C++17 code following Google C++ Style Guide
- ✅ Interactive ImGui-based visualization
- ✅ Modular architecture with facade pattern
- ✅ Multiple convex hull algorithms implemented
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
│   ├── convex_hull/            # Convex hull module
│   │   ├── convex_hull.h/cc    # ConvexHull class
│   │   ├── convex_hull_builder.h/cc  # Algorithm implementations
│   │   └── convex_hull_factory.h/cc  # Factory pattern
│   ├── intersection/           # Intersection algorithms (TODO)
│   ├── triangulation/          # Triangulation algorithms (TODO)
│   └── voronoi/               # Voronoi diagrams (TODO)
├── viewer/                     # ImGui visualization
│   ├── main.cc                # Entry point
│   ├── scene_manager.h/cc     # Scene management
│   ├── geometry_canvas.h/cc   # Rendering canvas
│   └── demos/                 # Algorithm demonstrations
│       ├── scene_base.h       # Base scene class
│       └── convex_hull/       # Convex hull scenes
│           ├── to_left_test_scene.h/cc
│           ├── incremental_construction_scene.h/cc
│           └── convex_hull_scene.h/cc
├── docs/                       # Documentation
│   └── convex_hull_algorithms.md  # Algorithm comparison
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

- **Scenes Menu** (top menu bar): Switch between different algorithm demonstrations
- **Scene Control** (left panel): Select scene and reset
- **Scene Info** (left panel): View scene details and adjust parameters
- **Canvas** (right panel): Click to add points and visualize algorithms

### Code Example

```cpp
#include "geometry_utils.h"
#include "convex_hull/convex_hull_factory.h"
#include "core/point2d.h"

using namespace geometry;

// Create points
std::vector<Point2D> points = {
  {0.0, 0.0},
  {3.0, 4.0},
  {1.0, 1.0}
};

// Build convex hull using Monotone Chain (recommended)
ConvexHull hull = ConvexHullFactory::Create(
  ConvexHullAlgorithm::MonotoneChain,
  points
);

// Get hull properties
std::cout << "Hull vertices: " << hull.Size() << std::endl;
std::cout << "Area: " << hull.Area() << std::endl;
std::cout << "Perimeter: " << hull.Perimeter() << std::endl;

// Check if point is inside hull
Point2D test_point(1.0, 0.5);
bool inside = hull.Contains(test_point);

// Use To-Left Test
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
- Interactive visualization scenes

### 🚧 Stage 3: Intersection (Next)
- Line segment intersection
- Circle-line intersection
- Half-plane intersection

### 📋 Stage 4: Triangulation (TODO)
- Polygon triangulation
- Delaunay triangulation

### 📋 Stage 5: Voronoi Diagrams (TODO)
- Fortune's sweep line algorithm
- Dual relationship with Delaunay

## Dependencies

- **GLFW** - Window and input management (auto-downloaded by CMake)
- **OpenGL** - Graphics rendering
- **Dear ImGui** - Immediate mode GUI framework (included in third_party)

## Current Status

### ✅ Completed
- Basic project structure with CMake
- Point2D, Vector2D, Edge2D classes
- GeometryUtils (public facade) and GeometryCore (internal implementation)
- Three convex hull algorithms: Jarvis March, Graham Scan, Monotone Chain
- ConvexHull class with area, perimeter, and point-in-polygon tests
- Factory pattern for algorithm selection
- Interactive ImGui viewer with multiple scenes
- Comprehensive algorithm documentation

### 🚧 In Progress
- None

### 📋 TODO
- Intersection algorithms
- Triangulation algorithms
- Voronoi diagrams
- More geometric predicates and algorithms

## Documentation

- [Convex Hull Algorithms Comparison](docs/convex_hull_algorithms.md) - Detailed comparison of three implemented algorithms
- [Library Export Architecture](docs/library_export_architecture.md) - Design documentation for the facade pattern

## Code Style

This project follows **Google C++ Style Guide**:
- File naming: `snake_case.cc` and `snake_case.h`
- Class naming: `PascalCase`
- Function naming: `PascalCase`
- Member variables: `snake_case_` (with trailing underscore)

## License

MIT License - Feel free to use for learning and projects.
