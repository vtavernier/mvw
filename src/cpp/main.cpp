#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <iostream>

#include <boost/program_options.hpp>

#include "viewer_window.hpp"

using namespace shadertoy;

namespace po = boost::program_options;

int main(int argc, char *argv[]) {
    int code = 0;

    // Initialize logger
    auto log = spd::stderr_color_st("viewer");

    std::string geometry_path;
    std::string shader_path;
    int width, height;

    // clang-format off
    po::options_description v_desc("Viewer options");
    v_desc.add_options()
        ("geometry,g", po::value(&geometry_path)->default_value("../models/mcguire/bunny/bunny.obj"), "Path to the geometry to load")
        ("shader,s", po::value(&shader_path)->default_value("glsl/gabor-noise-surface.glsl"), "Path to the shader program to use")
        ("width,W", po::value(&width)->default_value(1280), "Window width")
        ("height,H", po::value(&height)->default_value(960), "Window height")
        ("help,h", "Show this help message");
    // clang-format on

    po::positional_options_description p;
    p.add("geometry", 1);
    p.add("shader", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv)
                      .options(v_desc)
                      .positional(p)
                      .run(),
                  vm);
        po::notify(vm);
    } catch (po::error &ex) {
        std::cerr << "Invalid usage: " << ex.what() << std::endl
                  << v_desc << std::endl;
        return 1;
    }

    if (vm.count("help") > 0) {
        std::cout << v_desc << std::endl;
        return 0;
    }

    if (!glfwInit()) {
        log->critical("Failed to initialize glfw");
        return 2;
    }

    // Initialize window
    try {
        viewer_window window(log, width, height, geometry_path, shader_path);
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