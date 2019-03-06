#ifndef _GEOMETRY_HPP_
#define _GEOMETRY_HPP_

#include <memory>

#include "mvw_geometry.hpp"

#include "options.hpp"

std::unique_ptr<mvw_geometry> make_geometry(const geometry_options &opt);

#endif /* _GEOMETRY_HPP_ */
