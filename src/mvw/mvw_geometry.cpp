#include <epoxy/gl.h>

#include "mvw/mvw_geometry.hpp"

using namespace shadertoy;
using shadertoy::gl::gl_call;

mvw_geometry::mvw_geometry()
    : basic_geometry()
{
}

void mvw_geometry::draw() const {
    gl_call(glDrawElements, GL_TRIANGLES, indices_size_, GL_UNSIGNED_INT,
            nullptr);
}
