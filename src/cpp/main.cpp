#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <iostream>

#include "viewer_window.hpp"

using namespace shadertoy;

int main(int argc, char *argv[]) {
    int code = 0;

    // Initialize logger
    auto log = spd::stderr_color_st("viewer");

    if (!glfwInit()) {
        log->critical("Failed to initialize glfw");
        return 2;
    }

    // Initialize window
    int width = 1280, height = 960;
    try {
        viewer_window window(log, width, height, "../models/mcguire/bunny/bunny.obj");
        window.run();
    } catch (gl::shader_compilation_error &sce) {
        log->critical("Failed to compile shader: {}", sce.log());
        code = 2;
    } catch (shadertoy_error &err) {
        log->critical("GL error: {}", err.what());
        code = 2;
    } catch (std::runtime_error &ex) {
        log->critical("Generic error: {}", ex.what());
        code = 1;
    }

    glfwTerminate();
    return code;
}
