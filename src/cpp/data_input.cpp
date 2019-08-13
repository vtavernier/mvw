#include <shadertoy/backends/gx.hpp>

#include "data_input.hpp"

using namespace shadertoy;
namespace gx = shadertoy::backends::gx;

data_input::data_input()
    : state(dis_gpu_dirty)
{}

GLenum data_input::load_input() {
    return dims[2] == 4 ?
        GL_RGBA32F :
        dims[2] == 3 ?
        GL_RGB32F :
        GL_R32F;
}

void data_input::reset_input() {
    // nothing to do
}

gx::texture *data_input::use_input() {
    if (state != dis_gpu_uptodate) {
        // texture needs to be uploaded
        if (!tex_) {
            tex_ = backends::current()->make_texture(GL_TEXTURE_2D);
        }

        GLint internalFormat = dims[2] == 4 ?
            GL_RGBA32F :
            dims[2] == 3 ?
            GL_RGB32F :
            GL_R32F;

        GLint format = dims[2] == 4 ?
            GL_RGBA :
            dims[2] == 3 ?
            GL_RGB :
            GL_RED;

        tex_->image_2d(GL_TEXTURE_2D, 0, internalFormat,
                       dims[0], dims[1], 0, format,
                       GL_FLOAT, data.data());

        tex_->generate_mipmap();

        state = dis_gpu_uptodate;
    }

    return tex_.get();
}
