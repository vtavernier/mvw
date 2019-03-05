#ifndef _NET_SERVER_HPP_
#define _NET_SERVER_HPP_

#include <memory>

class gl_state;

#define CMD_NAME_GETFRAME "getframe"
#define CMD_NAME_GETPARAMS "getparams"
#define CMD_NAME_GETPARAM "getparam"
#define CMD_NAME_SETPARAM "setparam"

namespace net {
class server_impl;

class server {
    std::unique_ptr<server_impl> impl_;

    void handle_getframe(gl_state &gl_state, int revision) const;

    void handle_getparams(gl_state &gl_state, int revision) const;

    void handle_getparam(gl_state &gl_state, int revision) const;

    void handle_setparam(gl_state &gl_state, int revision, bool &changed_state) const;

   public:
    server(const std::string &bind_addr);
    // non-trivial destructor because of pimpl
    ~server();

    void poll(gl_state &gl_state, int revision) const;
};
}  // namespace net

#endif /* _NET_SERVER_HPP_ */
