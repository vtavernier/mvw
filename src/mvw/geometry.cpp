#include <epoxy/gl.h>

#include "mvw/geometry.hpp"

#include "mvw/assimp_geometry.hpp"

std::unique_ptr<mvw_geometry> make_geometry(const std::string &path)
{
    return std::make_unique<assimp_geometry>(path);
}
