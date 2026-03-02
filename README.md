# BeyondConvex - Computational Geometry Library

A modern C++ computational geometry library with interactive visualization using Dear ImGui.

## Features

- ✅ Modern C++17 code following Google C++ Style Guide
- ✅ Interactive ImGui-based visualization
- ✅ Modular architecture for easy extension
- ✅ Cross-platform support (Windows, Linux, macOS)

## Project Structure

```
BeyondConvex/
├── CMakeLists.txt              # Main build configuration
├── src/                        # Core library
│   ├── core/                   # Basic geometry classes
│   │   ├── point.h/cc          # Point class
│   │   └── geometry_utils.h    # Utility functions
│   ├── convex_hull/            # Convex hull algorithms
│   ├── intersection/           # Intersection algorithms
│   ├── triangulation/          # Triangulation algorithms
│   └── voronoi/               # Voronoi diagrams
├── viewer/                     # ImGui visualization
│   ├── main.cc                # Entry point with ImGui
│   └── demos/                 # Algorithm demonstrations
└── third_party/               # External dependencies
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

### ✅ Stage 1: Foundation (Complete)
- Point class
- Basic vector operations
- Cross product and dot product
- ImGui visualization setup

### 🚧 Stage 2: Convex Hull (Next)set
- **View → Demo Window** to see ImGui capabilities

### Code Example

```cpp
#include "core/point.h"
#include "core/geometry_utils.h"

using namespace geometry;

// Create points
Point p1(0.0, 0.0);
Point p2(3.0, 4.0);

// Calculate distance
double dist = p1.DistanceTo(p2);  // Returns 5.0

// Calculate cross product
Point p3(1.0, 0.0);
Point p4(0.0, 1.0);
double cross = CrossProduct(p3, p4);  // Returns 1.0
```

## Learning Path
## Dependencies

- **GLFW** - Window and input management (auto-downloaded by CMake)
- **OpenGL** - Graphics rendering
- **Dear ImGui** - Immediate mode GUI framework (included in third_party)

## Current Status

✅ Basic project structure created
✅ Point class implemented
✅ Geometry utilities (cross product, dot product)
✅ CMake build system configured
✅ ImGui + GLFW integration complete
✅ Interactive viewer working

🚧 Next: Implement Graham Scan convex hull algorithm

## License2: Convex Hull (Next)
- Graham Scan algorithm
- Andrew's Monotone Chain
- Rotating Calipers

### Stage 3: Intersection
- Line segment intersection
- Circle-line intersection
- Half-plane intersection

### Stage 4: Triangulation
- Polygon triangulation
- Delaunay triangulation

### Stage 5: Voronoi Diagrams
- Fortune's sweep line algorithm
- Dual relationship with Delaunay

## Code Style

This project follows **Google C++ Style Guide**:
- File naming: `snake_case.cc` and `snake_case.h`
- Class naming: `PascalCase`
- Function naming: `PascalCase`
- Member variables: `snake_case_` (with trailing underscore)

## Current Status

✅ Basic project structure created
✅ Point class implemented
✅ Geometry utilities (cross product, dot product)
✅ CMake build system configured
✅ Basic console viewer working

🚧 Next: Add ImGui for interactive visualization

## License

MIT License - Feel free to use for learning and projects.
