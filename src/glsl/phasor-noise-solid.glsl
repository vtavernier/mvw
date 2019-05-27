[% PROCESS core/math.glsl %]
[% PROCESS core/hash.glsl %]
[% PROCESS prng/xoroshiro.glsl %]
[% PROCESS prng/poisson.glsl %]
[% SET white_poisson = 0 %]
[% PROCESS pg/3d/white.glsl %]
[% SET kernel = "trunc" %]
[% PROCESS gabor/params.glsl %]
[% PROCESS gabor/kernel.glsl %]
[% PROCESS gabor/grid.glsl %]

#define W0VEC(w0) vec3(cos(w0.x)*cos(w0.y),sin(w0.x)*cos(w0.y),sin(w0.y))
#define _TILE_SIZE vec3(gTilesize)

void cgaborCell(inout vec4 O, vec3 P, ivec3 ccell, ivec3 cell, vec3 center, vec3 n) {
    // Orientation at current world position
    vec3 w0 = W0VEC(gW0);

    // Seed the point generator
    int splats;
    pg_state pstate;

    ivec3 count = ivec3(ceil((bboxMax - bboxMin) / _TILE_SIZE));
    pg_seed(pstate, cell, count, 156237, gSplats);

    for (int i = 0; i < pg_splats(pstate); ++i)
    {
        // Get a point properties
        vec4 td_point;
        pg_point4(pstate, td_point);

        // Adjust point for tile properties
        td_point.xyz = center + _TILE_SIZE / 2. * td_point.xyz;

        // Project 3D point onto tangent plane
        vec3 v = td_point.xyz - P;
        float d = dot(v, n);

        // Relative location computed by ch3

        // Compute contribution
        O.rg += ch3(P, td_point.xyz, 1.0, gF0, w0, _TILE_SIZE, M_PI * (2. * td_point.w - 1.));
    }
}

[% grid3d_define(name="cgaborGrid",cell="cgaborCell") %]

[% PROCESS util/cgabor_main.glsl %]
