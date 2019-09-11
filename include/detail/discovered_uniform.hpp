#ifndef _DETAIL_DISCOVERED_UNIFORM_HPP_
#define _DETAIL_DISCOVERED_UNIFORM_HPP_

#include <variant>

#ifndef __EMSCRIPTEN__
#include <msgpack.hpp>

template <typename Stream>
class uniform_variant_packer {
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

    template <typename T>
    struct pack<glm::tvec2<T>> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            glm::tvec2<T> const &v) const {
            o.pack_array(2);
            msgpack::adaptor::pack<T>()(o, v.x);
            msgpack::adaptor::pack<T>()(o, v.y);
            return o;
        }
    };

    template <typename T>
    struct convert<glm::tvec2<T>> {
        msgpack::object const &operator()(msgpack::object const &o,
                                          glm::tvec2<T> &v) const {
            if (o.type == msgpack::type::ARRAY) {
                if (o.via.array.size == 2) {
                    if (o.via.array.ptr[0].type == msgpack::type::FLOAT32 ||
                        o.via.array.ptr[0].type == msgpack::type::FLOAT64) {
                        v = glm::tvec2<T>(o.via.array.ptr[0].as<float>(),
                                          o.via.array.ptr[1].as<float>());
                    } else if (o.via.array.ptr[0].type ==
                                   msgpack::type::POSITIVE_INTEGER ||
                               o.via.array.ptr[0].type ==
                                   msgpack::type::NEGATIVE_INTEGER) {
                        v = glm::tvec2<T>(o.via.array.ptr[0].as<int>(),
                                          o.via.array.ptr[1].as<int>());
                    } else if (o.via.array.ptr[0].type ==
                               msgpack::type::BOOLEAN) {
                        v = glm::tvec2<T>(o.via.array.ptr[0].as<bool>(),
                                          o.via.array.ptr[1].as<bool>());
                    } else {
                        throw msgpack::type_error();
                    }
                }
            } else if (o.type == msgpack::type::FLOAT32 ||
                       o.type == msgpack::type::FLOAT64) {
                v = glm::tvec2<T>(o.as<float>());
            } else if (o.type == msgpack::type::POSITIVE_INTEGER ||
                       o.type == msgpack::type::NEGATIVE_INTEGER) {
                v = glm::tvec2<T>(o.as<int>());
            } else if (o.type == msgpack::type::BOOLEAN) {
                v = glm::tvec2<T>(o.as<bool>());
            } else {
                throw msgpack::type_error();
            }

            return o;
        }
    };

    template <typename T>
    struct pack<glm::tvec3<T>> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            glm::tvec3<T> const &v) const {
            o.pack_array(3);
            msgpack::adaptor::pack<T>()(o, v.x);
            msgpack::adaptor::pack<T>()(o, v.y);
            msgpack::adaptor::pack<T>()(o, v.z);
            return o;
        }
    };

    template <typename T>
    struct convert<glm::tvec3<T>> {
        msgpack::object const &operator()(msgpack::object const &o,
                                          glm::tvec3<T> &v) const {
            if (o.type == msgpack::type::ARRAY) {
                if (o.via.array.size == 3) {
                    if (o.via.array.ptr[0].type == msgpack::type::FLOAT32 ||
                        o.via.array.ptr[0].type == msgpack::type::FLOAT64) {
                        v = glm::tvec3<T>(o.via.array.ptr[0].as<float>(),
                                          o.via.array.ptr[1].as<float>(),
                                          o.via.array.ptr[2].as<float>());
                    } else if (o.via.array.ptr[0].type ==
                                   msgpack::type::POSITIVE_INTEGER ||
                               o.via.array.ptr[0].type ==
                                   msgpack::type::NEGATIVE_INTEGER) {
                        v = glm::tvec3<T>(o.via.array.ptr[0].as<int>(),
                                          o.via.array.ptr[1].as<int>(),
                                          o.via.array.ptr[2].as<int>());
                    } else if (o.via.array.ptr[0].type ==
                               msgpack::type::BOOLEAN) {
                        v = glm::tvec3<T>(o.via.array.ptr[0].as<bool>(),
                                          o.via.array.ptr[1].as<bool>(),
                                          o.via.array.ptr[2].as<bool>());
                    } else {
                        throw msgpack::type_error();
                    }
                }
            } else if (o.type == msgpack::type::FLOAT32 ||
                       o.type == msgpack::type::FLOAT64) {
                v = glm::tvec3<T>(o.as<float>());
            } else if (o.type == msgpack::type::POSITIVE_INTEGER ||
                       o.type == msgpack::type::NEGATIVE_INTEGER) {
                v = glm::tvec3<T>(o.as<int>());
            } else if (o.type == msgpack::type::BOOLEAN) {
                v = glm::tvec3<T>(o.as<bool>());
            } else {
                throw msgpack::type_error();
            }

            return o;
        }
    };

    template <typename T>
    struct pack<glm::tvec4<T>> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            glm::tvec4<T> const &v) const {
            o.pack_array(4);
            msgpack::adaptor::pack<T>()(o, v.x);
            msgpack::adaptor::pack<T>()(o, v.y);
            msgpack::adaptor::pack<T>()(o, v.z);
            msgpack::adaptor::pack<T>()(o, v.w);
            return o;
        }
    };

    template <typename T>
    struct convert<glm::tvec4<T>> {
        msgpack::object const &operator()(msgpack::object const &o,
                                          glm::tvec4<T> &v) const {
            if (o.type == msgpack::type::ARRAY) {
                if (o.via.array.size == 4) {
                    if (o.via.array.ptr[0].type == msgpack::type::FLOAT32 ||
                        o.via.array.ptr[0].type == msgpack::type::FLOAT64) {
                        v = glm::tvec4<T>(o.via.array.ptr[0].as<float>(),
                                          o.via.array.ptr[1].as<float>(),
                                          o.via.array.ptr[2].as<float>(),
                                          o.via.array.ptr[3].as<float>());
                    } else if (o.via.array.ptr[0].type ==
                                   msgpack::type::POSITIVE_INTEGER ||
                               o.via.array.ptr[0].type ==
                                   msgpack::type::NEGATIVE_INTEGER) {
                        v = glm::tvec4<T>(o.via.array.ptr[0].as<int>(),
                                          o.via.array.ptr[1].as<int>(),
                                          o.via.array.ptr[2].as<int>(),
                                          o.via.array.ptr[3].as<int>());
                    } else if (o.via.array.ptr[0].type ==
                               msgpack::type::BOOLEAN) {
                        v = glm::tvec4<T>(o.via.array.ptr[0].as<bool>(),
                                          o.via.array.ptr[1].as<bool>(),
                                          o.via.array.ptr[2].as<bool>(),
                                          o.via.array.ptr[3].as<bool>());
                    } else {
                        throw msgpack::type_error();
                    }
                }
            } else if (o.type == msgpack::type::FLOAT32 ||
                       o.type == msgpack::type::FLOAT64) {
                v = glm::tvec4<T>(o.as<float>());
            } else if (o.type == msgpack::type::POSITIVE_INTEGER ||
                       o.type == msgpack::type::NEGATIVE_INTEGER) {
                v = glm::tvec4<T>(o.as<int>());
            } else if (o.type == msgpack::type::BOOLEAN) {
                v = glm::tvec4<T>(o.as<bool>());
            } else {
                throw msgpack::type_error();
            }

            return o;
        }
    };

    template <>
    struct pack<uniform_variant> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            uniform_variant const &v) const {
            std::visit(uniform_variant_packer<Stream>{o}, v);
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
#endif /* __EMSCRIPTEN__ */

#endif /* _DETAIL_DISCOVERED_UNIFORM_HPP_ */
