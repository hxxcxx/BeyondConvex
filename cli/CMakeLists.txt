# CLI CMakeLists.txt
# C++/CLI wrapper for BeyondConvex library
# Note: CMake has limited C++/CLI support, recommend using VS directly

if(WIN32)
    message(STATUS "C++/CLI support: Use Visual Studio to build cli/BeyondConvex.CLI.vcxproj")
else()
    message(STATUS "C++/CLI is Windows-only, skipping CLI project")
endif()
