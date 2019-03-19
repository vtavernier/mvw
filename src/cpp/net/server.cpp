#include <epoxy/gl.h>
#include <msgpack.hpp>
#include <zmq.hpp>

#include <optional>

#include "gl_state.hpp"
#include "log.hpp"
#include "viewer_state.hpp"

#include "detail/rsize.hpp"
#include "net/server.hpp"

using namespace net;
using shadertoy::operator==; // for output_name_t

namespace net {
typedef msgpack::type::tuple<bool, std::string> default_reply;
typedef msgpack::type::tuple<bool, std::map<std::string, int>> getframe_reply;
typedef msgpack::type::tuple<std::string, shadertoy::rsize> getframe_args;
typedef msgpack::type::tuple<bool, std::vector<discovered_uniform>>
    getparams_reply;
typedef std::string getparam_args;
typedef msgpack::type::tuple<bool, discovered_uniform> getparam_reply;
typedef msgpack::type::tuple<std::string, uniform_variant> setparam_args;
typedef bool setparam_reply;
typedef msgpack::type::tuple<bool, glm::vec3, glm::vec3, glm::vec3>
    getcamera_reply;
typedef msgpack::type::tuple<glm::vec3, glm::vec3, glm::vec3> setcamera_args;
typedef bool setcamera_reply;
typedef msgpack::type::tuple<bool, glm::vec2> getrotation_reply;
typedef glm::vec2 setrotation_args;
typedef bool setrotation_reply;
typedef msgpack::type::tuple<bool, float> getscale_reply;
typedef float setscale_args;
typedef bool setscale_reply;
typedef msgpack::type::tuple<bool, std::string> geometry_args; // 0 is true if NFF format
typedef bool geometry_reply;

class server_impl {
    static void free_msgpack(void *data, void *hint) {
        auto ptr = reinterpret_cast<msgpack::sbuffer *>(hint);
        delete ptr;
    }

   public:
    zmq::context_t context;
    zmq::socket_t socket;
    std::shared_ptr<spdlog::logger> logger;
    std::optional<getframe_args> getframe_pending;

