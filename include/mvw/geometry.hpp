#ifndef _GEOMETRY_HPP_
#define _GEOMETRY_HPP_

#include <memory>

#include "mvw_geometry.hpp"

std::unique_ptr<mvw_geometry> make_geometry(const std::string &path);

#endif /* _GEOMETRY_HPP_ */
