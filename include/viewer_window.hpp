#ifndef _VIEWER_WINDOW_HPP_
#define _VIEWER_WINDOW_HPP_

#include <memory>

#include <shadertoy.hpp>
#include "log.hpp"
#include "uniforms.hpp"
#include "viewer_state.hpp"

#include "mvw/mvw_geometry.hpp"

#include "gl_state.hpp"
#include "net/server.hpp"

class viewer_window {
    GLFWwindow *window_;
    std::unique_ptr<viewer_state> state_;
    std::unique_ptr<gl_state> gl_state_;
    std::unique_ptr<net::server> server_;

    const std::string &shader_path_;
    const std::string &postprocess_path_;
    const bool use_make_;

    int viewed_revision_;

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

    void compile_and_discover_uniforms();

  public:
    viewer_window(std::shared_ptr<spd::logger> log, int width, int height,
                  const std::string &geometry_path, const std::string &shader_path,
                  const std::string &postprocess_path, bool use_make,
                  const std::string &bind_addr);

    void run();

    ~viewer_window();
};

#endif /* _VIEWER_WINDOW_HPP_ */
