//! float gTilesize min=0.01 max=10.0 fmt="%2.3f" pow=5.0 cat="Gabor noise (2019)" unm="Tile size" def=1.0
uniform float gTilesize;

[% MACRO grid3d_define BLOCK %]
void $name(inout vec4 O, in vec3 P, in int max_disp, in vec3 tile_size, in vec3 n) {
    ivec3 ccell = ivec3(P / tile_size + (dQuad ? vec3(0.) : .5 * sign(P)));
    ivec3 disp;

    for (disp.x = -max_disp; disp.x <= max_disp; ++disp.x)
        for (disp.y = -max_disp; disp.y <= max_disp; ++disp.y)
        {
            if (dQuad) {
                // Current cell coordinates
                ivec3 cell = ccell + disp;
                // Cell center (pixel coordinates)
                vec3 center = tile_size * (vec3(.5) + vec3(cell));

                $cell(O, P, ccell, cell, center, n);
            } else {
                for (disp.z = -max_disp; disp.z <= max_disp; ++disp.z)
                {
                    // Current cell coordinates
                    ivec3 cell = ccell + disp;
                    // Cell center (pixel coordinates)
                    vec3 center = tile_size * vec3(cell);

                    $cell(O, P, ccell, cell, center, n);
                }
            }
        }
}
[% END %]
