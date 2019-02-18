#include <epoxy/gl.h>
#include <zmq.hpp>

#include "gl_state.hpp"
#include "log.hpp"

#include "net/server.hpp"

using namespace net;

namespace net {
struct server_impl {
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
};
}  // namespace net

server::server(const std::string &bind_addr)
    : impl_{std::make_unique<server_impl>(bind_addr)} {}

server::~server() {}

void server::poll(gl_state &gl_state, int revision) {
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
            zmq::message_t cmd_msg;
            if (!impl_->getframe_pending)
                impl_->socket.recv(&cmd_msg);

            if (impl_->getframe_pending ||
                std::strncmp("getframe",
                             reinterpret_cast<const char *>(cmd_msg.data()),
                             cmd_msg.size()) == 0) {
                if (read_frame) {
                    // We already read a frame this poll-round
                    // This means setparam messages were in the queue
                    // and we now need to render a new frame with those
                    // changed parameters.
                    next_frame = true;
                    impl_->getframe_pending = true;
                    break;
                }

                // Get rendered-to texture
                auto &texture = gl_state.get_render_result(revision);

                // Status message
                const char status[] = "success";
                zmq::message_t status_msg(sizeof(status) - 1);
                memcpy(status_msg.data(), status, sizeof(status) - 1);
                impl_->socket.send(status_msg, ZMQ_SNDMORE);

                // Get texture parameters and format
                GLint width, height, internal_format;
                texture.get_parameter(0, GL_TEXTURE_WIDTH, &width);
                texture.get_parameter(0, GL_TEXTURE_HEIGHT, &height);
                texture.get_parameter(0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);

                // Build format message data
                int32_t format_spec[] = { width, height, internal_format };

                zmq::message_t format_msg(sizeof(format_spec));
                memcpy(format_msg.data(), format_spec, sizeof(format_spec));
                impl_->socket.send(format_msg, ZMQ_SNDMORE);

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

                read_frame = true;
                impl_->getframe_pending = false;
            } else {
                const char status[] = "error";
                const char details[] = "unknown command";

                zmq::message_t status_msg(sizeof(status) - 1);
                zmq::message_t err_msg(sizeof(details) - 1);

                memcpy(status_msg.data(), status, sizeof(status) - 1);
                memcpy(err_msg.data(), details, sizeof(details) - 1);

                impl_->socket.send(status_msg, ZMQ_SNDMORE);
                impl_->socket.send(err_msg, ZMQ_SNDMORE);
            }
        } else {
            next_frame = true;
        }
    }
}
