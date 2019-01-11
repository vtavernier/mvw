set(GLSL_SRC ${CMAKE_CURRENT_SOURCE_DIR})
set(GLSL_OUTPUT ${CMAKE_BINARY_DIR}/glsl)

add_custom_target(glsl-output-dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GLSL_OUTPUT})

file(GLOB GLSL_SOURCES ${GLSL_SRC}/*.glsl)
set(GLSL_OUTPUT_FILES "")
foreach (GLSL_FILE ${GLSL_SOURCES})
    message(STATUS "Adding glslify target for ${GLSL_FILE}")
    get_filename_component(GLSL_FILE_BASE ${GLSL_FILE} NAME)
    set(GLSL_OUTPUT_FILE "${GLSL_OUTPUT}/${GLSL_FILE_BASE}")

    add_custom_command(OUTPUT ${GLSL_OUTPUT_FILE}
        COMMAND glslify ${GLSL_FILE} -o ${GLSL_OUTPUT_FILE}
        MAIN_DEPENDENCY ${GLSL_FILE}
        DEPENDS glsl-output-dir
        WORKING_DIRECTORY ${GLSL_SRC})

    list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT_FILE})
endforeach()

add_custom_target(glsl ALL
    DEPENDS ${GLSL_OUTPUT_FILES})
