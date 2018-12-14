#ifndef _VERTEX_DATA_HPP_
#define _VERTEX_DATA_HPP_

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#pragma pack(0)
struct vertex_data {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;

    inline vertex_data()
        : position(0.f), normal(0.f), texCoords(0.f)
    {}
};
#pragma pack()

#endif /* _VERTEX_DATA_HPP_ */
