#ifndef _DETAIL_RSIZE_HPP_
#define _DETAIL_RSIZE_HPP_

#include <msgpack.hpp>
#include <shadertoy/size.hpp>

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <>
    struct pack<shadertoy::rsize> {
        template <typename Stream>
        msgpack::packer<Stream> &operator()(msgpack::packer<Stream> &o,
                                            shadertoy::rsize const &v) const {
            o.pack_array(2);
            o.pack_int32(v.width);
            o.pack_int32(v.height);
            return o;
        }
    };

    template <>
    struct convert<shadertoy::rsize> {
        msgpack::object const &operator()(msgpack::object const &o,
                                          shadertoy::rsize &v) const {
            if (o.type == msgpack::type::ARRAY && o.via.array.size == 2) {
                v.width = o.via.array.ptr[0].as<int>();
                v.height = o.via.array.ptr[1].as<int>();
            } else {
                throw msgpack::type_error();
            }

            return o;
        }
    };
    }  // namespace adaptor
}  // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
}  // namespace msgpack

#endif /* _DETAIL_RSIZE_HPP_ */
