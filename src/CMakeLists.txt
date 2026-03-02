# Geometry library CMakeLists.txt

# Collect all source files
file(GLOB_RECURSE geometry_sources
    "*.cc"
)

file(GLOB_RECURSE geometry_headers
    "*.h"
)

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
