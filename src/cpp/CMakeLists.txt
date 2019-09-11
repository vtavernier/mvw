set(SRC ${CMAKE_CURRENT_SOURCE_DIR})

# Create viewer library
file(GLOB MVW_SOURCES ${SRC}/mvw/*.cpp ${INCLUDE_ROOT}/mvw/*.hpp)
add_library(mvw ${MVW_SOURCES})

target_include_directories(mvw PUBLIC ${INCLUDE_ROOT})
target_include_directories(mvw PRIVATE ${Boost_INCLUDE_DIRS})
if(NOT EMSCRIPTEN)
    target_link_libraries(mvw PUBLIC shadertoy-shared stbackend-gl4-shared assimp)
else()
    target_link_libraries(mvw PUBLIC shadertoy stbackend-webgl assimp)
endif()
target_compile_options(mvw PRIVATE -Wall;-Werror=return-type)

# Create viewer target
file(GLOB VIEWER_SOURCES ${SRC}/*.cpp ${INCLUDE_ROOT}/*.hpp ${INCLUDE_ROOT}/detail/*.hpp)
file(GLOB VIEWER_NET_SOURCES ${SRC}/net/*.cpp ${INCLUDE_ROOT}/net/*.hpp)
if(NOT EMSCRIPTEN)
    set(VIEWER_SOURCES ${VIEWER_SOURCES} ${VIEWER_NET_SOURCES})
endif()
add_executable(viewer ${VIEWER_SOURCES})

target_include_directories(viewer PRIVATE
    ${INCLUDE_ROOT}
    ${ZeroMQ_INCLUDE_DIRS})

target_link_libraries(viewer PRIVATE
    mvw
    imgui
    ${Glfw3_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES})

if(NOT EMSCRIPTEN)
    target_link_libraries(viewer PRIVATE
        ${ZeroMQ_LIBRARIES}
        msgpackc-cxx)
endif()

if(EMSCRIPTEN)
    target_compile_options(viewer PRIVATE
        "SHELL:--bind")

    target_link_options(viewer PRIVATE
        "SHELL:--bind"
        "SHELL:-s USE_WEBGL2"
        "SHELL:-s NO_EXIT_RUNTIME=1"
        "SHELL:-s DISABLE_EXCEPTION_CATCHING=0"
        "SHELL:-s USE_GLFW=3"
        "SHELL:-s EXPORTED_FUNCTIONS='[\"_main\"]'"
        "SHELL:-s EXTRA_EXPORTED_RUNTIME_METHODS='[\"cwrap\"]'"
        "SHELL:--embed-file ${SHADERS_ROOT}/vertex.glsl@vertex.glsl"
        "SHELL:--embed-file ${SHADERS_ROOT}/fragment.glsl@fragment.glsl")
endif()

target_compile_options(viewer PRIVATE -Wall;-Werror=return-type)

# Output into main folder
set_target_properties(viewer PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

target_compile_definitions(viewer PUBLIC GLM_ENABLE_EXPERIMENTAL)

if(NOT EMSCRIPTEN)
    target_compile_definitions(viewer PUBLIC SHADERS_BASE="${SHADERS_ROOT}/")
else()
    target_compile_definitions(viewer PUBLIC SHADERS_BASE="")
endif()
