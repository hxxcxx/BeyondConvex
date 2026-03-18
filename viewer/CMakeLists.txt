# Viewer CMakeLists.txt

# Collect viewer source files
file(GLOB_RECURSE viewer_sources
    "*.cc"
)

file(GLOB_RECURSE viewer_headers
    "*.h"
)

# Collect demo source files
file(GLOB_RECURSE demo_sources
    "demos/*.cc"
)

file(GLOB_RECURSE demo_headers
    "demos/*.h"
)

# Exclude benchmark files (they have their own main)
list(FILTER viewer_sources EXCLUDE REGEX ".*delaunay_benchmark\\.cc$")
list(FILTER demo_sources EXCLUDE REGEX ".*delaunay_benchmark\\.cc$")

# Create viewer executable (include headers for IDE)
add_executable(geometry_viewer 
    ${viewer_sources} 
    ${viewer_headers}
    ${demo_sources}
    ${demo_headers}
)

# Link with libraries
target_link_libraries(geometry_viewer PRIVATE
    geometry_lib
    imgui_lib
    OpenGL::GL
)

# Set include directories
target_include_directories(geometry_viewer PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Compiler warnings
if(MSVC)
    target_compile_options(geometry_viewer PRIVATE /W4)
else()
    target_compile_options(geometry_viewer PRIVATE -Wall -Wextra -Wpedantic)
endif()
