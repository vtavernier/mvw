//! float aSigma min=0.01 max=2.0 fmt="%2.3f" cat="Prefiltering" unm="Sigma" def=1.0
uniform float aSigma;

layout(location = 1) out vec4 fragLighting;

void mainImage(out vec4 O, in vec2 U)
{
    // Initial return value
    O = vec4(0.);

    //debugRotGrid(O, vPosition, 0, _TILE_SIZE);

    // Orientation at current world position
    vec3 w0 = W0VEC(gW0);

    // Grid-evaluate Gabor noise
    gaborGrid(O, bboxMin + vPosition, 1, _TILE_SIZE, w0);

    // Normalize output
    O = sqrt(3.) * O / sqrt(float(gSplats));

    // Compute variance in O.b for renorm, using mipmap
    O.b = O.r * O.r;

    // Compute prefiltering at current pixel
    vec3 n = normalize(vNormal);
    vec3 w0p0 = w0 - dot(w0, n) * n;

    float p = dot(vPosition, w0);
    vec2 w0p = abs(vec2(dFdx(p), dFdy(p)));
    float w0pn = length(w0p);

    vec2 wP = 2. * M_PI * gF0 * w0p;
    float f = exp(-pow(length(w0p0) * length(wP) / aSigma, 2.) / 2.);

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
