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

# Collect GLFW source files
file(GLOB GLFW_SOURCES
    glfw/src/context.c
    glfw/src/init.c
    glfw/src/input.c
    glfw/src/monitor.c
    glfw/src/vulkan.c
    glfw/src/window.c
    glfw/src/osmesa_context.c
    glfw/src/egl_context.c
)

# Platform-specific sources
if(WIN32)
    file(GLOB GLFW_PLATFORM_SOURCES
        glfw/src/win32_*.c
        glfw/src/wgl_context.c
    )
elseif(APPLE)
    file(GLOB GLFW_PLATFORM_SOURCES
        glfw/src/cocoa_*.m
        glfw/src/nsgl_context.m
    )
else()
    file(GLOB GLFW_PLATFORM_SOURCES
        glfw/src/x11_*.c
        glfw/src/glx_context.c
        glfw/src/linux-*.c
        glfw/src/posix_*.c
    )
endif()

# Create GLFW library
add_library(glfw3 STATIC ${GLFW_SOURCES} ${GLFW_PLATFORM_SOURCES})
set_target_properties(glfw3 PROPERTIES
    LINKER_LANGUAGE C
)

# GLFW include directories
target_include_directories(glfw3 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/glfw/include
    ${CMAKE_CURRENT_SOURCE_DIR}/glfw/src
)

# Link system libraries
if(WIN32)
    target_link_libraries(glfw3 PUBLIC gdi32)
elseif(APPLE)
    find_library(OPENGL_LIBRARY OpenGL)
    target_link_libraries(glfw3 PUBLIC ${OPENGL_LIBRARY}
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
else()
    target_link_libraries(glfw3 PUBLIC GL dl pthread m X11)
endif()

# Create alias for easier linking
add_library(glfw ALIAS glfw3)
