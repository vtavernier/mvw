#ifndef _TINY_GEOMETRY_HPP_
#define _TINY_GEOMETRY_HPP_

#include <shadertoy.hpp>

// Type that will manage the geometry
class tiny_geometry : public shadertoy::geometry::basic_geometry {
    /// Vertex array object
    shadertoy::gl::vertex_array vao_;
    /// Buffer objects
    shadertoy::gl::buffer vertices_;
    shadertoy::gl::buffer indices_;
    size_t indices_size_;
    glm::vec3 bbox_min_;
    glm::vec3 bbox_max_;
    glm::dvec3 bbox_centroid_;

   public:
    tiny_geometry(const std::string &geometry_path);

    inline const shadertoy::gl::vertex_array &vertex_array() const { return vao_; }

    void draw() const final;

    inline void get_dimensions(glm::vec3 &bbox_min, glm::vec3 &bbox_max) const {
        bbox_min = bbox_min_;
        bbox_max = bbox_max_;
    }

    inline glm::dvec3 get_centroid() const { return bbox_centroid_; }
};

#endif /* _TINY_GEOMETRY_HPP_ */
