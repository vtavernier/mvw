#include <epoxy/gl.h>

#include <shadertoy.hpp>

#include <regex>

#include "imgui.h"

#include "log.hpp"

#include "discovered_uniform.hpp"

using std::regex;
using std::regex_search;
using std::smatch;

static const regex regex_vardecl(
    "^//"
    "!\\s+(float|vec2|vec3|vec4|int|ivec2|ivec3|ivec4|bool|bvec2|bvec3|"
    "bvec4)\\s+(\\S+)\\s+(.*)$",
    regex::ECMAScript);
static const regex regex_varmin("\\bmin=(\\S+)", regex::ECMAScript);
static const regex regex_varmax("\\bmax=(\\S+)", regex::ECMAScript);
static const regex regex_varfmt("\\b(?:fmt|format)=\"([^\"]+)\"", regex::ECMAScript);
static const regex regex_varpow("\\bpow(?:er)?=(\\S+)\\b", regex::ECMAScript);
static const regex regex_varcat("\\bcat(?:egory)?=\"([^\"]+)\"", regex::ECMAScript);
static const regex regex_vardef("\\bdef(?:ault)?=(\\S+)", regex::ECMAScript);
static const regex regex_varunm("\\b(?:unm|username|label)=\"([^\"]+)\"", regex::ECMAScript);
static const regex regex_varang("\\bang(?:le)?\\b", regex::ECMAScript);

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

class uniform_setter_visitor {
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

class uniform_create_visitor {
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
                    const std::string &label, const std::string &format,
                    bool angle) {
        if (angle) {
            ImGui::SliderAngle(label.c_str(), &vx, vmin, vmax, format.c_str());
        } else {
            ImGui::SliderFloat(label.c_str(), &vx, vmin, vmax, format.c_str(),
                               vpw);
        }
    }
};

template <>
struct single_value<int> {
    void operator()(int vmin, int vmax, int &vx, int _vpw,
                    const std::string &label, const std::string &format,
                    bool _angle) {
        ImGui::SliderInt(label.c_str(), &vx, vmin, vmax, format.c_str());
    }
};

template <>
struct single_value<bool> {
    void operator()(bool _vmin, bool _vmax, bool &vx, bool _vpw,
                    const std::string &label, const std::string &format,
                    bool _angle) {
        ImGui::Checkbox(label.c_str(), &vx);
    }
};

template <typename T>
struct vec2_value {
    void operator()(glm::tvec2<T> vmin, glm::tvec2<T> vmax, glm::tvec2<T> &vx,
                    glm::tvec2<T> vpw, const std::string &label,
                    const std::string &format, bool angle) {
        single_value<T>{}(vmin.x, vmax.x, vx.x, vpw.x, label + ".x", format,
                          angle);
        single_value<T>{}(vmin.y, vmax.y, vx.y, vpw.y, label + ".y", format,
                          angle);
    }
};

template <typename T>
struct vec3_value {
    void operator()(glm::tvec3<T> vmin, glm::tvec3<T> vmax, glm::tvec3<T> &vx,
                    glm::tvec3<T> vpw, const std::string &label,
                    const std::string &format, bool angle) {
        single_value<T>{}(vmin.x, vmax.x, vx.x, vpw.x, label + ".x", format,
                          angle);
        single_value<T>{}(vmin.y, vmax.y, vx.y, vpw.y, label + ".y", format,
                          angle);
        single_value<T>{}(vmin.z, vmax.z, vx.z, vpw.z, label + ".z", format,
                          angle);
    }
};

