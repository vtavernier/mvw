#ifndef _ASSIMP_GEOMETRY_HPP_
#define _ASSIMP_GEOMETRY_HPP_

#include "mvw/mvw_geometry.hpp"

class assimp_geometry : public mvw_geometry {
   public:
    assimp_geometry(const std::string &geometry, bool is_nff_source = false);
};

#endif /* _ASSIMP_GEOMETRY_HPP_ */
