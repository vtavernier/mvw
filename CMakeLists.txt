cmake_minimum_required(VERSION 2.8)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(test-models LANGUAGES CXX)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/libshadertoy/modules
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Set CMake standard
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

# OpenGL libraries
find_package(OpenGL REQUIRED)
find_package(Epoxy REQUIRED)
find_package(Glfw3 REQUIRED)

# Boost
find_package(Boost 1.58 REQUIRED COMPONENTS filesystem)

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

# Directories
set(INCLUDE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SHADERS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/shaders)

# Download tiny_obj_loader
file(DOWNLOAD https://raw.githubusercontent.com/syoyo/tinyobjloader/v1.0.6/tiny_obj_loader.h
    ${CMAKE_CURRENT_BINARY_DIR}/ext/tiny_obj_loader.h
    EXPECTED_HASH SHA256=740c4e262a6ac56940852671bbe3e5e162cba7325c59ef57f3d9efdba2c5244c)
file(DOWNLOAD https://raw.githubusercontent.com/syoyo/tinyobjloader/v1.0.6/tiny_obj_loader.cc
    ${CMAKE_CURRENT_BINARY_DIR}/ext/tiny_obj_loader.cc
    EXPECTED_HASH SHA256=b27322737fbd97b615412efd404d20d4a9b0a6556cada15ca4b77c7447b38bd2)

# Create viewer library (used for the viewer and for testing)
file(GLOB MVW_SOURCES ${SRC_ROOT}/mvw/*.cpp)
add_library(mvw ${MVW_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/ext/tiny_obj_loader.cc)

target_include_directories(mvw PUBLIC ${INCLUDE_ROOT})
target_include_directories(mvw PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/ext ${Boost_INCLUDE_DIRS})
target_link_libraries(mvw PUBLIC shadertoy-shared ${Boost_filesystem_LIBRARY} assimp)
target_compile_options(mvw PRIVATE -Wall;-Werror=return-type)

# Create viewer target
file(GLOB VIEWER_SOURCES ${SRC_ROOT}/*.cpp)
add_executable(viewer ${VIEWER_SOURCES})

target_include_directories(viewer PRIVATE ${INCLUDE_ROOT})
target_link_libraries(viewer PRIVATE mvw ${Glfw3_LIBRARY})
target_compile_options(viewer PRIVATE -Wall;-Werror=return-type)
