layout(location = 1) out vec4 fragLighting;

void mainImage(out vec4 O, in vec2 U)
{
    // Initial return value
    O = vec4(0.);

    //debugRotGrid(O, vPosition, 0, _TILE_SIZE);
    vec3 n = normalize(vNormal);

    // Grid-evaluate Gabor noise
    gaborGrid(O, bboxMin + vPosition, 1, _TILE_SIZE, n);

    // Normalize output
    O.rb *= .5 * sqrt(3.) * 3. / (4. * sqrt((1. - exp(-2. * M_PI * gF0 * gF0 * gTilesize * gTilesize)) * float(gSplats)));

    // Compute variance in O.b for renorm, using mipmap
    // We use the non-prefiltered value of the noise for that
    O.b = O.b * O.b;

    // [0, 1] range for the noise value
    O.r = .5 * O.r + .5;

    // alpha = 1 on all shaded pixels
    O.a = 1.;

    // Overlay debug output if needed
    if (dGrid) {
        debugGrid(O, vPosition, 0, _TILE_SIZE, vec3(0.));
    }

    // Compute fragment lighting
    fragLighting = vec4(0., 0., 0., 1.);

    // Shading for shape
    vec3 direction = normalize(vec3(1., 0., 1.));
    vec3 normal = normalize(mat3(mModel) * vNormal);
    float diffuse = max(dot(normal, direction), 0.0);

    fragLighting.rgb = vec3(.2 + .8 * max(diffuse, 0.));
}
