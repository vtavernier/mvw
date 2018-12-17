#ifndef _CUBEMAP_BUFFER_HPP_
#define _CUBEMAP_BUFFER_HPP_

#include <shadertoy.hpp>

class cubemap_buffer : public shadertoy::buffers::toy_buffer
{
protected:
    // Override output attach method to bind all faces
    void attach_framebuffer_outputs(const shadertoy::gl::bound_ops<shadertoy::gl::framebuffer> &target_fbo,
                                    const shadertoy::gl::texture &texture) override;

public:
    cubemap_buffer(const std::string &id);
};

#endif /* _CUBEMAP_BUFFER_HPP_ */
