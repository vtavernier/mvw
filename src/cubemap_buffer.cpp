#include <epoxy/gl.h>

#include "cubemap_buffer.hpp"

using namespace shadertoy;
using shadertoy::gl::gl_call;

void cubemap_buffer::attach_framebuffer_outputs(
    const gl::bound_ops<gl::framebuffer> &target_fbo,
    const gl::texture &texture) {
    target_fbo.resource().texture(GL_COLOR_ATTACHMENT0, texture, 0);
}

