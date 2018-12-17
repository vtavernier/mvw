#ifndef _MVW_GEOMETRY_HPP_
#define _MVW_GEOMETRY_HPP_

#include <shadertoy.hpp>

#include "mvw/vertex_data.hpp"

// Base class for geometry loaded by the mvw library
class mvw_geometry : public shadertoy::geometry::basic_geometry {
    // Provide subclasses direct access to the geometry fields
    struct mvw_mesh {
        shadertoy::gl::vertex_array vao;
        shadertoy::gl::buffer vertices;
        shadertoy::gl::buffer indices;
        size_t indices_size;
    };

    /// Default VAO: actually unused
    shadertoy::gl::vertex_array vao_;

    /// List of meshes to render
    std::vector<mvw_mesh> meshes_;

   protected:
    glm::vec3 bbox_min_;
    glm::vec3 bbox_max_;
    glm::dvec3 bbox_centroid_;

    mvw_geometry();

    void add_vertex_data(const std::vector<vertex_data> &vertices,
                         const std::vector<uint32_t> &indices);

   public:
    inline const shadertoy::gl::vertex_array &vertex_array() const {
        return vao_;
    }

    inline void get_dimensions(glm::vec3 &bbox_min, glm::vec3 &bbox_max) const {
        bbox_min = bbox_min_;
        bbox_max = bbox_max_;
    }

    inline glm::dvec3 get_centroid() const { return bbox_centroid_; }

    void draw() const override;
};

#endif /* _MVW_GEOMETRY_HPP_ */
