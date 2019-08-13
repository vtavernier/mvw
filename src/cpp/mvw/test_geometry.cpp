#include <fstream>
#include <iostream>
#include <sstream>

#include "mvw/test_geometry.hpp"

#define SYNTAX_GETL(input, line, lnum) \
    do {                               \
        std::getline(*input, line);    \
        lnum++;                        \
    } while (0)

#define SYNTAX_ASSERT(as, msg, lnum)                              \
    do {                                                          \
        if (!as) {                                                \
            std::stringstream ss;                                 \
            ss << "syntax error at line " << lnum << ": " << msg; \
            throw std::runtime_error(ss.str());                   \
        }                                                         \
    } while (0)

void test_geometry::plane_geometry(const std::string &line) {
    // Plane geometry
    std::vector<vertex_data> vertices;
    std::vector<uint32_t> indices;
    std::istringstream in(line);

    std::string s;
    in >> s; // plane

    float dim;
    in >> dim;
    if (in.fail()) dim = 100.0;

    vertices.emplace_back(glm::vec3(-dim, 0, -dim),
                          glm::vec3(0, 1, 0),
                          glm::vec2(-dim, -dim));

    vertices.emplace_back(glm::vec3(-dim, 0, dim),
                          glm::vec3(0, 1, 0),
                          glm::vec2(-dim, dim));

    vertices.emplace_back(glm::vec3(dim, 0, -dim),
                          glm::vec3(0, 1, 0),
                          glm::vec2(dim, -dim));

    vertices.emplace_back(glm::vec3(dim, 0, dim),
                          glm::vec3(0, 1, 0),
                          glm::vec2(dim, dim));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(3);

    indices.push_back(0);
    indices.push_back(3);
    indices.push_back(2);

    add_vertex_data(vertices, indices);

    // Manual bounding box
    bbox_min_ = glm::min(bbox_min_, glm::vec3(-dim));
    bbox_max_ = glm::max(bbox_max_, glm::vec3(dim));

    // Don't scale
    set_hint(HINT_NOSCALE);
    // Don't light by default
    set_hint(HINT_NOLIGHT);
}

test_geometry::test_geometry(const std::string &geometry, bool is_tst_source)
    : mvw_geometry() {
    bbox_min_ = glm::vec3(0.0f);
    bbox_max_ = glm::vec3(0.0f);
    bbox_centroid_ = glm::vec3(0.0f);

    auto input = std::unique_ptr<std::istream>(
        is_tst_source
            ? static_cast<std::istream *>(new std::istringstream(geometry))
            : static_cast<std::istream *>(new std::ifstream(geometry)));

    int lnum = 0;
    std::string line;

    // Check for marker on first line
    SYNTAX_GETL(input, line, lnum);

    bool has_marker = line.compare("#test") == 0;
    SYNTAX_ASSERT(has_marker, "invalid marker '" + line + "'", lnum);

    // Get geometry type
    SYNTAX_GETL(input, line, lnum);

    if (line.find("plane") == 0) {
        plane_geometry(line);
    } else {
        SYNTAX_ASSERT(false, "invalid geometry '" + line + "'", lnum);
    }
}
