#include "discovered_bindings.hpp"

#include "log.hpp"

#include <algorithm>
#include <cctype>
#include <regex>

using std::regex;
using std::regex_search;
using std::smatch;

static const regex regex_binding("^//!\\s+(\\S+)\\s+binding=(\\S+)\\s*$");

discovered_binding::discovered_binding(std::string uniform_name,
                                       shadertoy::output_name_t target_name)
    : uniform_name(uniform_name), target_name(target_name) {}

bool try_parse_binding(const std::string &line,
                       std::vector<discovered_binding> &discovered_bindings) {
    std::smatch match;
    if (regex_search(line, match, regex_binding)) {
        auto uniform_name = match.str(1);
        auto target_name = match.str(2);
        std::string type_name;

        if (std::all_of(target_name.begin(), target_name.end(), ::isdigit)) {
            type_name = "numerical";

            // Numerical binding
            discovered_bindings.emplace_back(
                uniform_name,
                shadertoy::output_name_t(std::atoi(target_name.c_str())));
        } else {
            type_name = "string";

            // Name binding
            discovered_bindings.emplace_back(
                uniform_name, shadertoy::output_name_t(target_name));
        }

        VLOG->debug("Parsed {} binding declaration for {} -> {}",
                    type_name, uniform_name, target_name);

        return true;
    }

    return false;
}