    server_impl(const server_options &opt)
        : context(),
          socket(context, ZMQ_REP),
          logger(spdlog::stderr_color_st("server")),
          getframe_pending{} {
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

void server::handle_getframe(gl_state &gl_state, int revision, const std::string &target) const {
    std::vector<shadertoy::members::member_output_t> output;
    std::vector<shadertoy::members::member_output_t>::const_iterator output_target;
    shadertoy::output_name_t buffer_output_name = 0;

    try
    {
        // Split name on dot
        auto dot_pos = target.find('.');
        std::string target_name, output_name;

        if (dot_pos == std::string::npos) 
        {
            target_name = target;
        }
        else
        {
            target_name.assign(target.begin(), target.begin() + dot_pos);
            output_name.assign(target.begin() + dot_pos + 1, target.end());
        }

        // Get rendered-to texture
        output = gl_state.get_render_result(revision, target_name);

        // Set the output name
        if (!output_name.empty())
        {
            int id = -1;
            std::stringstream ss(output_name);
            ss >> id;

            // Set it either as a location or a name
            if (ss.fail())
                buffer_output_name = output_name;
            else
                buffer_output_name = id;
        }

        // Try to find the right output
        output_target =
            std::find_if(output.begin(), output.end(),
                         [&buffer_output_name](const auto &out) {
                             return std::get<0>(out) == buffer_output_name;
                         });

        if (output_target == output.end())
        {
            std::stringstream ss;
            std::visit(
                [&ss, &target_name](const auto &name) {
                    ss << "output target '" << name
                       << "' was not found on buffer '" << target_name << "'";
                },
                buffer_output_name);
            throw std::runtime_error(ss.str());
        }
    }
    catch (std::runtime_error &ex)
    {
        net::default_reply result(false, ex.what());
        impl_->send(result);
        return;
    }

    auto texture(std::get<1>(*output_target));

    // Get texture parameters and format
    GLint width, height, internal_format;
    texture->get_parameter(0, GL_TEXTURE_WIDTH, &width);
    texture->get_parameter(0, GL_TEXTURE_HEIGHT, &height);
    texture->get_parameter(0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);

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
    texture->get_image(0, GL_RGBA, GL_FLOAT, sz, data_msg.data());
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

            changed_state = true;
        } else {
            // Send failure response
            net::default_reply result(
                false, std::string("invalid type for param " + param_name));
            impl_->send(result);
        }
    }
}

void server::handle_getcamera(viewer_state &state) const {
    // Nothing to do, just send the camera parameters
    getcamera_reply result(true, state.camera_location, state.camera_target,
                           state.camera_up);
    impl_->send(result);
}

void server::handle_setcamera(viewer_state &state, bool &changed_state) const {
    setcamera_args args;

    try {
        args = impl_->recv<setcamera_args>();
    } catch (msgpack::type_error &ex) {
        net::default_reply result(false, "invalid camera parameters");
        impl_->send(result);
        return;
    }

    state.camera_location = args.get<0>();
    state.camera_target = args.get<1>();
    state.camera_up = args.get<2>();

    changed_state = true;

    setcamera_reply result(true);
    impl_->send(result);
}

void server::handle_getrotation(viewer_state &state) const {
    // Nothing to do, just send the rotation parameters
    getrotation_reply result(true, state.user_rotate);
    impl_->send(result);
}

void server::handle_setrotation(viewer_state &state,
                                bool &changed_state) const {
    setrotation_args args;

    try {
        args = impl_->recv<setrotation_args>();
    } catch (msgpack::type_error &ex) {
        net::default_reply result(false, "invalid rotation parameters");
        impl_->send(result);
        return;
    }

    state.user_rotate = args;
    // Disable camera rotation if we set a manual orientation, as if
    // the user clicked in the UI
    state.rotate_camera = false;

    changed_state = true;

    setrotation_reply result(true);
    impl_->send(result);
}

void server::handle_getscale(viewer_state &state) const {
    // Nothing to do, just send the scale parameters
    getscale_reply result(true, state.scale);
    impl_->send(result);
}

void server::handle_setscale(viewer_state &state, bool &changed_state) const {
    setscale_args args;

    try {
        args = impl_->recv<setscale_args>();
    } catch (msgpack::type_error &ex) {
        net::default_reply result(false, "invalid scale parameter");
        impl_->send(result);
        return;
    }

    state.scale = args;

    changed_state = true;

    setscale_reply result(true);
    impl_->send(result);
}

void server::handle_geometry(gl_state &gl_state, bool &changed_state) const
{
    geometry_args args;

    try {
        args = impl_->recv<geometry_args>();
    } catch (msgpack::type_error &ex) {
        net::default_reply result(false, std::string("invalid argument for geometry"));
        impl_->send(result);
        return;
    }

    geometry_options opts;
    if (args.get<0>())
        opts.nff_source = args.get<1>();
    else
        opts.path = args.get<1>();

    try {
        gl_state.load_geometry(opts);
    } catch (std::runtime_error &ex) {
        net::default_reply result(false, std::string("could not load geometry: ") + std::string(ex.what()));
        impl_->send(result);
        return;
    }

    changed_state = true;

    net::geometry_reply result(true);
    impl_->send(result);
}

server::server(const server_options &opt)
    : opt_(opt), impl_{std::make_unique<server_impl>(opt)} {}

server::~server() {}

bool server::poll(viewer_state &state, gl_state &gl_state, int revision) const {
    // true if we should stop reading messages and render the next frame
    bool next_frame = false;
    // true if we changed any render state, meaning the current frame does
    // not match the new render state
    bool changed_state = false;

    // Check for pending getframe that we have to reply to
    if (impl_->getframe_pending) {
        // Note that it is unlikely the rendering size changed between two
        // requests So we don't check again that gl_state.render_size matches
        // getframe_pending args
        handle_getframe(gl_state, revision, impl_->getframe_pending->get<0>());

        // Acknowledge getframe
        impl_->getframe_pending.reset();
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
                auto args = impl_->recv<getframe_args>();

                if (args.get<1>() != gl_state.render_size) {
                    // We are not rendering at the right size
                    gl_state.render_size = args.get<1>();
                    gl_state.allocate_textures();

                    // The current frame is thus invalid w.r.t the requested size
                    changed_state = true;
                }

                if (changed_state) {
                    // We changed some render state, so the user probably wants
                    // the updated result instead of the current frame
                    impl_->getframe_pending = args;
                    next_frame = true;
                } else {
                    handle_getframe(gl_state, revision, args.get<0>());
                }
            } else if (cmdname.compare(CMD_NAME_GETPARAMS) == 0) {
                handle_getparams(gl_state, revision);
            } else if (cmdname.compare(CMD_NAME_GETPARAM) == 0) {
                handle_getparam(gl_state, revision);
            } else if (cmdname.compare(CMD_NAME_SETPARAM) == 0) {
                handle_setparam(gl_state, revision, changed_state);
            } else if (cmdname.compare(CMD_NAME_GETCAMERA) == 0) {
                handle_getcamera(state);
            } else if (cmdname.compare(CMD_NAME_SETCAMERA) == 0) {
                handle_setcamera(state, changed_state);
            } else if (cmdname.compare(CMD_NAME_GETROTATION) == 0) {
                handle_getrotation(state);
            } else if (cmdname.compare(CMD_NAME_SETROTATION) == 0) {
                handle_setrotation(state, changed_state);
            } else if (cmdname.compare(CMD_NAME_GETSCALE) == 0) {
                handle_getscale(state);
            } else if (cmdname.compare(CMD_NAME_SETSCALE) == 0) {
                handle_setscale(state, changed_state);
            } else if (cmdname.compare(CMD_NAME_GEOMETRY) == 0) {
                handle_geometry(gl_state, changed_state);
            } else {
                net::default_reply result(false, "unknown command");
                impl_->send(result);
            }
        } else {
            break;
        }
    }

    // We need a new render if we either changed state or actually need a new
    // frame
    return next_frame || changed_state;
}
