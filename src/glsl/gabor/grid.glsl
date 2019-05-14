//! float gTilesize min=0.01 max=10.0 fmt="%2.3f" pow=5.0 cat="Gabor noise (2019)" unm="Tile size" def=1.0
uniform float gTilesize;

m4_define(GRID3D_DEFINE,`void $1(inout vec4 O, in vec3 P, in int max_disp, in vec3 tile_size, in vec3 w0) {
    ivec3 ccell = ivec3(P / tile_size + .5 * sign(P));
    ivec3 disp;

    for (disp.x = -max_disp; disp.x <= max_disp; ++disp.x)
        for (disp.y = -max_disp; disp.y <= max_disp; ++disp.y)
            for (disp.z = -max_disp; disp.z <= max_disp; ++disp.z)
        {
            // Current cell coordinates
            ivec3 cell = ccell + disp;
            // Cell center (pixel coordinates)
            vec3 center = tile_size * vec3(cell);

            $2(O, P, ccell, cell, center, w0);
        }
}')
