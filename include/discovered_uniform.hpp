#ifndef _DISCOVERED_UNIFORM_HPP_
#define _DISCOVERED_UNIFORM_HPP_

#include <variant>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <msgpack.hpp>

#include "uniforms.hpp"

typedef std::variant<float, glm::vec2, glm::vec3, glm::vec4, int, glm::ivec2,
                     glm::ivec3, glm::ivec4, bool, glm::bvec2, glm::bvec3,
                     glm::bvec4>
    uniform_variant;

#include "detail/discovered_uniform.hpp"

struct discovered_uniform {
    uniform_variant value;
    uniform_variant s_min;
    uniform_variant s_max;
    uniform_variant s_pow;
    uniform_variant s_def;

    std::string s_fmt;
    std::string s_cat;
    std::string s_name;
    std::string s_username;

    bool s_angle;

    discovered_uniform(uniform_variant s_min, uniform_variant s_max,
                       uniform_variant s_pow, uniform_variant s_def,
                       const std::string &s_fmt, const std::string &s_cat,
                       const std::string &s_name, const std::string &s_username,
                       bool s_angle);

    void set_uniform(parsed_inputs_t &inputs);

    void create_uniform(parsed_inputs_t &inputs);

    void render_imgui();

    static discovered_uniform parse_spec(
        const std::string &l_min, const std::string &l_max,
        const std::string &l_fmt, const std::string &l_pow,
        const std::string &l_cat, const std::string &l_def,
        const std::string &l_name, const std::string &l_unm,
        const std::string &type, bool l_ang);

    MSGPACK_DEFINE_MAP(value, s_min, s_max, s_pow, s_def, s_fmt, s_cat, s_name,
                       s_username, s_angle);
};

void try_parse_uniform(const std::string &line,
                       std::vector<discovered_uniform> &discovered_uniforms);

bool try_set_variant(uniform_variant &dst, const uniform_variant &value);

#endif /* _DISCOVERED_UNIFORM_HPP_ */
