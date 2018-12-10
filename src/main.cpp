#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <iostream>

#include <shadertoy.hpp>
#include <shadertoy/utils/log.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <boost/filesystem.hpp>

#include "ui.hpp"
#include "uniforms.hpp"

#include "mvw/tiny_geometry.hpp"

using shadertoy::gl::gl_call;
using namespace shadertoy;

namespace fs = boost::filesystem;

// We need a custom inputs type to pass the MVP matrix
typedef shader_inputs<eMVP> geometry_inputs_t;

template<typename T>
std::unique_ptr<geometry::basic_geometry> make_geometry_object(const std::string &path, glm::vec3 &bbox_min, glm::vec3 &bbox_max, glm::dvec3 &centroid)
{
    auto g = std::make_unique<T>(path);
    g->get_dimensions(bbox_min, bbox_max);
    centroid = g->get_centroid();
    return std::move(g);
}

std::unique_ptr<geometry::basic_geometry> make_geometry(const std::string &path, glm::vec3 &bbox_min, glm::vec3 &bbox_max, glm::dvec3 &centroid) {
    fs::path p(path);
    auto ext(p.extension());
    std::unique_ptr<geometry::basic_geometry> result;

    if (ext == ".obj") {
        result = make_geometry_object<tiny_geometry>(path, bbox_min, bbox_max, centroid);
    } else {
        std::stringstream ss;
        ss << "Unsupported model type: " << ext;
        throw std::runtime_error(ss.str());
    }

    return result;
}

int main(int argc, char *argv[]) {
    int code = 0;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize glfw" << std::endl;
        return 2;
    }

    // Initialize window
    int width = 1280, height = 960;
    GLFWwindow *window = glfwCreateWindow(
        width, height, "Test model viewer", nullptr, nullptr);

    if (!window) {
        std::cerr << "Failed to create glfw window" << std::endl;
        code = 1;
    } else {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        utils::log::shadertoy()->set_level(spdlog::level::trace);

        try {
            example_ctx ctx;
            auto &context(ctx.context);
            auto &chain(ctx.chain);

            // Extra uniform inputs storage
            geometry_inputs_t extra_inputs;

            // Register the custom inputs with the buffer template
            context.buffer_template().shader_inputs().emplace("geometry",
                                                              &extra_inputs);

            // The default vertex shader is not sufficient, we replace it with
            // our own
            context.buffer_template()[GL_VERTEX_SHADER] =
                compiler::shader_template::parse_file(
                    "../shaders/20_vertex.glsl");

            // Force compilation of new template
            context.buffer_template().compile(GL_VERTEX_SHADER);

            // Load geometry
            const char *geometry_path = argc == 1 ? "../models/mcguire/bunny/bunny.obj" : argv[1];
            glm::vec3 bbox_min, bbox_max;
            glm::dvec3 centroid;
            std::shared_ptr<geometry::basic_geometry> geometry(make_geometry(geometry_path, bbox_min, bbox_max, centroid));

            glm::vec3 dimensions = bbox_max - bbox_min, center = (bbox_max + bbox_min) / 2.f;
            std::cout << "Object dimensions: " << glm::to_string(dimensions) << std::endl;
            std::cout << "Object center: " << glm::to_string(center) << std::endl;
            std::cout << "Object centroid: " << glm::to_string(centroid) << std::endl;

            float scale = 1. / dimensions.z;
 
            // Set the context parameters (render size and some uniforms)
            ctx.render_size = rsize(width, height);
            context.state().get<iTimeDelta>() = 1.0 / 60.0;
            context.state().get<iFrameRate>() = 60.0;

            // Create the image buffer
            auto imageBuffer(
                std::make_shared<buffers::geometry_buffer>("image"));
            imageBuffer->source_file("../shaders/shader-gradient.glsl");
            imageBuffer->geometry(geometry);

            // Without a background, the buffer should also clear the previous
            // contents
            imageBuffer->clear_color({.15f, .15f, .15f, 1.f});
            imageBuffer->clear_depth(1.f);
            imageBuffer->clear_bits(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Also we need depth test
            gl_call(glEnable, GL_DEPTH_TEST);

            // Add the image buffer to the swap chain, at the given size
            // The default_framebuffer policy makes this buffer draw directly to
            // the window instead of using a texture that is then copied to the
            // screen.
            chain.emplace_back(imageBuffer, make_size_ref(ctx.render_size),
                               member_swap_policy::default_framebuffer);

            // Initialize context
            context.init(chain);
            std::cout << "Initialized swap chain" << std::endl;

            // Now render for 5s
            int frameCount = 0;
            double t = 0.;

            // Set the resize callback
            glfwSetWindowUserPointer(window, &ctx);
            glfwSetFramebufferSizeCallback(
                window, example_set_framebuffer_size<example_ctx>);

            while (!glfwWindowShouldClose(window)) {
                // Poll events
                glfwPollEvents();

                // Update uniforms
                context.state().get<iTime>() = t;
                context.state().get<iFrame>() = frameCount;

                // Set viewport
                gl_call(glViewport, 0, 0, ctx.render_size.width, ctx.render_size.height);

                // Projection matrix display range : 0.1 unit <-> 100 units
                glm::mat4 Projection = glm::perspective(
                    glm::radians(25.0f), (float)ctx.render_size.width /
                                             (float)ctx.render_size.height,
                    0.1f, 100.0f);

                // Camera matrix
                glm::mat4 View = glm::lookAt(
                    glm::vec3(3, 3, 3),  // Location
                    glm::vec3(0, 0, 0),  // Target
                    glm::vec3(0, 1, 0)   // Up
                    );

                // Model matrix
                glm::mat4 Model = glm::rotate(
                    glm::translate(glm::scale(glm::mat4(1.f), glm::vec3(scale)), -center),
                    frameCount * 0.0125f,
                    glm::vec3(0.f, 1.f, 0.f));

                // Our ModelViewProjection : multiplication of our 3 matrices
                extra_inputs.get<eMVP>() = Projection * View * Model;

                // Render the swap chain
                context.render(chain);

                // Buffer swapping
                glfwSwapBuffers(window);

                // Update time and framecount
                t = glfwGetTime();
                frameCount++;

                if (libshadertoy_test_exit())
                    glfwSetWindowShouldClose(window, true);
            }
        } catch (gl::shader_compilation_error &sce) {
            std::cerr << "Failed to compile shader: " << sce.log();
            code = 2;
        } catch (shadertoy_error &err) {
            std::cerr << "Error: " << err.what();
            code = 2;
        }

        glfwDestroyWindow(window);
    }

    glfwTerminate();
    return code;
}
