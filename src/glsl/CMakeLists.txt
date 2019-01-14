set(GLSL_SRC ${CMAKE_CURRENT_SOURCE_DIR})
set(GLSL_OUTPUT ${CMAKE_BINARY_DIR}/glsl)

add_custom_target(glsl-output-dir
    COMMAND ${CMAKE_COMMAND} -E make_directory ${GLSL_OUTPUT})

file(GLOB GLSL_SOURCES ${GLSL_SRC}/*.glsl)
set(GLSL_OUTPUT_FILES "")
foreach(GLSL_FILE ${GLSL_SOURCES})
    get_filename_component(GLSL_FILE_BASE ${GLSL_FILE} NAME)
    set(GLSL_OUTPUT_FILE "${GLSL_OUTPUT}/${GLSL_FILE_BASE}")


    # Run command to find out dependencies
    execute_process(COMMAND sh -c "m4 -P -di ${GLSL_FILE} 2>&1 >/dev/null | grep -Po '(?<=input read from ).*$'"
        WORKING_DIRECTORY ${GLSL_SRC}
        RESULT_VARIABLE GLSL_DEPENDENCIES_RESULT
        OUTPUT_VARIABLE GLSL_DEPENDENCIES_OUTPUT
        ERROR_VARIABLE  GLSL_DEPENDENCIES_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE)

    # Only add dependencies on success
    if(GLSL_DEPENDENCIES_RESULT EQUAL 0)
        string(REGEX REPLACE "\n" ";" GLSL_DEPENDENCIES_OUTPUT "${GLSL_DEPENDENCIES_OUTPUT}")
        list(REMOVE_AT GLSL_DEPENDENCIES_OUTPUT 0)
    else()
        message(WARNING "Failed to parse dependencies for ${GLSL_FILE} (${GLSL_DEPENDENCIES_RESULT}): ${GLSL_DEPENDENCIES_ERROR}")
        set(GLSL_DEPENDENCIES_OUTPUT "")
    endif()

    set(GLSL_DEPS "")
    foreach(GLSL_DEP ${GLSL_DEPENDENCIES_OUTPUT})
        set(ABSOLUTE_PATH "${GLSL_SRC}/${GLSL_DEP}")
        if(EXISTS "${ABSOLUTE_PATH}")
            list(APPEND GLSL_DEPS "${ABSOLUTE_PATH}")
        else()
            list(APPEND GLSL_DEPS "${GLSL_DEP}")
        endif()
    endforeach()

    if(GLSL_DEPS EQUAL "")
        message(STATUS "Adding glslify target for ${GLSL_FILE}")
    else()
        message(STATUS "Adding glslify target for ${GLSL_FILE}: ${GLSL_DEPS}")
    endif()

    add_custom_command(OUTPUT ${GLSL_OUTPUT_FILE}
        COMMAND m4
        ARGS -P ${GLSL_FILE} > ${GLSL_OUTPUT_FILE}
        MAIN_DEPENDENCY ${GLSL_FILE}
        DEPENDS "${GLSL_DEPS};glsl-output-dir"
        WORKING_DIRECTORY ${GLSL_SRC})

    list(APPEND GLSL_OUTPUT_FILES ${GLSL_OUTPUT_FILE})
endforeach()

add_custom_target(glsl ALL DEPENDS ${GLSL_OUTPUT_FILES})
