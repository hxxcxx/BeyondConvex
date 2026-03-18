# Geometry library CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE geometry_sources
    "*.cc"
)

file(GLOB_RECURSE geometry_headers
    "*.h"
)

# Temporarily exclude problematic triangulation files
list(FILTER geometry_sources EXCLUDE REGEX ".*delaunay_triangulation_optimized\\.cc$")
list(FILTER geometry_sources EXCLUDE REGEX ".*ear_clipping_triangulation\\.cc$")
list(FILTER geometry_sources EXCLUDE REGEX ".*sweep_line_triangulation\\.cc$")
list(FILTER geometry_sources EXCLUDE REGEX ".*triangulation_utils\\.cc$")

# Create geometry library (include headers for IDE)
add_library(geometry_lib 
    ${geometry_sources}
    ${geometry_headers}
)

# Set include directories
target_include_directories(geometry_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler warnings
if(MSVC)
    target_compile_options(geometry_lib PRIVATE /W4)
else()
    target_compile_options(geometry_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()
