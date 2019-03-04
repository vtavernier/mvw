//! int gSplats min=1 max=30 def=3 fmt="%d" cat="Gabor noise" unm="Splats"
//! float gF0 min=0.001 max=100.0 fmt="%2.4f" pow=7.0 cat="Gabor noise" unm="F0" def=1.0
//! vec2 gW0 min=0.0 max=6.283185307179586 fmt="%2.2f" cat="Gabor noise" unm="W0"
//! bool dGrid def=false cat="Debug" unm="Show grid"

void mainImage(out vec4 O, in vec2 U)
{
    // Initial return value
    O = vec4(0.);

    //debugRotGrid(O, vPosition, 0, _TILE_SIZE);

    // Grid-evaluate Gabor noise
    gaborGrid(O, bboxMin + vPosition, 1, _TILE_SIZE);

    // Normalize output
    O = 1. / sqrt(3.) * O / sqrt(float(gSplats));

    // Compute variance in O.b for renorm, using mipmap
    O.b = O.x * O.x;

    // Shading for shape, store in g
    vec3 direction = normalize(vec3(1., 0., 1.));
    vec3 normal = normalize(mat3(mModel) * vNormal);
    float diffuse = max(dot(normal, direction), 0.0);

    O.g = (.2 + .8 * max(diffuse, 0.));

    // [0, 1] range for the noise value
    O.r = .5 * O.r + .5;

    // alpha = 1 on all shaded pixels
    O.a = 1.;

    if (dGrid) {
        debugGrid(O, vPosition, 0, _TILE_SIZE);
    }
}
