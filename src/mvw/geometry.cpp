#include <epoxy/gl.h>

#include <boost/filesystem.hpp>

#include "mvw/geometry.hpp"

#include "mvw/tiny_geometry.hpp"

#include "mvw/assimp_geometry.hpp"

namespace fs = boost::filesystem;

#ifndef MVW_PREFER_LOCAL_IMPORTERS
#define MVW_PREFER_LOCAL_IMPORTERS 0
#endif /* MVW_PREFER_LOCAL_IMPORTERS */

std::unique_ptr<mvw_geometry> make_geometry(const std::string &path)
{
    fs::path p(path);
    auto ext(p.extension());
    std::unique_ptr<mvw_geometry> result;

#if MVW_PREFER_LOCAL_IMPORTERS
    if (ext == ".obj") {
        result = std::make_unique<tiny_geometry>(path);
    } else {
#endif /* MVW_PREFER_LOCAL_IMPORTERS */
        result = std::make_unique<assimp_geometry>(path);
#if MVW_PREFER_LOCAL_IMPORTERS
    }
#endif /* MVW_PREFER_LOCAL_IMPORTERS */

    return result;
}
