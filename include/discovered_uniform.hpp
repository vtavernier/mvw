#ifndef _DISCOVERED_UNIFORM_HPP_
#define _DISCOVERED_UNIFORM_HPP_

#include <boost/variant.hpp>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <msgpack.hpp>

#include "uniforms.hpp"

typedef boost::variant<float, glm::vec2, glm::vec3, glm::vec4, int, glm::ivec2,
                       glm::ivec3, glm::ivec4, bool, glm::bvec2, glm::bvec3,
                       glm::bvec4>
    uniform_variant;

template <typename Stream>
class uniform_variant_packer : public boost::static_visitor<void> {
    msgpack::packer<Stream> &o_;

#define PACK_UNIFORM_SCALAR(U)                 \
    void operator()(const U &item) const {     \
        msgpack::adaptor::pack<U>()(o_, item); \
    }
#define PACK_UNIFORM_VEC2(U)                           \
    void operator()(const glm::tvec2<U> &item) const { \
        o_.pack_array(2);                              \
        msgpack::adaptor::pack<U>()(o_, item.x);       \
        msgpack::adaptor::pack<U>()(o_, item.y);       \
    }
#define PACK_UNIFORM_VEC3(U)                           \
    void operator()(const glm::tvec3<U> &item) const { \
        o_.pack_array(3);                              \
        msgpack::adaptor::pack<U>()(o_, item.x);       \
        msgpack::adaptor::pack<U>()(o_, item.y);       \
        msgpack::adaptor::pack<U>()(o_, item.z);       \
    }
#define PACK_UNIFORM_VEC4(U)                           \
    void operator()(const glm::tvec4<U> &item) const { \
        o_.pack_array(4);                              \
        msgpack::adaptor::pack<U>()(o_, item.x);       \
        msgpack::adaptor::pack<U>()(o_, item.y);       \
        msgpack::adaptor::pack<U>()(o_, item.z);       \
        msgpack::adaptor::pack<U>()(o_, item.w);       \
    }
#define PACK_UNIFORM(U)    \
    PACK_UNIFORM_SCALAR(U) \
    PACK_UNIFORM_VEC2(U)   \
    PACK_UNIFORM_VEC3(U)   \
    PACK_UNIFORM_VEC4(U)

   public:
    PACK_UNIFORM(float);
    PACK_UNIFORM(int);
    PACK_UNIFORM(bool);

    uniform_variant_packer(msgpack::packer<Stream> &o) : o_(o) {}
};

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <>
    struct pack<uniform_variant> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            uniform_variant const &v) const {
            boost::apply_visitor(uniform_variant_packer<Stream>{o}, v);
            return o;
        }
    };

    template <>
    struct convert<uniform_variant> {
        msgpack::object const &operator()(msgpack::object const &o,
                                          uniform_variant &v) const {
            if (o.type == msgpack::type::ARRAY) {
                if (o.via.array.size == 2) {
                    if (o.via.array.ptr[0].type == msgpack::type::FLOAT32 ||
                        o.via.array.ptr[0].type == msgpack::type::FLOAT64) {
                        v = glm::vec2(o.via.array.ptr[0].as<float>(),
                                      o.via.array.ptr[1].as<float>());
                    } else if (o.via.array.ptr[0].type ==
                                   msgpack::type::POSITIVE_INTEGER ||
                               o.via.array.ptr[0].type ==
                                   msgpack::type::NEGATIVE_INTEGER) {
                        v = glm::ivec2(o.via.array.ptr[0].as<int>(),
                                       o.via.array.ptr[1].as<int>());
                    } else if (o.via.array.ptr[0].type ==
                               msgpack::type::BOOLEAN) {
                        v = glm::bvec2(o.via.array.ptr[0].as<bool>(),
                                       o.via.array.ptr[1].as<bool>());
                    } else {
                        throw msgpack::type_error();
                    }
                } else if (o.via.array.size == 3) {
                    if (o.via.array.ptr[0].type == msgpack::type::FLOAT32 ||
                        o.via.array.ptr[0].type == msgpack::type::FLOAT64) {
                        v = glm::vec3(o.via.array.ptr[0].as<float>(),
                                      o.via.array.ptr[1].as<float>(),
                                      o.via.array.ptr[2].as<float>());
                    } else if (o.via.array.ptr[0].type ==
                                   msgpack::type::POSITIVE_INTEGER ||
                               o.via.array.ptr[0].type ==
                                   msgpack::type::NEGATIVE_INTEGER) {
                        v = glm::ivec3(o.via.array.ptr[0].as<int>(),
                                       o.via.array.ptr[1].as<int>(),
                                       o.via.array.ptr[2].as<int>());
                    } else if (o.via.array.ptr[0].type ==
                               msgpack::type::BOOLEAN) {
                        v = glm::bvec3(o.via.array.ptr[0].as<bool>(),
                                       o.via.array.ptr[1].as<bool>(),
                                       o.via.array.ptr[2].as<bool>());
                    } else {
                        throw msgpack::type_error();
                    }
                } else if (o.via.array.size == 4) {
                    if (o.via.array.ptr[0].type == msgpack::type::FLOAT32 ||
                        o.via.array.ptr[0].type == msgpack::type::FLOAT64) {
                        v = glm::vec4(o.via.array.ptr[0].as<float>(),
                                      o.via.array.ptr[1].as<float>(),
                                      o.via.array.ptr[2].as<float>(),
                                      o.via.array.ptr[3].as<float>());
                    } else if (o.via.array.ptr[0].type ==
                                   msgpack::type::POSITIVE_INTEGER ||
                               o.via.array.ptr[0].type ==
                                   msgpack::type::NEGATIVE_INTEGER) {
                        v = glm::ivec4(o.via.array.ptr[0].as<int>(),
                                       o.via.array.ptr[1].as<int>(),
                                       o.via.array.ptr[2].as<int>(),
                                       o.via.array.ptr[3].as<int>());
                    } else if (o.via.array.ptr[0].type ==
                               msgpack::type::BOOLEAN) {
                        v = glm::bvec4(o.via.array.ptr[0].as<bool>(),
                                       o.via.array.ptr[1].as<bool>(),
                                       o.via.array.ptr[2].as<bool>(),
                                       o.via.array.ptr[3].as<bool>());
                    } else {
                        throw msgpack::type_error();
                    }
                } else {
                    throw msgpack::type_error();
                }
            } else if (o.type == msgpack::type::FLOAT32 ||
                       o.type == msgpack::type::FLOAT64) {
                v = o.as<float>();
            } else if (o.type == msgpack::type::POSITIVE_INTEGER ||
                       o.type == msgpack::type::NEGATIVE_INTEGER) {
                v = o.as<int>();

            } else if (o.type == msgpack::type::BOOLEAN) {
                v = o.as<bool>();
            } else {
                throw msgpack::type_error();
            }

            return o;
        }
    };
    }  // namespace adaptor
}  // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
}  // namespace msgpack

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
                       std::vector<discovered_uniform> &discovered_uniforms,
                       std::shared_ptr<spdlog::logger> log);

bool try_set_variant(uniform_variant &dst, const uniform_variant &value);

#endif /* _DISCOVERED_UNIFORM_HPP_ */
