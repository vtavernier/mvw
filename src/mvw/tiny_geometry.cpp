#include <epoxy/gl.h>

#include <iostream>

#include "mvw/tiny_geometry.hpp"

#include "tiny_obj_loader.h"

using namespace shadertoy;
using shadertoy::gl::gl_call;

tiny_geometry::tiny_geometry(const std::string &geometry_path)
    : mvw_geometry() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                                geometry_path.c_str());

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    (void)(ret); // Prevent -Wunused-variable in release
    assert(ret);
    assert(shapes.size() > 0);

    glm::vec3 d_min(0., 0., 0.),
              d_max(0., 0., 0.);
    glm::dvec3 d_centroid(0., 0., 0.);
    size_t centroid_count = 0;

    for (size_t i = 0; i < shapes.size(); ++i) {
        const auto &mesh(shapes[i].mesh);
        std::vector<float> vertices;
        std::vector<uint32_t> indices;

        bool has_uv = true;

        vertices.reserve(mesh.indices.size() * 8);
        indices.reserve(mesh.indices.size());

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

                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            } else if (has_uv) {
                vertices.push_back(attrib.texcoords[2 * index.texcoord_index + 0]);
                vertices.push_back(attrib.texcoords[2 * index.texcoord_index + 1]);
            }

            indices.push_back(indices.size());
        }

        centroid_count += vertices.size() / 8;

        add_vertex_data(vertices, indices);
    }

    bbox_min_ = d_min;
    bbox_max_ = d_max;
    if (centroid_count > 0)
        bbox_centroid_ = d_centroid / static_cast<double>(centroid_count);
}
