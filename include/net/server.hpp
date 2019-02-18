#ifndef _NET_SERVER_HPP_
#define _NET_SERVER_HPP_

#include <memory>

class gl_state;

namespace net {
struct server_impl;

class server {
    std::unique_ptr<server_impl> impl_;

   public:
    server(const std::string &bind_addr);
    // non-trivial destructor because of pimpl
    ~server();

    void poll(gl_state &gl_state, int revision);
};
}  // namespace net

#endif /* _NET_SERVER_HPP_ */
