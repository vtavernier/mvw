#include <epoxy/gl.h>

#include "mvw/mvw_geometry.hpp"

using namespace shadertoy;
using shadertoy::gl::gl_call;

mvw_geometry::mvw_geometry()
    : basic_geometry()
{
}

void mvw_geometry::load_vertex_data(const std::vector<float> &vertices, const std::vector<uint32_t> &indices) {
    indices_size_ = indices.size();

    vao_.bind();
    vertices_.bind(GL_ARRAY_BUFFER);
    indices_.bind(GL_ELEMENT_ARRAY_BUFFER);

    vertices_.data(sizeof(float) * vertices.size(), vertices.data(),
                   GL_STATIC_DRAW);
    indices_.data(sizeof(uint32_t) * indices.size(), indices.data(),
                  GL_STATIC_DRAW);

    // bind input "position" to vertex locations (3 floats)
    gl::attrib_location position(0);
    position.vertex_pointer(3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                            (void *)0);
    position.enable_vertex_array();

    // bind input "texCoord" to vertex texture coordinates (2 floats)
    gl::attrib_location texCoord(1);
    texCoord.vertex_pointer(2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                            (void *)(3 * sizeof(GLfloat)));
    texCoord.enable_vertex_array();

    // bind input "normals" to vertex normals (3 floats)
    gl::attrib_location normals(2);
    normals.vertex_pointer(3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                           (void *)(5 * sizeof(GLfloat)));
    normals.enable_vertex_array();

    // Unbind
    vao_.unbind();
    indices_.unbind(GL_ELEMENT_ARRAY_BUFFER);
    vertices_.unbind(GL_ARRAY_BUFFER);
}

void mvw_geometry::draw() const {
    gl_call(glDrawElements, GL_TRIANGLES, indices_size_, GL_UNSIGNED_INT,
            nullptr);
}
