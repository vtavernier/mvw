#ifndef _VIEWER_WINDOW_HPP_
#define _VIEWER_WINDOW_HPP_

#include <memory>

#include "log.hpp"

#include "viewer_state.hpp"

#include "mvw/mvw_geometry.hpp"

class viewer_window {
    GLFWwindow *window_;
    std::unique_ptr<viewer_state> state_;

    std::shared_ptr<shadertoy::buffers::geometry_buffer> geometry_buffer_;
    std::shared_ptr<shadertoy::members::basic_member> geometry_target_;
    std::shared_ptr<shadertoy::buffers::toy_buffer> postprocess_buffer_;
    std::shared_ptr<shadertoy::members::screen_member> main_screen_;

    std::shared_ptr<mvw_geometry> geometry_;
    const std::string &shader_path_;
    const std::string &postprocess_path_;
    const bool use_make_;

    static void glfw_window_mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
    static void glfw_window_set_framebuffer_size(GLFWwindow *window, int width, int height);
    static void glfw_window_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void glfw_window_char_callback(GLFWwindow *window, unsigned int codepoint);
    static void glfw_window_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
    static void glfw_window_scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

    void glfw_mouse_button_callback(int button, int action, int mods);
    void glfw_set_framebuffer_size(int width, int height);
    void glfw_key_callback(int key, int scancode, int action, int mods);
    void glfw_char_callback(unsigned int codepoint);
    void glfw_cursor_pos_callback(double xpos, double ypos);
    void glfw_scroll_callback(double xoffset, double yoffset);

    void compile_shader_source(const std::string &shader_path);

    void reload_shader();

   public:
    viewer_window(std::shared_ptr<spd::logger> log, int width, int height,
                  const std::string &geometry_path, const std::string &shader_path,
                  const std::string &postprocess_path, bool use_make);

    void run();

    ~viewer_window();
};

#endif /* _VIEWER_WINDOW_HPP_ */
