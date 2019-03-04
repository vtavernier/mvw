#include <epoxy/gl.h>

#include <shadertoy.hpp>

#include <regex>

#include "imgui.h"

#include "discovered_uniform.hpp"

using std::regex;
using std::regex_search;
using std::smatch;

static const regex regex_vardecl(
    "^//"
    "!\\s+(float|vec2|vec3|vec4|int|ivec2|ivec3|ivec4|bool|bvec2|bvec3|"
    "bvec4)\\s+(\\S+)\\s+(.*)$",
    regex::ECMAScript);
static const regex regex_varmin("min=(\\S+)", regex::ECMAScript);
static const regex regex_varmax("max=(\\S+)", regex::ECMAScript);
static const regex regex_varfmt("fmt=\"([^\"]+)\"", regex::ECMAScript);
static const regex regex_varpow("pow=(\\S+)", regex::ECMAScript);
static const regex regex_varcat("cat=\"([^\"]+)\"", regex::ECMAScript);
static const regex regex_vardef("def=(\\S+)", regex::ECMAScript);
static const regex regex_varunm("unm=\"([^\"]+)\"", regex::ECMAScript);

template <typename Tvec>
uniform_variant parse_variant1(const std::string &l) {
    if (l.empty()) return uniform_variant{Tvec{}};

    std::stringstream ss(l);
    Tvec value;
    ss >> value;
    return uniform_variant{value};
}

template <typename Tvec, size_t N>
uniform_variant parse_variantN(const std::string &l) {
    if (l.empty()) return uniform_variant{Tvec{}};

    std::stringstream ss(l);
    std::string part;
    size_t i = 0;
    Tvec value;

    // Initialize component values
    while (!ss.eof() && i < N) {
        std::getline(ss, part, ',');
        if (!part.empty()) {
            std::stringstream sp(part);
            sp >> value[i];
        }

        i++;
    }

    // Initialize all components to same value if there is only one
    if (i == 1) value = Tvec(value[0]);

    return uniform_variant{value};
}

class uniform_setter_visitor : public boost::static_visitor<void> {
    const std::string &name_;
    parsed_inputs_t &inputs_;

   public:
    template <typename T>
    inline void operator()(T &value) const {
        inputs_.get<iMvwParsedUniforms>().get<T>(name_) = value;
    };

    uniform_setter_visitor(const std::string &name, parsed_inputs_t &inputs)
        : name_(name), inputs_(inputs) {}
};

class uniform_create_visitor : public boost::static_visitor<void> {
    const std::string &name_;
    parsed_inputs_t &inputs_;

   public:
    template <typename T>
    inline void operator()(T &value) const {
        inputs_.get<iMvwParsedUniforms>().insert<T>(name_, value);
    };

    uniform_create_visitor(const std::string &name, parsed_inputs_t &inputs)
        : name_(name), inputs_(inputs) {}
};

template <typename T>
struct single_value {};

template <>
struct single_value<float> {
    void operator()(float vmin, float vmax, float &vx, float vpw,
                    const std::string &label, const std::string &format) {
        ImGui::SliderFloat(label.c_str(), &vx, vmin, vmax, format.c_str(), vpw);
    }
};

template <>
struct single_value<int> {
    void operator()(int vmin, int vmax, int &vx, int _vpw,
                    const std::string &label, const std::string &format) {
        ImGui::SliderInt(label.c_str(), &vx, vmin, vmax, format.c_str());
    }
};

template <>
struct single_value<bool> {
    void operator()(bool _vmin, bool _vmax, bool &vx, bool _vpw,
                    const std::string &label, const std::string &format) {
        ImGui::Checkbox(label.c_str(), &vx);
    }
};

template <typename T>
struct vec2_value {
    void operator()(glm::tvec2<T> vmin, glm::tvec2<T> vmax, glm::tvec2<T> &vx,
                    glm::tvec2<T> vpw, const std::string &label,
                    const std::string &format) {
        single_value<T>{}(vmin.x, vmax.x, vx.x, vpw.x, label + ".x", format);
        single_value<T>{}(vmin.y, vmax.y, vx.y, vpw.y, label + ".y", format);
    }
};