template <typename T>
struct vec4_value {
    void operator()(glm::tvec4<T> vmin, glm::tvec4<T> vmax, glm::tvec4<T> &vx,
                    glm::tvec4<T> vpw, const std::string &label,
                    const std::string &format, bool angle) {
        single_value<T>{}(vmin.x, vmax.x, vx.x, vpw.x, label + ".x", format,
                          angle);
        single_value<T>{}(vmin.y, vmax.y, vx.y, vpw.y, label + ".y", format,
                          angle);
        single_value<T>{}(vmin.z, vmax.z, vx.z, vpw.z, label + ".z", format,
                          angle);
        single_value<T>{}(vmin.w, vmax.w, vx.w, vpw.w, label + ".w", format,
                          angle);
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

class imgui_render_visitor {
    discovered_uniform &uniform_;

   public:
    template <typename T>
    void operator()(T &value) const {
        typename vec_dispatch<T>::target_type{}(
            std::get<T>(uniform_.s_min), std::get<T>(uniform_.s_max), value,
            std::get<T>(uniform_.s_pow), uniform_.s_username, uniform_.s_fmt,
            uniform_.s_angle);
    }

    imgui_render_visitor(discovered_uniform &uniform) : uniform_(uniform) {}
};

discovered_uniform::discovered_uniform(
    uniform_variant s_min, uniform_variant s_max, uniform_variant s_pow,
    uniform_variant s_def, const std::string &s_fmt, const std::string &s_cat,
    const std::string &s_name, const std::string &s_username, bool s_angle)
    : value(s_def),
      s_min(s_min),
      s_max(s_max),
      s_pow(s_pow),
      s_def(s_def),
      s_fmt(s_fmt),
      s_cat(s_cat),
      s_name(s_name),
      s_username(s_username),
      s_angle(s_angle) {}

void discovered_uniform::set_uniform(parsed_inputs_t &inputs) {
    std::visit(uniform_setter_visitor{s_name, inputs}, value);
}

void discovered_uniform::create_uniform(parsed_inputs_t &inputs) {
    std::visit(uniform_create_visitor{s_name, inputs}, value);
}

void discovered_uniform::render_imgui() {
    std::visit(imgui_render_visitor{*this}, value);
}

discovered_uniform discovered_uniform::parse_spec(
    const std::string &l_min, const std::string &l_max,
    const std::string &l_fmt, const std::string &l_pow,
    const std::string &l_cat, const std::string &l_def,
    const std::string &l_name, const std::string &l_unm,
    const std::string &type, bool l_ang) {
#define __UNIFORM_T1(T, ___min, ___max, ___fmt, ___pow, ___cat, ___def,       \
                     ___name, ___unm, ___ang)                                 \
    return discovered_uniform(                                                \
        parse_variant1<T>(___min), parse_variant1<T>(___max),                 \
        parse_variant1<T>(___pow), parse_variant1<T>(___def), ___fmt, ___cat, \
        ___name, ___unm, ___ang)
#define __UNIFORM_TN(T, N, ___min, ___max, ___fmt, ___pow, ___cat, ___def,  \
                     ___name, ___unm, ___ang)                               \
    return discovered_uniform(parse_variantN<T##N COMMA N>(___min),         \
                              parse_variantN<T##N COMMA N>(___max),         \
                              parse_variantN<T##N COMMA N>(___pow),         \
                              parse_variantN<T##N COMMA N>(___def), ___fmt, \
                              ___cat, ___name, ___unm, ___ang)
#define COMMA ,

    if (type == "float")
        __UNIFORM_T1(float, l_min, l_max, l_fmt, l_pow, l_cat, l_def, l_name,
                     l_unm, l_ang);
    if (type == "vec2")
        __UNIFORM_TN(glm::vec, 2, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm, l_ang);
    if (type == "vec3")
        __UNIFORM_TN(glm::vec, 3, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm, l_ang);
    if (type == "vec4")
        __UNIFORM_TN(glm::vec, 4, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm, l_ang);

    if (type == "int")
        __UNIFORM_T1(int, l_min, l_max, l_fmt, l_pow, l_cat, l_def, l_name,
                     l_unm, l_ang);
    if (type == "ivec2")
        __UNIFORM_TN(glm::ivec, 2, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm, l_ang);
    if (type == "ivec3")
        __UNIFORM_TN(glm::ivec, 3, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm, l_ang);
    if (type == "ivec4")
        __UNIFORM_TN(glm::ivec, 4, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm, l_ang);

    if (type == "bool")
        __UNIFORM_T1(bool, l_min, l_max, l_fmt, l_pow, l_cat, l_def, l_name,
                     l_unm, l_ang);
    if (type == "bvec2")
        __UNIFORM_TN(glm::bvec, 2, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm, l_ang);
    if (type == "bvec3")
        __UNIFORM_TN(glm::bvec, 3, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm, l_ang);
    if (type == "bvec4")
        __UNIFORM_TN(glm::bvec, 4, l_min, l_max, l_fmt, l_pow, l_cat, l_def,
                     l_name, l_unm, l_ang);

