m4_define(GRID3D_DEFINE,`void $1(inout vec4 O, in vec3 P, in int max_disp, in vec3 tile_size) {
    ivec3 ccell = ivec3(P / tile_size);
    vec3 ccenter = tile_size * (vec3(ccell) + off(ccell));
    ivec3 disp;

    for (disp.x = -max_disp; disp.x <= max_disp; ++disp.x)
        for (disp.y = -max_disp; disp.y <= max_disp; ++disp.y)
            for (disp.z = -max_disp; disp.z <= max_disp; ++disp.z)
        {
            // Current cell coordinates
            ivec3 cell = ccell + disp;
            // Cell center (pixel coordinates)
            vec3 center = tile_size * (vec3(cell) + off(cell));

            $2(O, ccell, cell, center);
        }
}')

m4_define(GRID2D_DEFINE,`void $1(inout vec4 O, in vec2 P, in int max_disp, in vec2 tile_size) {
    ivec2 ccell = ivec2(P / tile_size);
    vec2 ccenter = tile_size * (vec2(ccell) + off(ccell));
    ivec2 disp;

    for (disp.x = -max_disp; disp.x <= max_disp; ++disp.x)
        for (disp.y = -max_disp; disp.y <= max_disp; ++disp.y)
    {
        // Current cell coordinates
        ivec2 cell = ccell + disp;
        // Cell center (pixel coordinates)
        vec2 center = tile_size * (vec2(cell) + off(cell));

        $2(O, ccell, cell, center);
    }
}')

