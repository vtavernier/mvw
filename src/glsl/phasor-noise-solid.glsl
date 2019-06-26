[% PROCESS core/debug.glsl %]
[% PROCESS core/math.glsl %]
[% IF ptgen == "data" %]
[% PROCESS pg/3d/data.glsl %]
[% ELSE %]
[% PROCESS core/hash.glsl %]
[% PROCESS prng/xoroshiro.glsl %]
[% PROCESS prng/poisson.glsl %]
[% SET white_poisson = 0 %]
[% PROCESS pg/3d/white.glsl %]
[% END %]
[% SET kernel = "trunc" %]
[% PROCESS gabor/params.glsl %]
[% PROCESS gabor/kernel.glsl %]
[% PROCESS gabor/grid.glsl %]
[% PROCESS gabor/orient.glsl %]

#define _TILE_SIZE vec3(gTilesize)

void cgaborCell(inout vec4 O, vec3 P, ivec3 ccell, ivec3 cell, vec3 center, vec3 n) {
    // Seed the point generator
    int splats;
    pg_state pstate;

    ivec3 count = ivec3((dQuad ? vec3(1.0, 1.0, 0.0) : ceil(bboxMax - bboxMin)) / _TILE_SIZE);
    pg_seed(pstate, cell, count, 156237, gSplats);

    for (int i = 0; i < pg_splats(pstate); ++i)
    {
        // Get a point properties
        vec4 td_point;
        vec2 td_extra;
        pg_point6(pstate, td_point, td_extra);

        // Orientation at current world position
        vec3 w0 = GETW0(gW0, td_extra.x);

        // On a quad, don't offset points in the z direction
        if (dQuad) {
            td_point.z = 0.;
        }

        // Adjust point for tile properties
        td_point.xyz = center + _TILE_SIZE / 2. * td_point.xyz;

        // Project 3D point onto tangent plane
        vec3 v = td_point.xyz - P;
        float d = dot(v, n);

        // Relative location computed by ch3

        // Compute contribution
        O += ch3(P, td_point.xyz, 1.0, gF0, w0, _TILE_SIZE, M_PI * (2. * td_point.w - 1.));
    }

}

void cdebugCell(inout vec4 O, vec3 P, ivec3 ccell, ivec3 cell, vec3 center, vec3 n) {
    // Seed the point generator
    int splats;
    pg_state pstate;

    ivec3 count = ivec3((dQuad ? vec3(1.0, 1.0, 0.0) : ceil(bboxMax - bboxMin)) / _TILE_SIZE);
    pg_seed(pstate, cell, count, 156237, gSplats);

    int seed = int(cell.z * count.x * count.y + cell.y * count.x + cell.x);

    O = vec4(vec2(cell) / vec2(max(ivec2(1), count.xy) - 1), seed, 1.);
}

[% grid3d_define(name="cgaborGrid",cell="cgaborCell") %]
[% grid3d_define(name="cdebugGrid",cell="cdebugCell") %]

[% PROCESS util/cgabor_main.glsl %]