    throw std::runtime_error("invalid type");
}

void try_parse_uniform(const std::string &line,
                       std::vector<discovered_uniform> &discovered_uniforms) {
    std::smatch match;
    if (regex_search(line, match, regex_vardecl) && match.size() > 1) {
        auto type = match.str(1);
        auto name = match.str(2);
        auto spec = match.str(3);

        std::smatch match_min, match_max, match_fmt, match_pow, match_cat,
            match_def, match_unm, match_ang;
        regex_search(spec, match_min, regex_varmin);
        regex_search(spec, match_max, regex_varmax);
        regex_search(spec, match_fmt, regex_varfmt);
        regex_search(spec, match_pow, regex_varpow);
        regex_search(spec, match_cat, regex_varcat);
        regex_search(spec, match_def, regex_vardef);
        regex_search(spec, match_unm, regex_varunm);
        regex_search(spec, match_ang, regex_varang);

        VLOG->info("Parsed uniform declaration for {} \"{}\" (type {})", name,
                   match_unm.size() > 1 ? match_unm.str(1) : "", type);

        discovered_uniforms.emplace_back(discovered_uniform::parse_spec(
            match_min.size() > 1 ? match_min.str(1) : "0",
            match_max.size() > 1 ? match_max.str(1) : "1",
            match_fmt.size() > 1 ? match_fmt.str(1) : "",
            match_pow.size() > 1 ? match_pow.str(1) : "1",
            match_cat.size() > 1 ? match_cat.str(1) : "",
            match_def.size() > 1 ? match_def.str(1) : "", name,
            match_unm.size() > 1 ? match_unm.str(1) : name, type,
            match_ang.size() > 0));
    }
}

template <typename L, typename R>
struct variant_conversions : public std::false_type {};

template <typename T>
struct variant_conversions<T, T> : public std::true_type {};

template <>
struct variant_conversions<glm::vec2, int> : public std::true_type {};
template <>
struct variant_conversions<glm::vec3, int> : public std::true_type {};
template <>
struct variant_conversions<glm::vec4, int> : public std::true_type {};

template <>
struct variant_conversions<glm::vec2, float> : public std::true_type {};
template <>
struct variant_conversions<glm::vec3, float> : public std::true_type {};
template <>
struct variant_conversions<glm::vec4, float> : public std::true_type {};

template <>
struct variant_conversions<glm::ivec2, int> : public std::true_type {};
template <>
struct variant_conversions<glm::ivec3, int> : public std::true_type {};
template <>
struct variant_conversions<glm::ivec4, int> : public std::true_type {};

template <>
struct variant_conversions<glm::bvec2, bool> : public std::true_type {};
template <>
struct variant_conversions<glm::bvec3, bool> : public std::true_type {};
template <>
struct variant_conversions<glm::bvec4, bool> : public std::true_type {};

template <>
struct variant_conversions<float, int> : public std::true_type {};
template <>
struct variant_conversions<bool, int> : public std::true_type {};

struct variant_setter {
    template <typename L, typename R>
    typename std::enable_if<variant_conversions<L, R>::value, bool>::type
    try_set_variant(L &dst, const R &value) {
        dst = static_cast<L>(value);
        return true;
    }

    template <typename L, typename R>
    typename std::enable_if<!variant_conversions<L, R>::value, bool>::type
    try_set_variant(L &dst, const R &value) {
        return false;
    }
};

bool try_set_variant(uniform_variant &dst, const uniform_variant &value) {
    return std::visit(
        [&value](auto &left) -> bool {
            return std::visit(
                [&left](auto &right) -> bool {
                    return variant_setter()
                        .try_set_variant<
                            std::remove_const_t<
                                std::remove_reference_t<decltype(left)>>,
                            std::remove_const_t<
                                std::remove_reference_t<decltype(right)>>>(
                            left, right);
                },
                value);
        },
        dst);
}
