#include <epoxy/gl.h>

#include "mvw/mvw_geometry.hpp"

using namespace shadertoy;
using shadertoy::gl::gl_call;

mvw_geometry::mvw_geometry() : basic_geometry() {}

void mvw_geometry::add_vertex_data(const std::vector<vertex_data> &vertices,
                                   const std::vector<uint32_t> &indices) {
    mvw_mesh mesh;
    mesh.indices_size = indices.size();

    mesh.vao.bind();
    mesh.vertices.bind(GL_ARRAY_BUFFER);
    mesh.indices.bind(GL_ELEMENT_ARRAY_BUFFER);

    mesh.vertices.data(sizeof(vertex_data) * vertices.size(), vertices.data(),
                       GL_STATIC_DRAW);
    mesh.indices.data(sizeof(uint32_t) * indices.size(), indices.data(),
                      GL_STATIC_DRAW);

    // bind input "position" to vertex locations (3 floats)
    gl::attrib_location position(0);
    position.vertex_pointer(sizeof(vertex_data().position) / sizeof(float),
                            GL_FLOAT, GL_FALSE, sizeof(vertex_data),
                            (void *)(offsetof(struct vertex_data, position)));
    position.enable_vertex_array();

    // bind input "texCoord" to vertex texture coordinates (2 floats)
    gl::attrib_location texCoord(1);
    texCoord.vertex_pointer(sizeof(vertex_data().texCoords) / sizeof(float),
                            GL_FLOAT, GL_FALSE, sizeof(vertex_data),
                            (void *)(offsetof(struct vertex_data, texCoords)));
    texCoord.enable_vertex_array();

    // bind input "normals" to vertex normals (3 floats)
    gl::attrib_location normals(2);
    normals.vertex_pointer(sizeof(vertex_data().normal) / sizeof(float),
                           GL_FLOAT, GL_FALSE, sizeof(vertex_data),
                           (void *)(offsetof(struct vertex_data, normal)));
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
        gl_call(glDrawElements, GL_TRIANGLES, mesh.indices_size,
                GL_UNSIGNED_INT, nullptr);
    }
}
