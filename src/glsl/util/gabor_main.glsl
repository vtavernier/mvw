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
