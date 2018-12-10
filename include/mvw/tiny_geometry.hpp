#ifndef _TINY_GEOMETRY_HPP_
#define _TINY_GEOMETRY_HPP_

#include "mvw/mvw_geometry.hpp"

// Type that will manage the geometry
class tiny_geometry : public mvw_geometry {
   public:
    tiny_geometry(const std::string &geometry_path);
};

#endif /* _TINY_GEOMETRY_HPP_ */
