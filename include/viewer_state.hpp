#ifndef _VIEWER_STATE_HPP_
#define _VIEWER_STATE_HPP_

#include <shadertoy/utils/log.hpp>

#include <glm/glm.hpp>

#include "config.hpp"
#include "log.hpp"

struct viewer_state {
    bool draw_wireframe;
    bool draw_quad;
    bool rotate_camera;
    glm::vec2 user_rotate;
    double pressed_pos_x;
    double pressed_pos_y;
    double current_pos_x;
    double current_pos_y;
    int pressed_buttons;
    int frame_count;
    glm::vec3 center;
    float scale;

    glm::vec3 camera_location;
    glm::vec3 camera_target;
    glm::vec3 camera_up;

    viewer_state();

    void update_rotation(bool previous_rotate);

    glm::vec2 get_rotation(int frame_factor) const;

    void apply_mouse_rotation();

    glm::mat4 get_model() const;

    glm::mat4 get_view() const;
};

#endif /* _VIEWER_STATE_HPP_ */
