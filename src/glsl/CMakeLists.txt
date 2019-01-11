set(GLSL_SRC ${CMAKE_CURRENT_SOURCE_DIR})
set(GLSL_OUTPUT ${CMAKE_BINARY_DIR}/glsl)

add_custom_target(glsl-output-dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GLSL_OUTPUT})

file(GLOB GLSL_SOURCES ${GLSL_SRC}/*.glsl)
set(GLSL_OUTPUT_FILES "")
foreach (GLSL_FILE ${GLSL_SOURCES})
    get_filename_component(GLSL_FILE_BASE ${GLSL_FILE} NAME)
    set(GLSL_OUTPUT_FILE "${GLSL_OUTPUT}/${GLSL_FILE_BASE}")

    # Run command to find out dependencies
    execute_process(COMMAND npm run --silent glslify-deps -- -b ${GLSL_SRC} ${GLSL_FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GLSL_DEPENDENCIES_RESULT
        OUTPUT_VARIABLE GLSL_DEPENDENCIES_OUTPUT
        ERROR_VARIABLE  GLSL_DEPENDENCIES_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE)

    # Only add dependencies on success
    if (GLSL_DEPENDENCIES_RESULT EQUAL 0)
        string(REGEX REPLACE "\n" ";" GLSL_DEPENDENCIES_OUTPUT "${GLSL_DEPENDENCIES_OUTPUT}")
        message(STATUS "Adding glslify target for ${GLSL_FILE}: ${GLSL_DEPENDENCIES_OUTPUT}")
    else()
        message(WARNING "Failed to parse dependencies for ${GLSL_FILE} (${GLSL_DEPENDENCIES_RESULT}): ${GLSL_DEPENDENCIES_ERROR}")
        set(GLSL_DEPENDENCIES_OUTPUT "")
        message(STATUS "Adding glslify target for ${GLSL_FILE}")
    endif()

    add_custom_command(OUTPUT ${GLSL_OUTPUT_FILE}
        COMMAND glslify ${GLSL_FILE} -o ${GLSL_OUTPUT_FILE}
        MAIN_DEPENDENCY ${GLSL_FILE}
        DEPENDS "${GLSL_DEPENDENCIES_OUTPUT};glsl-output-dir"
        WORKING_DIRECTORY ${GLSL_SRC})

    list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT_FILE})
endforeach()

add_custom_target(glsl ALL DEPENDS ${GLSL_OUTPUT_FILES})
