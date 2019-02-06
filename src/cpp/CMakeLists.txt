set(SRC ${CMAKE_CURRENT_SOURCE_DIR})

# Create viewer library
file(GLOB MVW_SOURCES ${SRC}/mvw/*.cpp)
add_library(mvw ${MVW_SOURCES})

target_include_directories(mvw PUBLIC ${INCLUDE_ROOT})
target_include_directories(mvw PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(mvw PUBLIC shadertoy-shared ${Boost_filesystem_LIBRARY} assimp)
target_compile_options(mvw PRIVATE -Wall;-Werror=return-type)

# Create viewer target
file(GLOB VIEWER_SOURCES ${SRC}/*.cpp)
add_executable(viewer ${VIEWER_SOURCES})

target_include_directories(viewer PRIVATE ${INCLUDE_ROOT})
target_link_libraries(viewer PRIVATE mvw imgui ${Glfw3_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
target_compile_options(viewer PRIVATE -Wall;-Werror=return-type)

# Output into main folder
set_target_properties(viewer PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

target_compile_definitions(viewer PUBLIC GLM_ENABLE_EXPERIMENTAL)
