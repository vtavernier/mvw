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

using shadertoy::gl::gl_call;
using namespace shadertoy;

void viewer_window::reload_shader() {
    // Reinitialize chain
    gl_state_->load_chain(opt_.program);
    VLOG->info("Reloaded swap-chain");
    need_render_ = true;
}

viewer_window::viewer_window(viewer_options &&opt)
    : server_{nullptr},
      opt_{opt},
      window_render_size_(opt_.frame.width, opt_.frame.height),
      viewed_revision_(0),
      need_render_(true) {

    // Hide windows in headless mode
    if (opt_.headless_mode)
        glfwWindowHint(GLFW_VISIBLE, 0);

    window_ =
        glfwCreateWindow(opt_.frame.width + window_width, opt_.frame.height,
                         "Test model viewer", nullptr, nullptr);

    if (!window_) {
        throw std::runtime_error("Failed to create window");
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);

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
    state_ = std::make_unique<viewer_state>();

    // Load OpenGL dependent state
    gl_state_ = std::make_unique<gl_state>(opt_.frame);

    // Load the initial state
    gl_state_->load_chain(opt_.program);

    // Load the geometry
    gl_state_->load_geometry(opt_.geometry);
    state_->center = gl_state_->center;
    state_->scale = gl_state_->scale;

    // Start server
    if (!opt_.server.bind_addr.empty())
        server_ = std::make_unique<net::server>(opt_.server, opt_.log);
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

void viewer_window::run() {
    // Rendering time
    double t = 0.;

    std::vector<float> runtime_acc(60, 0.0f);
    std::vector<float> runtime_p_acc(60, 0.0f);
    int runtime_acc_idx = 0;

    while (!glfwWindowShouldClose(window_)) {
        // Poll events
        glfwPollEvents();

        // Clear the default framebuffer (background for ImGui)
        gl_call(glViewport, 0, 0, window_width, window_render_size_.height);
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
            ImVec2(window_width, window_render_size_.height));
        ImGui::Begin("mvw", NULL,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        ImGui::Text("Camera settings");

        need_render_ |= ImGui::InputFloat3("Location", &state_->camera_location.x);
        need_render_ |= ImGui::InputFloat3("Target", &state_->camera_target.x);
        need_render_ |= ImGui::InputFloat3("Up", &state_->camera_up.x);

        need_render_ |= ImGui::InputFloat2("Rotation", &state_->user_rotate.x);
        need_render_ |= ImGui::InputFloat("Scale", &state_->scale);

        ImGui::Separator();

        ImGui::Text("Render settings");

        need_render_ |=
            ImGui::Checkbox("Show wireframe", &state_->draw_wireframe);

        if (ImGui::Button("Resize")) {
            gl_state_->render_size = window_render_size_;
            gl_state_->allocate_textures();

            need_render_ = true;
        }

        bool previous_rotate = state_->rotate_camera;
        ImGui::Checkbox("Rotate model", &state_->rotate_camera);
        state_->update_rotation(previous_rotate);

        need_render_ |=
            ImGui::SliderInt("Revision", &viewed_revision_,
                             -(gl_state_->chains.size() - 1), 0, "%d");

        ImGui::Separator();

        need_render_ |= gl_state_->render_imgui(viewed_revision_);

        ImGui::Text("Performance");

        char label_buf[30];
        char overlay_buf[30];

        {
            auto mean =
                std::accumulate(runtime_acc.begin(), runtime_acc.end(), 0.0f) /
                runtime_acc.size();
            sprintf(label_buf, "N %2.3fms", runtime_acc[runtime_acc_idx]);
            sprintf(overlay_buf, "a %2.3fms", mean);
            ImGui::PlotHistogram(label_buf, runtime_acc.data(),
                                 runtime_acc.size(), 0, overlay_buf);
        }

        if (gl_state_->has_postprocess(viewed_revision_)) {
            auto mean = std::accumulate(runtime_p_acc.begin(),
                                        runtime_p_acc.end(), 0.0f) /
                        runtime_acc.size();
            sprintf(label_buf, "P %2.3fms", runtime_p_acc[runtime_acc_idx]);
            sprintf(overlay_buf, "a %2.3fms", mean);
            ImGui::PlotHistogram(label_buf, runtime_p_acc.data(),
                                 runtime_p_acc.size(), 0, overlay_buf);
        }

        ImGui::Text("Status");

        if (!gl_state_->chains.empty()) {
            ImGui::Text("%s", gl_state_->chains.back()->error_status.c_str());
        }

        ImGui::End();

        // Complete render
        ImGui::Render();

        // Update uniforms
        gl_state_->update_uniforms(t, *state_);

        // Note that if rotation is enabled we need to render every frame
        need_render_ |= state_->rotate_camera;

        // Render current revision
        gl_state_->render(state_->draw_wireframe, viewed_revision_,
                          need_render_);

        // We updated the rendering to the latest version
        bool has_renderered = need_render_;
        need_render_ = false;

        // Poll server instance for requests
        if (server_) {
            need_render_ |=
                server_->poll(*state_, *gl_state_, viewed_revision_);
        }

        // Render ImGui overlay
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Buffer swapping
        glfwSwapBuffers(window_);

        if (has_renderered) {
            // Measure time
            float ts[2];
            gl_state_->get_render_ms(ts, viewed_revision_);
            runtime_acc[(runtime_acc_idx = (runtime_acc_idx + 1) %
                                           runtime_acc.size())] = ts[0];
            runtime_p_acc[runtime_acc_idx] = ts[1];
        }

        // Update time and framecount
        t = glfwGetTime();
        state_->frame_count++;
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
    bool match_size = window_render_size_ == gl_state_->render_size;

    window_render_size_ = shadertoy::rsize(width - window_width, height);

    if (match_size) {
        gl_state_->render_size = window_render_size_;
        gl_state_->allocate_textures();
    }

    need_render_ = true;
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
        need_render_ = true;
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
    } else {
        need_render_ = true;
    }
}

void viewer_window::glfw_scroll_callback(double xoffset, double yoffset) {
    state_->scale += yoffset / 4.0 * state_->scale;

    need_render_ = true;
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

