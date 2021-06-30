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

//! float aSigma min=0.01 max=2.0 fmt="%2.3f" cat="Prefiltering" unm="Sigma" def=1.0
uniform float aSigma;

void gaborCell(inout vec4 O, vec3 P, ivec3 ccell, ivec3 cell, vec3 center, vec3 n) {
    // Seed the point generator
    int splats;
    pg_state pstate;

    ivec3 count = ivec3((dQuad ? vec3(1.0, 1.0, 0.0) : ceil(bboxMax - bboxMin)) / _TILE_SIZE);
    pg_seed(pstate, cell, count, 156237, gSplats);

    for (int i = 0; i < pg_splats(pstate); ++i)
    {
        // Get a point properties
        vec4 td_point, td_extra;
        pg_point4(pstate, td_point);
        pg_point4(pstate, td_extra);

        // Orientation at current world position
        vec3 w0 = GETW0(gW0, .5 + .5 * td_extra.x);

        // Compute prefiltering at current pixel
        vec3 w0p0 = w0 - dot(w0, n) * n;

        float p = dot(vPosition, w0);
        vec2 w0p = abs(vec2(dFdx(p), dFdy(p)));
        float w0pn = length(w0p);

        vec2 wP = 2. * M_PI * gF0 * w0p;
        float f = exp(-pow(length(w0p0) * length(wP) / aSigma, 2.) / 2.);

        // On a quad, don't offset points in the z direction
        if (dQuad) {
            td_point.z = 0.;
        }

        // Adjust point for tile properties
        td_point.xyz = center + _TILE_SIZE / 2. * td_point.xyz;

        // Project 3D point onto tangent plane
        vec3 v = td_point.xyz - P;
        float d = dot(v, n);

        // Compute relative location
        td_point.xyz = (P - td_point.xyz) / _TILE_SIZE;

        // Compute prefiltered contribution
        float noise = h3(td_point.xyz, 1.0, gF0, w0, _TILE_SIZE, M_PI * (2. * td_point.w - 1.));
        O.r += f * noise;
        O.g += f;
        O.b += noise;
    }
}

[% grid3d_define(name="gaborGrid",cell="gaborCell") %]

[% PROCESS util/debug.glsl %]
[% PROCESS util/gabor_main.glsl %]
