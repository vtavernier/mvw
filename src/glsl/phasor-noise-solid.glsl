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

//! vec2 ePosition min=0 max=1 cat="Phasor fix" unm="Position"
uniform vec2 ePosition;
//! float eAngle min=-90 max=90 cat="Phasor fix" unm="Angle" mod=angle
uniform float eAngle;
//! float eSize min=0 max=1 def=1 cat="Phasor fix" unm="Overlay scale"
uniform float eSize;
//! float eNorm min=0 max=1 cat="Phasor fix" unm="Intensity"
uniform float eNorm;

void cgaborCell(inout vec4 O, vec3 P, ivec3 ccell, ivec3 cell, vec3 center, vec3 n) {
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
        vec3 w1 = GETW0(gW0 + vec2(M_PI, 0.), .5 + .5 * td_extra.x);

        // On a quad, don't offset points in the z direction
        if (dQuad) {
            td_point.z = 0.;
        }

        // Adjust point for tile properties
        td_point.xyz = center + _TILE_SIZE / 2. * td_point.xyz;

        // Relative location computed by ch3

        // Compute contribution
        O += ch3(P, td_point.xyz, 1.0, gF0, w0, w1, _TILE_SIZE, M_PI * (2. * td_point.w - 1.));
    }

    vec2 w = (gW0 + vec2(eAngle, 0.));
    vec2 ww = (gW0 + vec2(eAngle, 0.) + vec2(M_PI/2., 0.));
    O += eNorm * ch3(P, vec3(ePosition, 0.), 1.0, gF0, GETW0(w, 0.), GETW0(ww, 0.), eSize * _TILE_SIZE, 0.);
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
