#include <epoxy/gl.h>
#include <msgpack.hpp>
#include <zmq.hpp>

#include "gl_state.hpp"
#include "log.hpp"

#include "net/server.hpp"

using namespace net;

namespace net {
class server_impl {
    static void free_msgpack(void *data, void *hint) {
        auto ptr = reinterpret_cast<msgpack::sbuffer *>(hint);
        delete ptr;
    }

   public:
    zmq::context_t context;
    zmq::socket_t socket;
    std::shared_ptr<spd::logger> logger;
    bool getframe_pending;

    server_impl(const std::string &bind_addr)
        : context(),
          socket(context, ZMQ_REP),
          logger(spd::stderr_color_st("server")),
          getframe_pending(false) {
        logger->set_level(spd::level::info);

        logger->info("Binding to {}", bind_addr);
        socket.bind(bind_addr.c_str());
    }

    template <typename T>
    void send(T &&msg, int flags = 0) {
        // use new because of the C zero-copy API of ZMQ
        msgpack::sbuffer *buffer = new msgpack::sbuffer();
        msgpack::pack(*buffer, msg);

        zmq::message_t zmsg(buffer->data(), buffer->size(), free_msgpack,
                            buffer);
        socket.send(zmsg, flags);
    }

    std::string recv_cmd() {
        zmq::message_t cmd_msg;
        socket.recv(&cmd_msg);

        return std::string(
            reinterpret_cast<const char *>(cmd_msg.data()),
            reinterpret_cast<const char *>(cmd_msg.data()) + cmd_msg.size());
    }
};
}  // namespace net

void server::handle_getframe(gl_state &gl_state, int revision, bool &read_frame,
                             bool &render_next_frame) const {
    if (read_frame) {
        // We already read a frame this poll-round
        // This means setparam messages were in the queue
        // and we now need to render a new frame with those
        // changed parameters.
        render_next_frame = true;
        impl_->getframe_pending = true;
        return;
    }

    // Get rendered-to texture
    auto &texture = gl_state.get_render_result(revision);

    // Get texture parameters and format
    GLint width, height, internal_format;
    texture.get_parameter(0, GL_TEXTURE_WIDTH, &width);
    texture.get_parameter(0, GL_TEXTURE_HEIGHT, &height);
    texture.get_parameter(0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);

    // Status message
    msgpack::type::tuple<bool, std::map<std::string, int>> result(
        true, std::map<std::string, int>{});

    // Set format message data
    result.get<1>().emplace("width", width);
    result.get<1>().emplace("height", height);
    result.get<1>().emplace("format", internal_format);

    impl_->send(result, ZMQ_SNDMORE);

    // Send image data
    size_t bytes_per_pixel;
    if (internal_format == GL_RGBA32F)
        bytes_per_pixel = 4 * sizeof(float);
    else
        throw std::runtime_error("Unsupported internal format");

    size_t sz = width * height * bytes_per_pixel;
    zmq::message_t data_msg(sz);
    texture.get_image(0, GL_RGBA, GL_FLOAT, sz, data_msg.data());
    impl_->socket.send(data_msg);

    impl_->getframe_pending = false;
    read_frame = true;
}

server::server(const std::string &bind_addr)
    : impl_{std::make_unique<server_impl>(bind_addr)} {}

server::~server() {}

void server::poll(gl_state &gl_state, int revision) const {
    bool next_frame = false;
    bool read_frame = false;

    while (!next_frame) {
        zmq::pollitem_t items[] = {
            {static_cast<void *>(impl_->socket), 0, ZMQ_POLLIN, 0}};

        if (!impl_->getframe_pending) {
            // Check for an incoming request
            zmq::poll(&items[0], sizeof(items) / sizeof(items[0]), 0);
        }

        // Incoming request?
        if (items[0].revents & ZMQ_POLLIN || impl_->getframe_pending) {
            if (impl_->getframe_pending) {
                handle_getframe(gl_state, revision, read_frame, next_frame);
            } else {
                auto cmdname = impl_->recv_cmd();

                if (cmdname.compare(CMD_NAME_GETFRAME) == 0) {
                    handle_getframe(gl_state, revision, read_frame, next_frame);
                } else {
                    msgpack::type::tuple<bool, std::string> result(
                        false, "unknown command");
                    impl_->send(result);
                }
            }
        } else {
            next_frame = true;
        }
    }
}
