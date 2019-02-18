#include <epoxy/gl.h>

#include <GLFW/glfw3.h>

#include <shadertoy.hpp>

#include <numeric>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "mvw/geometry.hpp"

#include "viewer_window.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using shadertoy::gl::gl_call;
using namespace shadertoy;

void viewer_window::compile_shader_source(const std::string &shader_path) {
    if (shader_path.empty())
        return;

    state_->log->info("Compiling {} using make", shader_path);

    int pid = fork();

    if (pid == 0) {
        size_t begin;
        if ((begin = shader_path.find_last_of('/')) != std::string::npos) {
            begin++;
        } else {
            begin = 0;
        }

        std::string basename(shader_path.begin() + begin, shader_path.end());
        const char *args[] = {"make", basename.c_str(), NULL};

        execvp("make", const_cast<char *const *>(args));
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
            throw std::runtime_error("Failed to compile shader source code");
        }
    }
}

void viewer_window::reload_shader() {
    // Recompile shaders
    if (use_make_) {
        compile_shader_source(shader_path_);
        compile_shader_source(postprocess_path_);
    }

    // Reinitialize chain
    gl_state_->load_chain(shader_path_, postprocess_path_);

    state_->log->info("Reloaded swap-chain");
}

viewer_window::viewer_window(std::shared_ptr<spd::logger> log, int width,
                             int height, const std::string &geometry_path,
                             const std::string &shader_path,
                             const std::string &postprocess_path, bool use_make,
                             const std::string &bind_addr)
    : server_{nullptr},
      shader_path_(shader_path),
      postprocess_path_(postprocess_path),
      use_make_(use_make),
      viewed_revision_(0) {
    window_ =
        glfwCreateWindow(width, height, "Test model viewer", nullptr, nullptr);

    if (!window_) {
        throw std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

    utils::log::shadertoy()->set_level(spdlog::level::debug);

    glfwSetWindowUserPointer(window_, this);

    // Set callbacks
    glfwSetMouseButtonCallback(window_, glfw_window_mouse_button_callback);
    glfwSetFramebufferSizeCallback(window_, glfw_window_set_framebuffer_size);
    glfwSetKeyCallback(window_, glfw_window_key_callback);
    glfwSetCharCallback(window_, glfw_window_char_callback);
    glfwSetCursorPosCallback(window_, glfw_window_cursor_pos_callback);
    glfwSetScrollCallback(window_, glfw_window_scroll_callback);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // Bind to Glfw+OpenGL3
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Set dark style
    ImGui::StyleColorsDark();

    // No rounded windows
    ImGui::GetStyle().WindowRounding = 0.0f;

    // Load static state
    state_ = std::make_unique<viewer_state>(log);

    // Compile shader source if requested
    if (use_make) {
        compile_shader_source(shader_path);
        compile_shader_source(postprocess_path);
    }

    // Load OpenGL dependent state
    gl_state_ = std::make_unique<gl_state>(log, width, height, geometry_path);

    // Load the initial state
    gl_state_->load_chain(shader_path, postprocess_path);

    // Compute model scale, update state
    //  Fetch dimensions of model
    glm::vec3 bbox_min, bbox_max;
    gl_state_->geometry->get_dimensions(bbox_min, bbox_max);
    glm::dvec3 centroid = gl_state_->geometry->get_centroid();

    glm::vec3 dimensions = bbox_max - bbox_min,
              center = (bbox_max + bbox_min) / 2.f;
    log->info("Object dimensions: {}", glm::to_string(dimensions));
    log->info("Object center: {}", glm::to_string(center));
    log->info("Object centroid: {}", glm::to_string(centroid));

    //  Set state_
    gl_state_->extra_inputs.get<bboxMax>() = bbox_max;
    gl_state_->extra_inputs.get<bboxMin>() = bbox_min;

    state_->center = center;
    state_->scale = 1. / dimensions.z;

    // Start server
    if (!bind_addr.empty())
        server_ = std::make_unique<net::server>(bind_addr);
}

void viewer_window::run() {
    // Rendering time
    double t = 0.;

    // Defaults
    gl_state_->extra_inputs.get<gTilesize>() = .1f / state_->scale;
    gl_state_->extra_inputs.get<gSplats>() = 3;
    gl_state_->extra_inputs.get<gF0>() = 1.0f;
    gl_state_->extra_inputs.get<cFilterLod>() = 2.0f;

    std::vector<float> runtime_acc(60, 0.0f);
    std::vector<float> runtime_p_acc(60, 0.0f);
    int runtime_acc_idx = -1;

    while (!glfwWindowShouldClose(window_)) {
        // Poll events
        glfwPollEvents();

        // Clear the default framebuffer (background for ImGui)
        gl_call(glViewport, 0, 0, window_width, gl_state_->render_size.height);
        gl_call(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, 0);
        gl_call(glClearColor, 0.0f, 0.0f, 0.0f, 1.0f);
        gl_call(glClearDepth, 1.f);
        gl_call(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(
            ImVec2(window_width, gl_state_->render_size.height));
        ImGui::Begin("mvw", NULL,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::Checkbox("Show wireframe", &state_->draw_wireframe);

        ImGui::Checkbox("Render as quad", &state_->draw_quad);

        bool show_grid = gl_state_->extra_inputs.get<dGrid>();
        ImGui::Checkbox("Show grid", &show_grid);
        gl_state_->extra_inputs.get<dGrid>() = show_grid ? 1 : 0;

        bool previous_rotate = state_->rotate_camera;
        ImGui::Checkbox("Rotate model", &state_->rotate_camera);
        state_->update_rotation(previous_rotate);

        ImGui::SliderInt("Revision", &viewed_revision_, -(gl_state_->chains.size() - 1), 0, "%d");

        ImGui::Separator();

        ImGui::SliderFloat("Tile size",
                           &gl_state_->extra_inputs.get<gTilesize>(), 0.01f,
                           10.0f, "%2.3f", 5.0f);
        ImGui::SliderInt("Splats", &gl_state_->extra_inputs.get<gSplats>(), 1,
                         30, "%d");
        ImGui::SliderFloat("F0", &gl_state_->extra_inputs.get<gF0>(), 0.001f,
                           100.0f, "%2.4f", 7.0f);

        ImGui::SliderAngle("W0.x", &gl_state_->extra_inputs.get<gW0>().x, 0.0f,
                           360.0f, "%2.2f");
        ImGui::SliderAngle("W0.y", &gl_state_->extra_inputs.get<gW0>().y, 0.0f,
                           360.0f, "%2.2f");

        ImGui::Separator();

        ImGui::SliderFloat("C. LOD", &gl_state_->extra_inputs.get<cFilterLod>(),
                           1.0f, 8.0f, "%2.2f");

        ImGui::Separator();

        char label_buf[30];
        char overlay_buf[30];

        {
            auto mean = std::accumulate(runtime_acc.begin(), runtime_acc.end(), 0.0f)
                / runtime_acc.size();
            sprintf(label_buf, "N %2.3fms", runtime_acc[runtime_acc_idx]);
            sprintf(overlay_buf, "a %2.3fms", mean);
            ImGui::PlotHistogram(label_buf, runtime_acc.data(), runtime_acc.size(), 0, overlay_buf);
        }

        if (!postprocess_path_.empty()) {
            auto mean = std::accumulate(runtime_p_acc.begin(), runtime_p_acc.end(), 0.0f)
                / runtime_acc.size();
            sprintf(label_buf, "P %2.3fms", runtime_p_acc[runtime_acc_idx]);
            sprintf(overlay_buf, "a %2.3fms", mean);
            ImGui::PlotHistogram(label_buf, runtime_p_acc.data(), runtime_p_acc.size(), 0, overlay_buf);
        }

        ImGui::End();

        // Complete render
        ImGui::Render();

        // Update uniforms
        gl_state_->context.state().get<iTime>() = t;
        gl_state_->context.state().get<iFrame>() = state_->frame_count;

        // Projection matrix display range : 0.1 unit <-> 100 units
        glm::mat4 Projection = glm::perspective(
            glm::radians(25.0f), (float)gl_state_->render_size.width /
                                     (float)gl_state_->render_size.height,
            0.1f, 100.0f);

        // Camera matrix
        glm::mat4 View = glm::lookAt(glm::vec3(5, 2, 0),  // Location
                                     glm::vec3(0, 0, 0),  // Target
                                     glm::vec3(0, 1, 0)   // Up
                                     );

        // Model matrix
        auto Model = glm::scale(glm::mat4(1.f), glm::vec3(state_->scale));
        // Y rotation
        Model = glm::rotate(Model, state_->get_rotation_y(),
                            glm::vec3(0.f, 1.f, 0.f));
        // X rotation
        Model = glm::rotate(Model, state_->get_rotation_x(),
                            glm::vec3(1.f, 0.f, 0.f));

        // Center model at origin
        Model = glm::translate(Model, -state_->center);

        // Our ModelViewProjection : multiplication of our 3 matrices
        gl_state_->extra_inputs.get<mModel>() = Model;
        gl_state_->extra_inputs.get<mView>() = View;
        gl_state_->extra_inputs.get<mProj>() = Projection;

        // Render current revision
        gl_state_->render(state_->draw_wireframe, state_->draw_quad, viewed_revision_);

        // Poll server instance for requests
        if (server_)
            server_->poll(*gl_state_, viewed_revision_);

        // Render ImGui overlay
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Buffer swapping
        glfwSwapBuffers(window_);

        // Measure time
        float ts[2];
        gl_state_->get_render_ms(ts, viewed_revision_);
        runtime_acc[(runtime_acc_idx = (runtime_acc_idx + 1) %
                     runtime_acc.size())] = ts[0];
        runtime_p_acc[runtime_acc_idx] = ts[1];

        // Update time and framecount
        t = glfwGetTime();
        state_->frame_count++;
    }
}

viewer_window::~viewer_window() {
    if (window_) {
        state_ = {};
        gl_state_ = {};

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window_);
    }
}

void viewer_window::glfw_mouse_button_callback(int button, int action,
                                               int mods) {
    // Ignore mouse clicks in window area
    if (state_->current_pos_x < window_width) return;

    if (action == GLFW_PRESS) {
        state_->pressed_buttons |= (1 << button);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            // The left button is pressed, stop rotating the camera by itself
            bool previous_rotate = state_->rotate_camera;
            state_->rotate_camera = false;
            state_->update_rotation(previous_rotate);
        }
    } else if (action == GLFW_RELEASE) {
        state_->pressed_buttons &= ~(1 << button);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            state_->apply_mouse_rotation();
        }
    }
}

