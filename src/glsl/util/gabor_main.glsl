//! int gSplats min=1 max=30 def=3 fmt="%d" cat="Gabor noise" unm="Splats"
//! float gF0 min=0.001 max=100.0 fmt="%2.4f" pow=7.0 cat="Gabor noise" unm="F0" def=1.0
//! vec2 gW0 min=0 max=360 fmt="%2.2f" cat="Gabor noise" unm="W0" ang
//! bool dGrid def=false cat="Debug" unm="Show grid"

//! float aSigma min=0.01 max=2.0 fmt="%2.3f" cat="Prefiltering" unm="Sigma" def=1.0 pow=2.0

layout(location = 1) out vec4 fragLighting;

void mainImage(out vec4 O, in vec2 U)
{
    // Initial return value
    O = vec4(0.);

    //debugRotGrid(O, vPosition, 0, _TILE_SIZE);

    // Compute prefiltering at current pixel
    vec3 n = normalize(vNormal);
    vec3 w0 = W0VEC(gW0);
    vec3 w0p0 = w0 - dot(w0, n) * n;
    float w0p0n = length(w0p0);

    vec2 wP = 2. * M_PI * gF0 * vec2(length(dFdx(w0p0)), length(dFdy(w0p0)));
    float f = exp(-pow(w0p0n * length(wP) / aSigma, 2.) / 2.);

    // Grid-evaluate Gabor noise
    gaborGrid(O, bboxMin + vPosition, 1, _TILE_SIZE, w0);

    // Normalize output
    O = sqrt(3.) * O / sqrt(float(gSplats));

    // Compute variance in O.b for renorm, using mipmap
    O.b = O.r * O.r;

    // Apply prefiltering to the noise value
    O.r *= f;

    // Return filter strength in green channel
    O.g = f;

    // [0, 1] range for the noise value
    O.r = .5 * O.r + .5;

    // alpha = 1 on all shaded pixels
    O.a = 1.;

    // Overlay debug output if needed
    if (dGrid) {
        debugGrid(O, vPosition, 0, _TILE_SIZE, w0);
    }

    // Compute fragment lighting
    fragLighting = vec4(0., 0., 0., 1.);

    // Shading for shape
    vec3 direction = normalize(vec3(1., 0., 1.));
    vec3 normal = normalize(mat3(mModel) * vNormal);
    float diffuse = max(dot(normal, direction), 0.0);

    fragLighting.rgb = vec3(.2 + .8 * max(diffuse, 0.));
}
