#include <epoxy/gl.h>

#include "mvw/mvw_geometry.hpp"

using namespace shadertoy;
using shadertoy::gl::gl_call;

mvw_geometry::mvw_geometry()
    : basic_geometry()
{
}

void mvw_geometry::add_vertex_data(const std::vector<float> &vertices, const std::vector<uint32_t> &indices) {
    mvw_mesh mesh;
    mesh.indices_size = indices.size();

    mesh.vao.bind();
    mesh.vertices.bind(GL_ARRAY_BUFFER);
    mesh.indices.bind(GL_ELEMENT_ARRAY_BUFFER);

    mesh.vertices.data(sizeof(float) * vertices.size(), vertices.data(),
                   GL_STATIC_DRAW);
    mesh.indices.data(sizeof(uint32_t) * indices.size(), indices.data(),
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
    mesh.vao.unbind();
    mesh.indices.unbind(GL_ELEMENT_ARRAY_BUFFER);
    mesh.vertices.unbind(GL_ARRAY_BUFFER);

    meshes_.emplace_back(std::move(mesh));
}

void mvw_geometry::draw() const {
    for (const auto &mesh : meshes_) {
        auto vao_bind(gl::get_bind_guard(mesh.vao));
        gl_call(glDrawElements, GL_TRIANGLES, mesh.indices_size, GL_UNSIGNED_INT, nullptr);
    }
}
