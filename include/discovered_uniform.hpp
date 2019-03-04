#ifndef _DISCOVERED_UNIFORM_HPP_
#define _DISCOVERED_UNIFORM_HPP_

#include <boost/variant.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "uniforms.hpp"

typedef boost::variant<float, glm::vec2, glm::vec3, glm::vec4, int, glm::ivec2,
                       glm::ivec3, glm::ivec4, bool, glm::bvec2, glm::bvec3,
                       glm::bvec4>
    uniform_variant;

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

    discovered_uniform(uniform_variant s_min, uniform_variant s_max,
                       uniform_variant s_pow, uniform_variant s_def,
                       const std::string &s_fmt, const std::string &s_cat,
                       const std::string &s_name,
                       const std::string &s_username);

    void set_uniform(parsed_inputs_t &inputs);

    void create_uniform(parsed_inputs_t &inputs);

    void render_imgui();

    static discovered_uniform parse_spec(
        const std::string &l_min, const std::string &l_max,
        const std::string &l_fmt, const std::string &l_pow,
        const std::string &l_cat, const std::string &l_def,
        const std::string &l_name, const std::string &l_unm,
        const std::string &type);
};

void try_parse_uniform(const std::string &line,
                       std::vector<discovered_uniform> &discovered_uniforms,
                       std::shared_ptr<spdlog::logger> log);

#endif /* _DISCOVERED_UNIFORM_HPP_ */
