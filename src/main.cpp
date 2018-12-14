#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <iostream>

#include <shadertoy.hpp>
#include <shadertoy/utils/log.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "uniforms.hpp"

#include "mvw/geometry.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using shadertoy::gl::gl_call;
using namespace shadertoy;

namespace spd = spdlog;

struct viewer_state {
    bool draw_wireframe;
    int pressed_buttons;

    shadertoy::render_context context;
    shadertoy::swap_chain chain;
    shadertoy::rsize render_size;

    viewer_state()
        : draw_wireframe(false), pressed_buttons(0), context(), chain(), render_size()
    {}
};

const int window_width = 200;

void glfw_mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    auto &state = *reinterpret_cast<viewer_state*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS) {
        state.pressed_buttons |= button;
    } else if (action == GLFW_RELEASE) {
        state.pressed_buttons &= ~button;
    }
}

void glfw_set_framebuffer_size(GLFWwindow *window, int width, int height) {
    auto &state = *reinterpret_cast<viewer_state*>(glfwGetWindowUserPointer(window));

    state.render_size = shadertoy::rsize(width, height);
    state.context.allocate_textures(state.chain);
}

void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto &state = *reinterpret_cast<viewer_state*>(glfwGetWindowUserPointer(window)); (void)state;

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
        }
    }
}

void glfw_char_callback(GLFWwindow *window, unsigned int codepoint) {
    auto &state = *reinterpret_cast<viewer_state*>(glfwGetWindowUserPointer(window));

    if (codepoint == 'w') {
        state.draw_wireframe = !state.draw_wireframe;
    }
}

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
    GLFWwindow *window = glfwCreateWindow(
        width, height, "Test model viewer", nullptr, nullptr);

    if (!window) {
        log->critical("Failed to create glfw window");
        code = 1;
    } else {
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        utils::log::shadertoy()->set_level(spdlog::level::info);

        try {
            viewer_state state;
            glfwSetWindowUserPointer(window, &state);

            // Set callbacks
            glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
            glfwSetFramebufferSizeCallback(window, glfw_set_framebuffer_size);
            glfwSetKeyCallback(window, glfw_key_callback);
            glfwSetCharCallback(window, glfw_char_callback);

            // Initialize ImGui
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;

            // Bind to Glfw+OpenGL3
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init("#version 130");

            // Set dark style
            ImGui::StyleColorsDark();

            // No rounded windows
            ImGui::GetStyle().WindowRounding = 0.0f;

            // Get reference to context and swap chain
            auto &context(state.context);
            auto &chain(state.chain);

            // Extra uniform inputs storage
            geometry_inputs_t extra_inputs;

            // Register the custom inputs with the buffer template
            context.buffer_template().shader_inputs().emplace("geometry",
                                                              &extra_inputs);

            // The default vertex shader is not sufficient, we replace it with our own
            context.buffer_template()[GL_VERTEX_SHADER] =
                compiler::shader_template::parse_file(
                    "../shaders/vertex.glsl");

            // Same for fragment shader
            context.buffer_template()[GL_FRAGMENT_SHADER] =
                compiler::shader_template::parse_file(
                    "../shaders/fragment.glsl");

            // Force compilation of new template
            context.buffer_template().compile(GL_VERTEX_SHADER);

            // Load geometry
            const char *geometry_path = argc == 1 ? "../models/mcguire/bunny/bunny.obj" : argv[1];
            log->info("Loading model {}", geometry_path);
            std::shared_ptr<mvw_geometry> geometry(make_geometry(geometry_path));

            // Fetch dimensions of model
            glm::vec3 bbox_min, bbox_max;
            geometry->get_dimensions(bbox_min, bbox_max);
            glm::dvec3 centroid = geometry->get_centroid();

            glm::vec3 dimensions = bbox_max - bbox_min, center = (bbox_max + bbox_min) / 2.f;
            log->info("Object dimensions: {}", glm::to_string(dimensions));
            log->info("Object center: {}", glm::to_string(center));
            log->info("Object centroid: {}", glm::to_string(centroid));

            // Compute model scale
            float scale = 1. / dimensions.z;
 
            // Set the context parameters (render size and some uniforms)
            state.render_size = rsize(width, height);
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
            gl_call(glDepthFunc, GL_LEQUAL);

            // Smooth wireframe
            gl_call(glEnable, GL_LINE_SMOOTH);

            // Add the image buffer to the swap chain, at the given size
            // The default_framebuffer policy makes this buffer draw directly to
            // the window instead of using a texture that is then copied to the
            // screen.
            chain.emplace_back(imageBuffer, make_size_ref(state.render_size),
                               member_swap_policy::default_framebuffer);

            // Initialize context
            context.init(chain);
            log->info("Initialized swap chain");

            // Now render for 5s
            int frameCount = 0;
            double t = 0.;

            while (!glfwWindowShouldClose(window)) {
                // Poll events
                glfwPollEvents();

                // Start ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                ImGui::SetNextWindowPos(ImVec2(0, 0));
                ImGui::SetNextWindowSize(ImVec2(window_width, state.render_size.height));
                ImGui::Begin("mvw", NULL,
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove);

                ImGui::Checkbox("Show wireframe", &state.draw_wireframe);

                ImGui::End();

                // Complete render
                ImGui::Render();

                // Update uniforms
                context.state().get<iTime>() = t;
                context.state().get<iFrame>() = frameCount;

                // Set viewport
                gl_call(glViewport, 0, 0, state.render_size.width, state.render_size.height);

                // Projection matrix display range : 0.1 unit <-> 100 units
                glm::mat4 Projection = glm::perspective(
                    glm::radians(25.0f), (float)state.render_size.width /
                                             (float)state.render_size.height,
                    0.1f, 100.0f);

                // Camera matrix
                glm::mat4 View = glm::lookAt(
                    glm::vec3(5, 2, 0),  // Location
                    glm::vec3(0, 0.5, 0),  // Target
                    glm::vec3(0, 1, 0)   // Up
                    );

                // Model matrix
                glm::mat4 Model = glm::rotate(
                    glm::translate(glm::scale(glm::mat4(1.f), glm::vec3(scale)), -center),
                    frameCount * 0.0125f,
                    glm::vec3(0.f, 1.f, 0.f));

                // Our ModelViewProjection : multiplication of our 3 matrices
                extra_inputs.get<mModel>() = Model;
                extra_inputs.get<mView>() = View;
                extra_inputs.get<mProj>() = Projection;
                extra_inputs.get<bWireframe>() = GL_FALSE;

                // First call: clear everything
                imageBuffer->clear_bits(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                // Render the swap chain
                gl_call(glPolygonMode, GL_FRONT_AND_BACK, GL_FILL);
                context.render(chain);

                if (state.draw_wireframe) {
                    // Second call: render wireframe on top
                    extra_inputs.get<bWireframe>() = GL_TRUE;
                    imageBuffer->clear_bits(0);
                    // Render swap chain
                    gl_call(glPolygonMode, GL_FRONT_AND_BACK, GL_LINE);
                    context.render(chain);
                }

                // Render ImGui overlay
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                // Buffer swapping
                glfwSwapBuffers(window);

                // Update time and framecount
                t = glfwGetTime();
                frameCount++;
            }
        } catch (gl::shader_compilation_error &sce) {
            log->critical("Failed to compile shader: {}", sce.log());
            code = 2;
        } catch (shadertoy_error &err) {
            log->critical("GL error: {}", err.what());
            code = 2;
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
    }

    glfwTerminate();
    return code;
}
