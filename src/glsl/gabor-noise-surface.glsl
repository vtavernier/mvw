[% PROCESS core/math.glsl %]
[% PROCESS core/hash.glsl %]
[% PROCESS prng/xorshift.glsl %]
[% PROCESS prng/poisson.glsl %]
[% SET white_poisson = 0 %]
[% PROCESS pg/3d/white.glsl %]
[% PROCESS gabor/kernel.glsl %]
[% PROCESS gabor/grid.glsl %]

#define W0VEC(w0) vec3(cos(w0.x)*cos(w0.y),sin(w0.x)*cos(w0.y),sin(w0.y))
#define _TILE_SIZE vec3(gTilesize)

void gaborCell(inout vec4 O, vec3 P, ivec3 ccell, ivec3 cell, vec3 center) {
    vec3 n = normalize(vNormal);

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
        td_point.xyz = v - d * n;

        // Compute relative location
        td_point.xyz /= _TILE_SIZE;

        // Compute contribution
        if (abs(d) < _TILE_SIZE.x)
            O += sign(td_point.w) * smoothstep(_TILE_SIZE.x, 0., abs(d)) * h3(td_point.xyz, 1.0, gF0, W0VEC(gW0), _TILE_SIZE, 0.);
    }
}

[% grid3d_define(name="gaborGrid",cell="gaborCell") %]

[% PROCESS util/debug.glsl %]
[% PROCESS util/gabor_main.glsl %]
