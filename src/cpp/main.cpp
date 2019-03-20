#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>

#include <boost/program_options.hpp>

#include "viewer_window.hpp"

using namespace shadertoy;

namespace po = boost::program_options;

std::string default_bind_addr() {
#if _WIN32
    return "tcp://127.0.0.1:7178";
#else
    std::stringstream ss;
    char *tmpdir = std::getenv("TMPDIR");
    ss << "ipc://" << (tmpdir ? tmpdir : "/tmp") << "/mvw_default.sock";
    return ss.str();
#endif /* _WIN32 */
}

int main(int argc, char *argv[]) {
    int code = 0;

    // Initialize logger
    spdlog::stderr_color_st(VLOG_NAME);

    viewer_options opt;

    // clang-format off
    po::options_description v_desc("Viewer options");
    v_desc.add_options()
        /* program options */
        ("shader-file,s", po::value(&opt.program.shader.path), "Path to the shader program")
        ("shader,S", po::value(&opt.program.shader.source), "Source of the shader program")
        ("postprocess-file,p", po::value(&opt.program.postprocess.path), "Path to the postprocessing shader")
        ("postprocess,P", po::value(&opt.program.postprocess.source), "Source of the postprocessing shader")
        ("use-make,m", po::bool_switch(&opt.program.use_make), "Compile the target shader file using make first")
        /* geometry */
        ("geometry-file,g", po::value(&opt.geometry.path), "Path to the geometry to load")
        ("geometry,G", po::value(&opt.geometry.nff_source), "NFF format string of the geometry to use")
        /* frame options */
        ("width,W", po::value(&opt.frame.width)->default_value(512), "Frame width")
        ("height,H", po::value(&opt.frame.height)->default_value(512), "Frame height")
        /* server options */
        ("bind,b", po::value(&opt.server.bind_addr)->default_value(default_bind_addr()), "Server bind address")
        /* log options */
        ("debug,d", po::bool_switch(&opt.log.debug)->default_value(false), "Enable debug logs")
        ("verbose,v", po::bool_switch(&opt.log.verbose)->default_value(false), "Enable verbose logs")
        /* viewer options */
        ("headless,q", po::bool_switch(&opt.headless_mode)->default_value(false), "Headless renderer mode")
        /* misc */
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
        VLOG->critical("Failed to initialize glfw");
        return 2;
    }

    // Set log levels
    auto level = opt.log.debug ? spdlog::level::debug :
        (opt.log.verbose ? spdlog::level::info : spdlog::level::warn);
    utils::log::shadertoy()->set_level(level);
    VLOG->set_level(level);

    // Initialize window
    try {
        viewer_window window(std::move(opt));
        window.run();
    } catch (gl::shader_compilation_error &sce) {
        VLOG->critical("Failed to compile shader: {}", sce.log());
        code = 2;
    } catch (gl::program_link_error &sce) {
        VLOG->critical("Failed to link program: {}", sce.log());

        code = 2;
    } catch (shadertoy_error &err) {
        VLOG->critical("GL error: {}", err.what());
        code = 2;
    } catch (std::runtime_error &ex) {
        VLOG->critical("Generic error: {}", ex.what());
        code = 1;
    }

    glfwTerminate();
    return code;
}
