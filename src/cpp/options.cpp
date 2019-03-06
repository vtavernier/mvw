#include <fstream>
#include <sstream>

#include "options.hpp"

bool shader_file_program::empty() const {
    return path.empty() && source.empty();
}

std::unique_ptr<std::istream> shader_file_program::open() const {
    if (source.empty()) {
        if (path.empty()) {
            throw std::runtime_error("cannot open empty shader_file_program");
        }

        return std::make_unique<std::ifstream>(path);
    }

    return std::make_unique<std::stringstream>(source);
}
