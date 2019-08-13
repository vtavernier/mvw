#include "mvw/mvw_geometry.hpp"

#include "log.hpp"

using namespace shadertoy;

mvw_geometry::mvw_mesh::mvw_mesh()
    : vao(backends::current()->make_vertex_array()),
    vertices(backends::current()->make_buffer(GL_ARRAY_BUFFER)),
    indices(backends::current()->make_buffer(GL_ELEMENT_ARRAY_BUFFER))
{
}

mvw_geometry::mvw_geometry() : basic_geometry() {}

void mvw_geometry::add_vertex_data(const std::vector<vertex_data> &vertices,
                                   const std::vector<uint32_t> &indices) {
    mvw_mesh mesh;
    mesh.indices_size = indices.size();

    mesh.vao->bind();
    mesh.vertices->bind(GL_ARRAY_BUFFER);
    mesh.indices->bind(GL_ELEMENT_ARRAY_BUFFER);

    mesh.vertices->data(sizeof(vertex_data) * vertices.size(), vertices.data(),
                       GL_STATIC_DRAW);
    mesh.indices->data(sizeof(uint32_t) * indices.size(), indices.data(),
                      GL_STATIC_DRAW);

    // bind input "position" to vertex locations (3 floats)
    auto position(backends::current()->make_attrib_location(0));
    position->vertex_pointer(sizeof(vertex_data().position) / sizeof(float),
                            GL_FLOAT, GL_FALSE, sizeof(vertex_data),
                            (void *)(offsetof(struct vertex_data, position)));
    position->enable_vertex_array();

    // bind input "texCoord" to vertex texture coordinates (2 floats)
    auto texCoord(backends::current()->make_attrib_location(1));
    texCoord->vertex_pointer(sizeof(vertex_data().texCoords) / sizeof(float),
                            GL_FLOAT, GL_FALSE, sizeof(vertex_data),
                            (void *)(offsetof(struct vertex_data, texCoords)));
    texCoord->enable_vertex_array();

    // bind input "normals" to vertex normals (3 floats)
    auto normals(backends::current()->make_attrib_location(2));
    normals->vertex_pointer(sizeof(vertex_data().normal) / sizeof(float),
                           GL_FLOAT, GL_FALSE, sizeof(vertex_data),
                           (void *)(offsetof(struct vertex_data, normal)));
    normals->enable_vertex_array();

    // Unbind
    mesh.vao->unbind();
    mesh.indices->unbind(GL_ELEMENT_ARRAY_BUFFER);
    mesh.vertices->unbind(GL_ARRAY_BUFFER);

    meshes_.emplace_back(std::move(mesh));
}

void mvw_geometry::set_hint(const std::string &hint, hint_value value) {
    hints_[hint] = value;
    std::visit([&](const auto &v) {
        VLOG->debug("Setting hint {} to {}", hint, v);
    }, value);
}

void mvw_geometry::draw() const {
    for (const auto &mesh : meshes_) {
        mesh.vao->draw_elements(GL_TRIANGLES, mesh.indices_size, GL_UNSIGNED_INT, nullptr);
    }
}

bool mvw_geometry::has_hint(const std::string &hint) const {
    return hints_.find(hint) != hints_.end();
}

std::optional<hint_value> mvw_geometry::hint_val(const std::string &hint) const {
    auto it = hints_.find(hint);

    if (it == hints_.end())
        return std::nullopt;
    return it->second;
}