void viewer_window::glfw_set_framebuffer_size(int width, int height) {
    gl_state_->render_size = shadertoy::rsize(width - window_width, height);
    gl_state_->allocate_textures();
}

void viewer_window::glfw_key_callback(int key, int scancode, int action,
                                      int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window_, true);
        } else if (key == GLFW_KEY_F5) {
            reload_shader();
        }
    }
}

void viewer_window::glfw_char_callback(unsigned int codepoint) {
    if (codepoint == 'w') {
        state_->draw_wireframe = !state_->draw_wireframe;
    } else if (codepoint == 'r') {
        reload_shader();
    }
}

void viewer_window::glfw_cursor_pos_callback(double xpos, double ypos) {
    state_->current_pos_x = xpos;
    state_->current_pos_y = ypos;

    if ((state_->pressed_buttons & (1 << GLFW_MOUSE_BUTTON_LEFT)) == 0) {
        state_->pressed_pos_x = xpos;
        state_->pressed_pos_y = ypos;
    }
}

void viewer_window::glfw_scroll_callback(double xoffset, double yoffset) {
    state_->scale += yoffset / 4.0 * state_->scale;
}

// Static callbacks
void viewer_window::glfw_window_mouse_button_callback(GLFWwindow *window,
                                                      int button, int action,
                                                      int mods) {
    reinterpret_cast<viewer_window *>(glfwGetWindowUserPointer(window))
        ->glfw_mouse_button_callback(button, action, mods);
}

