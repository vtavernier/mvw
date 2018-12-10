#include <epoxy/gl.h>

#include <iostream>

#include "tiny_geometry.hpp"

#include "tiny_obj_loader.h"

using namespace shadertoy;
using shadertoy::gl::gl_call;

tiny_geometry::tiny_geometry(const std::string &geometry_path)
    : basic_geometry() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                                geometry_path.c_str());

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    assert(ret);
    assert(shapes.size() > 0);

    const auto &mesh(shapes[0].mesh);
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    glm::vec3 d_min(0., 0., 0.),
              d_max(0., 0., 0.);
    glm::dvec3 d_centroid(0., 0., 0.);

    bool has_uv = true;

    for (const auto &index : mesh.indices) {
        glm::vec3 p(attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]);

        if (p.x < d_min.x) d_min.x = p.x;
        if (p.y < d_min.y) d_min.y = p.y;
        if (p.z < d_min.z) d_min.z = p.z;

        if (p.x > d_max.x) d_max.x = p.x;
        if (p.x > d_max.y) d_max.y = p.y;
        if (p.z > d_max.z) d_max.z = p.z;

        d_centroid += p;

        vertices.push_back(p.x);
        vertices.push_back(p.y);
        vertices.push_back(p.z);

        vertices.push_back(attrib.normals[3 * index.normal_index + 0]);
        vertices.push_back(attrib.normals[3 * index.normal_index + 1]);
        vertices.push_back(attrib.normals[3 * index.normal_index + 2]);

        if (index.texcoord_index < 0) {
            has_uv = false;
        } else if (has_uv) {
            vertices.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
            vertices.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);
        }

        indices.push_back(indices.size());
    }

    indices_size_ = indices.size();

    bbox_min_ = d_min;
    bbox_max_ = d_max;
    bbox_centroid_ = d_centroid / static_cast<double>(vertices.size() / 8);

    vao_.bind();
    vertices_.bind(GL_ARRAY_BUFFER);
    indices_.bind(GL_ELEMENT_ARRAY_BUFFER);

    vertices_.data(sizeof(float) * vertices.size(), vertices.data(),
                   GL_STATIC_DRAW);
    indices_.data(sizeof(uint32_t) * indices.size(), indices.data(),
                  GL_STATIC_DRAW);

    size_t vertex_data_stride = 6 + (has_uv ? 2 : 0);
    size_t texcoord_offset = 3;
    size_t normal_offset = texcoord_offset + (has_uv ? 2 : 0);

    // bind input "position" to vertex locations (3 floats)
    gl::attrib_location position(0);
    position.vertex_pointer(3, GL_FLOAT, GL_FALSE, vertex_data_stride * sizeof(GLfloat),
                            (void *)0);
    position.enable_vertex_array();

    if (has_uv) {
        // bind input "texCoord" to vertex texture coordinates (2 floats)
        gl::attrib_location texCoord(1);
        texCoord.vertex_pointer(2, GL_FLOAT, GL_FALSE, vertex_data_stride * sizeof(GLfloat),
                                (void *)(texcoord_offset * sizeof(GLfloat)));
        texCoord.enable_vertex_array();
    }

    // bind input "normals" to vertex normals (3 floats)
    gl::attrib_location normals(2);
    normals.vertex_pointer(3, GL_FLOAT, GL_FALSE, vertex_data_stride * sizeof(GLfloat),
                           (void *)(normal_offset * sizeof(GLfloat)));
    normals.enable_vertex_array();

    // Unbind
    vao_.unbind();
    indices_.unbind(GL_ELEMENT_ARRAY_BUFFER);
    vertices_.unbind(GL_ARRAY_BUFFER);
}

void tiny_geometry::draw() const {
    gl_call(glDrawElements, GL_TRIANGLES, indices_size_, GL_UNSIGNED_INT,
            nullptr);
}
