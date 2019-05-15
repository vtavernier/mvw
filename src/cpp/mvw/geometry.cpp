#include <epoxy/gl.h>

#include "mvw/geometry.hpp"

#include "mvw/test_geometry.hpp"
#include "mvw/assimp_geometry.hpp"

#include "log.hpp"

std::unique_ptr<mvw_geometry> make_geometry(const geometry_options &opt) {
    std::string value;
    bool is_source = false;
    bool is_test = false;

    opt.invoke(
        [&](const auto &path) {
            const std::string test_ext(".tst");

            if (path.size() > test_ext.size() &&
                std::equal(test_ext.rbegin(), test_ext.rend(), path.rbegin()))
            {
                VLOG->info("Loading test file {}", path);
                is_test = true;
            }
            else
            {
                VLOG->info("Loading model file {}", path);
            }

            value = path;
        },
        [&](const auto &source) {
            std::string nonl(source);
            for (auto &ch : nonl) {
                if (ch == '\r' || ch == '\n') {
                    ch = ' ';
                }
            }

            const std::string test_mark("#test");
            if (source.size() > test_mark.size() &&
                std::equal(test_mark.begin(), test_mark.end(), source.begin()))
            {
                VLOG->info("Loading test source <<< {} >>>", nonl);
                is_test = true;
            }
            else
            {
                VLOG->info("Loading model source <<< {} >>>", nonl);
            }

            value = source;
            is_source = true;
        });

    if (!value.empty())
        if (is_test)
            return std::make_unique<test_geometry>(value, is_source);
        else
            return std::make_unique<assimp_geometry>(value, is_source);
    else
        return {};
}
