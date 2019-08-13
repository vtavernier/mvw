#include "viewer_state.hpp"

#include <glm/gtc/matrix_transform.hpp>

viewer_state::viewer_state()
    : draw_wireframe(false),
      rotate_camera(false),
      user_rotate(0),
      pressed_pos_x(0.),
      pressed_pos_y(0.),
      current_pos_x(0.),
      current_pos_y(0.),
      pressed_buttons(0),
      frame_count(0),
      center(0.f),
      scale(0.f),
      camera_location(5, 2, 0),
      camera_target(0, 0, 0),
      camera_up(0, 1, 0) {}

void viewer_state::update_rotation(bool previous_rotate) {
    if (previous_rotate && !rotate_camera)
        user_rotate.y = fmod(get_rotation(1).y, 2. * M_PI) / rotate_speed;
    else if (!previous_rotate && rotate_camera)
        user_rotate.y = fmod(get_rotation(-1).y, 2. * M_PI) / rotate_speed;
}

glm::vec2 viewer_state::get_rotation(int frame_factor) const {
    return glm::vec2(
               (user_rotate.x + mouse_speed * (current_pos_y - pressed_pos_y)),
               ((frame_count * frame_factor) + user_rotate.y +
                mouse_speed * (current_pos_x - pressed_pos_x))) *
           rotate_speed;
}

void viewer_state::apply_mouse_rotation() {
    user_rotate.x += mouse_speed * (current_pos_y - pressed_pos_y);
    user_rotate.y += mouse_speed * (current_pos_x - pressed_pos_x);
    pressed_pos_x = current_pos_x;
    pressed_pos_y = current_pos_y;
}

glm::mat4 viewer_state::get_model() const {
    auto model = glm::scale(glm::mat4(1.f), glm::vec3(scale));
    auto rot = get_rotation(rotate_camera ? 1 : 0);

    // Y rotation
    model = glm::rotate(model, rot.y, glm::vec3(0.f, 1.f, 0.f));
    // X rotation
    model = glm::rotate(model, rot.x, glm::vec3(1.f, 0.f, 0.f));

    // Center model at origin
    model = glm::translate(model, -center);

    return model;
}

glm::mat4 viewer_state::get_view() const {
    return glm::lookAt(camera_location,  // Location
                       camera_target,    // Target
                       camera_up         // Up
    );
}
