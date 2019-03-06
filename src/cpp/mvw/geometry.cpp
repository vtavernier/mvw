#include <epoxy/gl.h>

#include "mvw/geometry.hpp"

#include "mvw/assimp_geometry.hpp"

#include "log.hpp"

std::unique_ptr<mvw_geometry> make_geometry(const geometry_options &opt) {
    std::string value;
    bool is_source = false;

    opt.invoke(
        [&](const auto &path) {
            VLOG->info("Loading model file {}", path);
            value = path;
        },
        [&](const auto &source) {
            std::string nonl(source);
            for (auto &ch : nonl) {
                if (ch == '\r' || ch == '\n') {
                    ch = ' ';
                }
            }

            VLOG->info("Loading model source <<< {} >>>", nonl);
            value = source;
            is_source = true;
        });

    if (!value.empty())
        return std::make_unique<assimp_geometry>(value, is_source);
    else
        return {};
}
