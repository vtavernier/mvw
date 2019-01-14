m4_include(core/math.glsl)
m4_include(prng/none.glsl)
m4_include(prng/poisson.glsl)
m4_define(WHITE_POISSON,1)
m4_include(pg/3d/white.glsl)
m4_include(gabor/kernel.glsl)
m4_include(gabor/grid.glsl)

#define W0VEC(w0) vec3(cos(w0), sin(w0), 0.)
#define _TILE_SIZE vec3(gTilesize)

#define off(v) vec3(v.x<0?-.5:.5,v.y<0?-.5:.5,v.z<0?-.5:.5)

void gaborCell(inout vec4 O, ivec3 ccell, ivec3 cell, vec3 center) {
    vec3 n = normalize(vNormal);

    // Seed the point generator
    int splats;
    pg_state pstate;
    pg_seed(pstate, cell, ivec3(ceil((bboxMax - bboxMin) / _TILE_SIZE)), 0, gSplats);

    for (int i = 0; i < pg_splats(pstate); ++i)
    {
        // Get a point properties
        vec4 td_point;
        vec2 props;
        pg_point4(pstate, td_point);

        // Project 3D point onto tangent plane
        vec3 v = td_point.xyz - vPosition;
        float d = dot(v, n);
        td_point.xyz = v - d * n;

        // Adjust point for tile properties
        td_point.xyz = center + _TILE_SIZE / 2. * td_point.xyz;

        // Compute relative location
        td_point.xyz = (vPosition - td_point.xyz) / _TILE_SIZE;

        // Compute contribution
        O += props.x * h3(td_point.xyz, 1.0, gF0, W0VEC(gW0), _TILE_SIZE);
    }
}

void debugCell(inout vec4 O, ivec3 ccell, ivec3 cell, vec3 center) {
    O = vec4(2. * vec3(ccell) / floor((bboxMax - bboxMin) / _TILE_SIZE), 1.);
}

GRID3D_DEFINE(gaborGrid,gaborCell)
GRID3D_DEFINE(debugGrid,debugCell)

void mainImage(out vec4 O, in vec2 U)
{
    // Initial return value
    O = vec4(0.);

    debugGrid(O, vPosition, 1, _TILE_SIZE);

    // Grid-evaluate Gabor noise
    //gaborGrid(O, vPosition, 1, _TILE_SIZE);

    // Normalize output
    //O = O / sqrt(float(gSplats));

    // Compute variance in O.a for renorm, using mipmap
    //O.a = O.x * O.x;

    // [0, 1] range
    O = .5 * O + .5;
}
