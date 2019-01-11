#include <epoxy/gl.h>

#include "viewer_state.hpp"

viewer_state::viewer_state(std::shared_ptr<spd::logger> log)
    : draw_wireframe(false),
      rotate_camera(true),
      user_rotate_x(0.0f),
      user_rotate_y(0.0f),
      pressed_pos_x(0.),
      pressed_pos_y(0.),
      current_pos_x(0.),
      current_pos_y(0.),
      pressed_buttons(0),
      frame_count(0),
      center(0.f),
      scale(0.f),
      context(),
      chain(),
      render_size(),
      extra_inputs(),
      log(log) {}

void viewer_state::reload() { context.init(chain); }

void viewer_state::update_rotation(bool previous_rotate) {
    if (previous_rotate && !rotate_camera)
        user_rotate_y = fmod(get_rotation_y(1), 2. * M_PI) / rotate_speed;
    else if (!previous_rotate && rotate_camera)
        user_rotate_y = fmod(get_rotation_y(-1), 2. * M_PI) / rotate_speed;
}

float viewer_state::get_rotation_x() {
    return (user_rotate_x + mouse_speed * (current_pos_y - pressed_pos_y)) *
           rotate_speed;
}

float viewer_state::get_rotation_y() {
    return get_rotation_y(rotate_camera ? 1 : 0);
}

float viewer_state::get_rotation_y(int frame_factor) {
    return ((frame_count * frame_factor) + user_rotate_y +
            mouse_speed * (current_pos_x - pressed_pos_x)) *
           rotate_speed;
}

void viewer_state::apply_mouse_rotation() {
    user_rotate_x += mouse_speed * (current_pos_y - pressed_pos_y);
    user_rotate_y += mouse_speed * (current_pos_x - pressed_pos_x);
    pressed_pos_x = current_pos_x;
    pressed_pos_y = current_pos_y;
}