template <typename T>
struct vec3_value {
    void operator()(glm::tvec3<T> vmin, glm::tvec3<T> vmax, glm::tvec3<T> &vx,
                    glm::tvec3<T> vpw, const std::string &label,
                    const std::string &format) {
        single_value<T>{}(vmin.x, vmax.x, vx.x, vpw.x, label + ".x", format);
        single_value<T>{}(vmin.y, vmax.y, vx.y, vpw.y, label + ".y", format);
        single_value<T>{}(vmin.z, vmax.z, vx.z, vpw.z, label + ".z", format);
    }
};

template <typename T>
struct vec4_value {
    void operator()(glm::tvec4<T> vmin, glm::tvec4<T> vmax, glm::tvec4<T> &vx,
                    glm::tvec4<T> vpw, const std::string &label,
                    const std::string &format) {
        single_value<T>{}(vmin.x, vmax.x, vx.x, vpw.x, label + ".x", format);
        single_value<T>{}(vmin.y, vmax.y, vx.y, vpw.y, label + ".y", format);
        single_value<T>{}(vmin.z, vmax.z, vx.z, vpw.z, label + ".z", format);
        single_value<T>{}(vmin.w, vmax.w, vx.w, vpw.w, label + ".w", format);
    }
};

template <typename T>
struct vec_dispatch;

#define DISPATCH_TARGET(T, U, K)  \
    template <>                   \
    struct vec_dispatch<T> {      \
        typedef U<K> target_type; \
    }

DISPATCH_TARGET(float, single_value, float);
DISPATCH_TARGET(int, single_value, int);
DISPATCH_TARGET(bool, single_value, bool);

DISPATCH_TARGET(glm::vec2, vec2_value, float);
DISPATCH_TARGET(glm::ivec2, vec2_value, int);
DISPATCH_TARGET(glm::bvec2, vec2_value, bool);

DISPATCH_TARGET(glm::vec3, vec3_value, float);
DISPATCH_TARGET(glm::ivec3, vec3_value, int);
DISPATCH_TARGET(glm::bvec3, vec3_value, bool);

DISPATCH_TARGET(glm::vec4, vec4_value, float);
DISPATCH_TARGET(glm::ivec4, vec4_value, int);
DISPATCH_TARGET(glm::bvec4, vec4_value, bool);

class imgui_render_visitor : public boost::static_visitor<void> {
    discovered_uniform &uniform_;

   public:
    template <typename T>
    void operator()(T &value) const {
        typename vec_dispatch<T>::target_type{}(
            boost::get<T>(uniform_.s_min), boost::get<T>(uniform_.s_max), value,
            boost::get<T>(uniform_.s_pow), uniform_.s_username, uniform_.s_fmt);
    }

    imgui_render_visitor(discovered_uniform &uniform) : uniform_(uniform) {}
};

discovered_uniform::discovered_uniform(
    uniform_variant s_min, uniform_variant s_max, uniform_variant s_pow,
    uniform_variant s_def, const std::string &s_fmt, const std::string &s_cat,
    const std::string &s_name, const std::string &s_username)
    : value(s_def),
      s_min(s_min),
      s_max(s_max),
      s_pow(s_pow),
      s_def(s_def),
      s_fmt(s_fmt),
      s_cat(s_cat),
      s_name(s_name),
      s_username(s_username) {}

void discovered_uniform::set_uniform(parsed_inputs_t &inputs) {
    boost::apply_visitor(uniform_setter_visitor{s_name, inputs}, value);
}

void discovered_uniform::create_uniform(parsed_inputs_t &inputs) {
    boost::apply_visitor(uniform_create_visitor{s_name, inputs}, value);
}

void discovered_uniform::render_imgui() {
    boost::apply_visitor(imgui_render_visitor{*this}, value);
}