void viewer_window::glfw_window_set_framebuffer_size(GLFWwindow *window,
                                                     int width, int height) {
    reinterpret_cast<viewer_window *>(glfwGetWindowUserPointer(window))
        ->glfw_set_framebuffer_size(width, height);
}

void viewer_window::glfw_window_key_callback(GLFWwindow *window, int key,
                                             int scancode, int action,
                                             int mods) {
    reinterpret_cast<viewer_window *>(glfwGetWindowUserPointer(window))
        ->glfw_key_callback(key, scancode, action, mods);
}

void viewer_window::glfw_window_char_callback(GLFWwindow *window,
                                              unsigned int codepoint) {
    reinterpret_cast<viewer_window *>(glfwGetWindowUserPointer(window))
        ->glfw_char_callback(codepoint);
}

void viewer_window::glfw_window_cursor_pos_callback(GLFWwindow *window,
                                                    double xpos, double ypos) {
    reinterpret_cast<viewer_window *>(glfwGetWindowUserPointer(window))
        ->glfw_cursor_pos_callback(xpos, ypos);
}

void viewer_window::glfw_window_scroll_callback(GLFWwindow *window,
                                                double xoffset,
                                                double yoffset) {
    reinterpret_cast<viewer_window *>(glfwGetWindowUserPointer(window))
        ->glfw_scroll_callback(xoffset, yoffset);
}

