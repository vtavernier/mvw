#ifndef _TEST_GEOMETRY_HPP_
#define _TEST_GEOMETRY_HPP_

#include "mvw/mvw_geometry.hpp"

class test_geometry : public mvw_geometry {
    void plane_geometry(const std::string &line);

   public:
    test_geometry(const std::string &geometry, bool is_tst_source = false);
};

#endif /* _TEST_GEOMETRY_HPP_ */
