#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <random>
#include <sstream>

#ifndef __EMSCRIPTEN__
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#endif

#include "viewer_window.hpp"

using namespace shadertoy;
namespace gx = shadertoy::backends::gx;

std::string default_bind_addr() {
    std::random_device rd;
    std::mt19937 mt(rd());

#if _WIN32
    std::uniform_int_distribution<int32_t> dist(32768, 65535);
    return "tcp://127.0.0.1:" + std::to_string(dist(mt));
#else
    static const char rnd[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::uniform_int_distribution<int32_t> dist(0, sizeof(rnd) - 1);
    char name[17];
    for (size_t i = 0; i < sizeof(name) - 1; ++i) name[i] = rnd[dist(mt)];
    name[sizeof(name) - 1] = '\0';

    std::stringstream ss;
    char *tmpdir = std::getenv("TMPDIR");
    ss << "ipc://" << (tmpdir ? tmpdir : "/tmp") << "/mvw_" << name << ".sock";
    return ss.str();
#endif /* _WIN32 */
}

#ifndef __EMSCRIPTEN__
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
    auto level = opt.log.debug ? spdlog::level::debug
                               : (opt.log.verbose ? spdlog::level::info
                                                  : spdlog::level::warn);
    utils::log::shadertoy()->set_level(level);
    VLOG->set_level(level);

    // Initialize window
    try {
        viewer_window window(std::move(opt));
        window.run();
    } catch (gx::shader_compilation_error &sce) {
        VLOG->critical("Failed to compile shader: {}", sce.log());
        code = 2;
    } catch (gx::program_link_error &sce) {
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
#else
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

int main(int argc, char *argv[]) {
    // Initialize logger
    spdlog::stderr_color_st(VLOG_NAME);

    if (!glfwInit()) {
        VLOG->critical("Failed to initialize glfw");
        return 2;
    }

    return 0;
}

void run_viewer(const viewer_options &opt) {
    // Set log levels
    auto level = spdlog::level::debug;
    utils::log::shadertoy()->set_level(level);
    VLOG->set_level(level);

    viewer_window::run_opt(opt);
}

EMSCRIPTEN_BINDINGS(viewer) {
    value_object<viewer_options>("ViewerOptions")
        .field("program", &viewer_options::program)
        .field("geometry", &viewer_options::geometry)
        .field("frame", &viewer_options::frame);

    function("start", &run_viewer);

    value_object<shader_program_options>("ShaderProgramOptions")
        .field("shader", &shader_program_options::shader)
        .field("postprocess", &shader_program_options::postprocess);

    value_object<shader_file_program>("ShaderFileProgram")
        .field("path", &shader_file_program::path)
        .field("source", &shader_file_program::source);

    value_object<geometry_options>("GeometryOptions")
        .field("path", &geometry_options::path)
        .field("nff_source", &geometry_options::nff_source);

    value_object<frame_options>("FrameOptions")
        .field("width", &frame_options::width)
        .field("height", &frame_options::height);
}
#endif
