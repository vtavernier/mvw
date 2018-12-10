#include <epoxy/gl.h>

#include <boost/filesystem.hpp>

#include "mvw/geometry.hpp"

#include "mvw/tiny_geometry.hpp"

namespace fs = boost::filesystem;

std::unique_ptr<mvw_geometry> make_geometry(const std::string &path)
{
    fs::path p(path);
    auto ext(p.extension());
    std::unique_ptr<mvw_geometry> result;

    if (ext == ".obj") {
        result = std::make_unique<tiny_geometry>(path);
    } else {
        std::stringstream ss;
        ss << "Unsupported model type: " << ext;
        throw std::runtime_error(ss.str());
    }

    return result;
}
