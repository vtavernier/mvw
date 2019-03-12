//! int gSplats min=1 max=30 def=3 fmt="%d" cat="Gabor noise" unm="Splats"
//! float gF0 min=0.001 max=100.0 fmt="%2.4f" pow=7.0 cat="Gabor noise" unm="F0" def=1.0
//! vec2 gW0 min=0 max=360 fmt="%2.2f" cat="Gabor noise" unm="W0" ang
//! bool dGrid def=false cat="Debug" unm="Show grid"

layout(location = 1) out vec4 fragLighting;

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
    O.b = O.r * O.r;



    // [0, 1] range for the noise value
    O.r = .5 * O.r + .5;

    // alpha = 1 on all shaded pixels
    O.a = 1.;

    // Overlay debug output if needed
    if (dGrid) {
        debugGrid(O, vPosition, 0, _TILE_SIZE);
    }

    // Compute fragment lighting
    fragLighting = vec4(0., 0., 0., 1.);

    // Shading for shape
    vec3 direction = normalize(vec3(1., 0., 1.));
    vec3 normal = normalize(mat3(mModel) * vNormal);
    float diffuse = max(dot(normal, direction), 0.0);

    fragLighting.rgb = vec3(.2 + .8 * max(diffuse, 0.));
}
