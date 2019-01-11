cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(test-models LANGUAGES CXX)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/libshadertoy/modules
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Set CMake standard
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

# pthread
find_package(Threads REQUIRED)

# OpenGL libraries
find_package(OpenGL REQUIRED)
find_package(Epoxy REQUIRED)
find_package(Glfw3 REQUIRED)

# Boost
find_package(Boost 1.58 REQUIRED COMPONENTS filesystem program_options)

# libshadertoy
add_subdirectory(libshadertoy)

# Assimp library
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT CACHE INTERNAL OFF)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_STL_IMPORTER ON)
set(ASSIMP_BUILD_NFF_IMPORTER ON)
add_subdirectory(assimp EXCLUDE_FROM_ALL)

# ImGui has no CMakeLists.txt for now, so manually make a library out of it
set(IMGUI_BUILD_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/imgui)
file(MAKE_DIRECTORY ${IMGUI_BUILD_DIRECTORY})
set(IMGUI_FILES
    imconfig.h
    imgui.cpp
    imgui.h
    imgui_draw.cpp
    imgui_internal.h
    imgui_widgets.cpp
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h
    examples/imgui_impl_glfw.h
    examples/imgui_impl_glfw.cpp
    examples/imgui_impl_opengl3.h
    examples/imgui_impl_opengl3.cpp)
foreach(IMGUI_FILE ${IMGUI_FILES})
    get_filename_component(IMGUI_FILE_BASENAME ${IMGUI_FILE} NAME)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/imgui/${IMGUI_FILE}
        ${IMGUI_BUILD_DIRECTORY}/${IMGUI_FILE_BASENAME}
        COPYONLY)
    list(APPEND IMGUI_SOURCES ${IMGUI_BUILD_DIRECTORY}/${IMGUI_FILE_BASENAME})
endforeach()
add_library(imgui STATIC ${IMGUI_SOURCES})
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_CUSTOM=<epoxy/gl.h>)
target_include_directories(imgui PUBLIC ${IMGUI_BUILD_DIRECTORY})

# Directories
set(INCLUDE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SHADERS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

# Define targets
add_subdirectory(${SRC_ROOT}/cpp)
