#ifndef _MVW_GEOMETRY_HPP_
#define _MVW_GEOMETRY_HPP_

#include <shadertoy.hpp>

// Base class for geometry loaded by the mvw library
class mvw_geometry : public shadertoy::geometry::basic_geometry {
    // Provide subclasses direct access to the geometry fields
protected:
    /// Vertex array object
    shadertoy::gl::vertex_array vao_;
    /// Buffer objects
    shadertoy::gl::buffer vertices_;
    shadertoy::gl::buffer indices_;
    size_t indices_size_;
    glm::vec3 bbox_min_;
    glm::vec3 bbox_max_;
    glm::dvec3 bbox_centroid_;

    mvw_geometry();

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
