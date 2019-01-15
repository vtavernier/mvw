m4_include(core/math.glsl)
m4_include(core/hash.glsl)
m4_include(prng/lcg.glsl)
m4_include(prng/poisson.glsl)
m4_define(WHITE_POISSON,0)
m4_include(pg/3d/white.glsl)
m4_include(gabor/kernel.glsl)
m4_include(gabor/grid.glsl)

#define W0VEC(w0) vec3(cos(w0.x)*cos(w0.y),sin(w0.x)*cos(w0.y),sin(w0.y))
#define _TILE_SIZE vec3(gTilesize)

#define off(v) vec3(v.x<0?-.5:.5,v.y<0?-.5:.5,v.z<0?-.5:.5)

void gaborCell(inout vec4 O, vec3 P, ivec3 ccell, ivec3 cell, vec3 center) {
    vec3 n = normalize(vNormal);

    // Seed the point generator
    int splats;
    pg_state pstate;
    ivec3 count = ivec3(ceil((bboxMax - bboxMin) / _TILE_SIZE));
    pg_seed(pstate, cell + count, count, 156237, gSplats);

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
        O += sign(td_point.w) * dk3(td_point.xyz, 1.0, gF0, W0VEC(gW0), _TILE_SIZE);
    }
}

void debugCell(inout vec4 O, vec3 P, ivec3 ccell, ivec3 cell, vec3 center) {
    O += vec4(2. * vec3(ccell) / floor((bboxMax - bboxMin) / _TILE_SIZE), 1.);
}

void debugRot(inout vec4 O, vec3 P, ivec3 ccell, ivec3 cell, vec3 center) {
    float phase = 2. * M_PI * gF0 * dot((P - center), W0VEC(gW0));
    O = vec4(cos(phase), sin(phase), -1., 1.);
}

GRID3D_DEFINE(gaborGrid,gaborCell)
GRID3D_DEFINE(debugGrid,debugCell)
GRID3D_DEFINE(debugRotGrid,debugRot)

void mainImage(out vec4 O, in vec2 U)
{
    // Initial return value
    O = vec4(0.);

    //debugRotGrid(O, vPosition, 0, _TILE_SIZE);

    // Grid-evaluate Gabor noise
    gaborGrid(O, vPosition, 1, _TILE_SIZE);
    debugGrid(O, vPosition, 0, _TILE_SIZE);

    // Normalize output
    //O = sqrt(3.) * O / sqrt(float(gSplats));

    // Compute variance in O.a for renorm, using mipmap
    //O.a = O.x * O.x;

    // [0, 1] range
    O = .5 * O + .5;
}
