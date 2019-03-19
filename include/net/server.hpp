#ifndef _NET_SERVER_HPP_
#define _NET_SERVER_HPP_

#include <memory>

#include "options.hpp"

class gl_state;
class viewer_state;

#define CMD_NAME_GETFRAME "getframe"
#define CMD_NAME_GETPARAMS "getparams"
#define CMD_NAME_GETPARAM "getparam"
#define CMD_NAME_SETPARAM "setparam"
#define CMD_NAME_GETCAMERA "getcamera"
#define CMD_NAME_SETCAMERA "setcamera"
#define CMD_NAME_GETROTATION "getrotation"
#define CMD_NAME_SETROTATION "setrotation"
#define CMD_NAME_GETSCALE "getscale"
#define CMD_NAME_SETSCALE "setscale"
#define CMD_NAME_GEOMETRY "geometry"

namespace net {
class server_impl;

class server {
    const server_options &opt_;
    std::unique_ptr<server_impl> impl_;

    void handle_getframe(gl_state &gl_state, int revision, const std::string &target) const;

    void handle_getparams(gl_state &gl_state, int revision) const;

    void handle_getparam(gl_state &gl_state, int revision) const;

    void handle_setparam(gl_state &gl_state, int revision, bool &changed_state) const;

    void handle_getcamera(viewer_state &state) const;

    void handle_setcamera(viewer_state &state, bool &changed_state) const;

    void handle_getrotation(viewer_state &state) const;

    void handle_setrotation(viewer_state &state, bool &changed_state) const;

    void handle_getscale(viewer_state &state) const;

    void handle_setscale(viewer_state &state, bool &changed_state) const;

    void handle_geometry(gl_state &gl_state, bool &changed_state) const;

   public:
    server(const server_options &opt);
    // non-trivial destructor because of pimpl
    ~server();

    bool poll(viewer_state &state, gl_state &gl_state, int revision) const;
};
}  // namespace net

#endif /* _NET_SERVER_HPP_ */
