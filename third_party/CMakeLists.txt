# Third-party libraries CMakeLists.txt
# This file manages all third-party dependencies without modifying their repos

# ============================================================================
# ImGui Library
# ============================================================================

# ImGui sources
set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

# ImGui headers
set(IMGUI_HEADERS
    imgui/imgui.h
    imgui/imgui_internal.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    imgui/backends/imgui_impl_glfw.h
    imgui/backends/imgui_impl_opengl3.h
    imgui/backends/imgui_impl_opengl3_loader.h
)

# Create ImGui library
add_library(imgui_lib ${IMGUI_SOURCES} ${IMGUI_HEADERS})

# Set include directories
target_include_directories(imgui_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

# Link with GLFW
target_link_libraries(imgui_lib PUBLIC glfw)

# C++ standard
target_compile_features(imgui_lib PUBLIC cxx_std_17)

# ============================================================================
# GLFW Library
# ============================================================================

# Use GLFW's own CMake build system
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)

# Create alias for easier linking (glfw3 -> glfw)
add_library(glfw3 ALIAS glfw)