discovered_uniform discovered_uniform::parse_spec(
    const std::string &l_min, const std::string &l_max,
    const std::string &l_fmt, const std::string &l_pow,
    const std::string &l_cat, const std::string &l_def,
    const std::string &l_name, const std::string &l_unm,
    const std::string &type) {
#define __UNIFORM_T1(T, ___min, ___max, ___fmt, ___pow, ___cat, ___def,       \
                     ___name, ___unm)                                         \
    return discovered_uniform(                                                \
        parse_variant1<T>(___min), parse_variant1<T>(___max),                 \
        parse_variant1<T>(___pow), parse_variant1<T>(___def), ___fmt, ___cat, \
        ___name, ___unm)
#define __UNIFORM_TN(T, N, ___min, ___max, ___fmt, ___pow, ___cat, ___def,  \
                     ___name, ___unm)                                       \
    return discovered_uniform(parse_variantN<T##N COMMA N>(___min),         \
                              parse_variantN<T##N COMMA N>(___max),         \
                              parse_variantN<T##N COMMA N>(___pow),         \
                              parse_variantN<T##N COMMA N>(___def), ___fmt, \
                              ___cat, ___name, ___unm)
#define COMMA ,

    if (type == "float")
        __UNIFORM_T1(float, l_min, l_max, l_fmt, l_pow, l_cat, l_def, l_name,
                     l_unm);
    if (type == "vec2")
        __UNIFORM_TN(glm::vec, 2, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm);
    if (type == "vec3")
        __UNIFORM_TN(glm::vec, 3, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm);
    if (type == "vec4")
        __UNIFORM_TN(glm::vec, 4, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm);

    if (type == "int")
        __UNIFORM_T1(int, l_min, l_max, l_fmt, l_pow, l_cat, l_def, l_name,
                     l_unm);
    if (type == "ivec2")
        __UNIFORM_TN(glm::ivec, 2, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm);
    if (type == "ivec3")
        __UNIFORM_TN(glm::ivec, 3, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm);
    if (type == "ivec4")
        __UNIFORM_TN(glm::ivec, 4, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm);

    if (type == "bool")
        __UNIFORM_T1(bool, l_min, l_max, l_fmt, l_pow, l_cat, l_def, l_name,
                     l_unm);
    if (type == "bvec2")
        __UNIFORM_TN(glm::bvec, 2, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm);
    if (type == "bvec3")
        __UNIFORM_TN(glm::bvec, 3, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm);
    if (type == "bvec4")
        __UNIFORM_TN(glm::bvec, 4, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm);

    throw std::runtime_error("invalid type");
}

void try_parse_uniform(const std::string &line,
                       std::vector<discovered_uniform> &discovered_uniforms,
                       std::shared_ptr<spdlog::logger> log) {
    std::smatch match;
    if (regex_search(line, match, regex_vardecl) && match.size() > 1) {
        auto type = match.str(1);
        auto name = match.str(2);
        auto spec = match.str(3);

        std::smatch match_min, match_max, match_fmt, match_pow, match_cat,
            match_def, match_unm;
        regex_search(spec, match_min, regex_varmin);
        regex_search(spec, match_max, regex_varmax);
        regex_search(spec, match_fmt, regex_varfmt);
        regex_search(spec, match_pow, regex_varpow);
        regex_search(spec, match_cat, regex_varcat);
        regex_search(spec, match_def, regex_vardef);
        regex_search(spec, match_unm, regex_varunm);

        log->info("Parsed uniform declaration for {} \"{}\" (type {})", name,
                  match_unm.size() > 1 ? match_unm.str(1) : "", type);

        discovered_uniforms.emplace_back(discovered_uniform::parse_spec(
            match_min.size() > 1 ? match_min.str(1) : "0",
            match_max.size() > 1 ? match_max.str(1) : "1",
            match_fmt.size() > 1 ? match_fmt.str(1) : "",
            match_pow.size() > 1 ? match_pow.str(1) : "1",
            match_cat.size() > 1 ? match_cat.str(1) : "",
            match_def.size() > 1 ? match_def.str(1) : "", name,
            match_unm.size() > 1 ? match_unm.str(1) : name, type));
    }
}
