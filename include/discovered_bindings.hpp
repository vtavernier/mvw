#ifndef _DISCOVERED_BINDINGS_HPP_
#define _DISCOVERED_BINDINGS_HPP_

#include <cstdint>
#include <string>
#include <vector>

#include <shadertoy/output_name.hpp>

struct discovered_binding {
    std::string uniform_name;
    shadertoy::output_name_t target_name;

    discovered_binding(std::string uniform_name,
                       shadertoy::output_name_t target_name);
};

bool try_parse_binding(const std::string &line,
                       std::vector<discovered_binding> &discovered_bindings);

#endif /* _DISCOVERED_BINDINGS_HPP_ */
