#include <epoxy/gl.h>
#include <msgpack.hpp>
#include <zmq.hpp>

#include "gl_state.hpp"
#include "log.hpp"

#include "net/server.hpp"

using namespace net;

namespace net {
typedef msgpack::type::tuple<bool, std::string> default_reply;
typedef msgpack::type::tuple<bool, std::map<std::string, int>> getframe_reply;
typedef msgpack::type::tuple<bool, std::vector<discovered_uniform>>
    getparams_reply;
typedef std::string getparam_args;
typedef msgpack::type::tuple<bool, discovered_uniform> getparam_reply;
typedef msgpack::type::tuple<std::string, uniform_variant> setparam_args;
typedef bool setparam_reply;

class server_impl {
    static void free_msgpack(void *data, void *hint) {
        auto ptr = reinterpret_cast<msgpack::sbuffer *>(hint);
        delete ptr;
    }

   public:
    zmq::context_t context;
    zmq::socket_t socket;
    std::shared_ptr<spdlog::logger> logger;
    bool getframe_pending;

    server_impl(const server_options &opt)
        : context(),
          socket(context, ZMQ_REP),
          logger(spdlog::stderr_color_st("server")),
          getframe_pending(false) {
        logger->set_level(spdlog::level::info);

        logger->info("Binding to {}", opt.bind_addr);
        socket.bind(opt.bind_addr);
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

    template <typename T>
    T recv() {
        zmq::message_t msg;
        socket.recv(&msg);
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(msg.data()),
                        msg.size());
        return result.get().as<T>();
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

void server::handle_getframe(gl_state &gl_state, int revision) const {
    // Get rendered-to texture
    auto &texture = gl_state.get_render_result(revision);

    // Get texture parameters and format
    GLint width, height, internal_format;
    texture.get_parameter(0, GL_TEXTURE_WIDTH, &width);
    texture.get_parameter(0, GL_TEXTURE_HEIGHT, &height);
    texture.get_parameter(0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);

    // Status message
    net::getframe_reply result(true, std::map<std::string, int>{});

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
    // Read the image into the message buffer directly
    texture.get_image(0, GL_RGBA, GL_FLOAT, sz, data_msg.data());
    impl_->socket.send(data_msg);
}

void server::handle_getparams(gl_state &gl_state, int revision) const {
    // Get the list of parameters
    auto &discovered_uniforms = gl_state.get_discovered_uniforms(revision);

    // Return result
    net::getparams_reply result(true, discovered_uniforms);
    impl_->send(result);
}

void server::handle_getparam(gl_state &gl_state, int revision) const {
    // Get the list of parameters
    auto &discovered_uniforms = gl_state.get_discovered_uniforms(revision);

    // Get the arguments
    auto param_name = impl_->recv<getparam_args>();

    // Find the right parameter
    auto it = std::find_if(
        discovered_uniforms.begin(), discovered_uniforms.end(),
        [&param_name](const auto &item) { return item.s_name == param_name; });

    if (it == discovered_uniforms.end()) {
        net::default_reply result(false, std::string("param ") + param_name +
                                             std::string(" not found"));
        impl_->send(result);
    } else {
        net::getparam_reply result(true, *it);
        impl_->send(result);
    }
}

void server::handle_setparam(gl_state &gl_state, int revision,
                             bool &changed_state) const {
    // Get the list of parameters
    auto &discovered_uniforms = gl_state.get_discovered_uniforms(revision);

    // Get the arguments
    auto args = impl_->recv<setparam_args>();
    const auto &param_name = args.get<0>();

    // Find the right parameter
    auto it = std::find_if(
        discovered_uniforms.begin(), discovered_uniforms.end(),
        [&param_name](const auto &item) { return item.s_name == param_name; });

    if (it == discovered_uniforms.end()) {
        net::default_reply result(false, std::string("param ") + param_name +
                                             std::string(" not found"));
        impl_->send(result);
    } else {
        // Try to set it with the decoded value
        bool set = try_set_variant(it->value, args.get<1>());

        if (set) {
            // Send success response
            net::setparam_reply result(true);
            impl_->send(result);
        } else {
            // Send failure response
            net::default_reply result(
                false, std::string("invalid type for param " + param_name));
            impl_->send(result);
        }
    }
}

server::server(const server_options &opt)
    : opt_(opt), impl_{std::make_unique<server_impl>(opt)} {}

server::~server() {}

void server::poll(gl_state &gl_state, int revision) const {
    // true if we should stop reading messages and render the next frame
    bool next_frame = false;
    // true if we changed any render state, meaning the current frame does
    // not match the new render state
    bool changed_state = false;

    // Check for pending getframe that we have to reply to
    if (impl_->getframe_pending) {
        handle_getframe(gl_state, revision);

        // Acknowledge getframe
        impl_->getframe_pending = false;
    }

    // Poll for incoming messages
    while (!next_frame) {
        zmq::pollitem_t items[] = {
            {static_cast<void *>(impl_->socket), 0, ZMQ_POLLIN, 0}};

        // Check for an incoming request
        zmq::poll(&items[0], sizeof(items) / sizeof(items[0]), 0);

        // Incoming request?
        if (items[0].revents & ZMQ_POLLIN) {
            auto cmdname = impl_->recv_cmd();

            if (cmdname.compare(CMD_NAME_GETFRAME) == 0) {
                if (changed_state) {
                    // We changed some render state, so the user probably wants
                    // the updated result instead of the current frame
                    impl_->getframe_pending = true;
                    next_frame = true;
                } else {
                    handle_getframe(gl_state, revision);
                }
            } else if (cmdname.compare(CMD_NAME_GETPARAMS) == 0) {
                handle_getparams(gl_state, revision);
            } else if (cmdname.compare(CMD_NAME_GETPARAM) == 0) {
                handle_getparam(gl_state, revision);
            } else if (cmdname.compare(CMD_NAME_SETPARAM) == 0) {
                handle_setparam(gl_state, revision, changed_state);
            } else {
                net::default_reply result(false, "unknown command");
                impl_->send(result);
            }
        } else {
            next_frame = true;
        }
    }
}
