#ifndef _VIEWER_STATE_HPP_
#define _VIEWER_STATE_HPP_

#include <shadertoy/utils/log.hpp>

#include <glm/vec3.hpp>

#include "config.hpp"
#include "log.hpp"

struct viewer_state {
    bool draw_wireframe;
    bool draw_quad;
    bool rotate_camera;
    float user_rotate_x;
    float user_rotate_y;
    double pressed_pos_x;
    double pressed_pos_y;
    double current_pos_x;
    double current_pos_y;
    int pressed_buttons;
    int frame_count;
    glm::vec3 center;
    float scale;

    std::shared_ptr<spd::logger> log;

    viewer_state(std::shared_ptr<spd::logger> log);

    void update_rotation(bool previous_rotate);

    float get_rotation_x();

    float get_rotation_y();

    float get_rotation_y(int frame_factor);

    void apply_mouse_rotation();
};

#endif /* _VIEWER_STATE_HPP_ */
