#ifndef _VIEWER_WINDOW_HPP_
#define _VIEWER_WINDOW_HPP_

#include <memory>

#include <shadertoy.hpp>
#include "log.hpp"
#include "viewer_state.hpp"

#include "mvw/mvw_geometry.hpp"

#include "gl_state.hpp"
#include "net/server.hpp"

#include "options.hpp"

class viewer_window {
    GLFWwindow *window_;
    std::unique_ptr<viewer_state> state_;
    std::unique_ptr<gl_state> gl_state_;
    std::unique_ptr<net::server> server_;

    viewer_options opt_;

    shadertoy::rsize window_render_size_;

    int viewed_revision_;

    bool need_render_;

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

    void reload_shader();

  public:
   viewer_window(viewer_options opt);
   ~viewer_window();

   void run();
};

#endif /* _VIEWER_WINDOW_HPP_ */
