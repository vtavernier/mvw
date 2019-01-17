#define off3(v) vec3(v.x<0?-.5:v.x==0?0:.5,v.y<0?-.5:v.y==0?0:.5,v.z<0?-.5:v.z==0?0:.5)
#define off2(v) vec2(v.x<0?-.5:v.x==0?0:.5,v.y<0?-.5:v.y==0?0:.5)

m4_define(GRID3D_DEFINE,`void $1(inout vec4 O, in vec3 P, in int max_disp, in vec3 tile_size) {
    ivec3 ccell = ivec3(P / tile_size + .5 * sign(P));
    vec3 ccenter = tile_size * (vec3(ccell) + off3(ccell));
    ivec3 disp;

    for (disp.x = -max_disp; disp.x <= max_disp; ++disp.x)
        for (disp.y = -max_disp; disp.y <= max_disp; ++disp.y)
            for (disp.z = -max_disp; disp.z <= max_disp; ++disp.z)
        {
            // Current cell coordinates
            ivec3 cell = ccell + disp;
            // Cell center (pixel coordinates)
            vec3 center = tile_size * (vec3(cell) + off3(cell));

            $2(O, P, ccell, cell, center);
        }
}')

m4_define(GRID2D_DEFINE,`void $1(inout vec4 O, in vec2 P, in int max_disp, in vec2 tile_size) {
    ivec3 ccell = ivec3(P / tile_size + .5 * sign(P));
    vec2 ccenter = tile_size * (vec2(ccell) + off2(ccell));
    ivec2 disp;

    for (disp.x = -max_disp; disp.x <= max_disp; ++disp.x)
        for (disp.y = -max_disp; disp.y <= max_disp; ++disp.y)
    {
        // Current cell coordinates
        ivec2 cell = ccell + disp;
        // Cell center (pixel coordinates)
        vec2 center = tile_size * (vec2(cell) + off2(cell));

        $2(O, P, ccell, cell, center);
    }
}')

