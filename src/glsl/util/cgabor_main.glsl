layout(location = 1) out vec4 fragLighting;
layout(location = 2) out vec4 phasorField;

//! float fRotate cat="Artifact fixes" unm="Rotate 90d" def=0
uniform float fRotate;

void mainImage(out vec4 O, in vec2 U)
{
    // Initial return value
    O = vec4(0.);

    //debugRotGrid(O, vPosition, 0, _TILE_SIZE);
    vec3 n = normalize(vNormal);

    vec3 x = dQuad ? vec3(U.xy, 0.) : bboxMin + vPosition;

    if (dGrid) {
        // Debug grid details
        cdebugGrid(O, x, 0, _TILE_SIZE, n);
    } else {
        // Grid-evaluate Gabor noise
        cgaborGrid(O, x, 1, _TILE_SIZE, n);

        // Normalize output
        O *= 1. / sqrt(float(gSplats));

        // [0, 1] range for the noise value
        //O.rg = .5 * (fRotate * O.rg + (1. - fRotate) * O.ba) + .5;
        O.rg = .5 * ((fRotate * (1. - length(O.ba)) + (1. - fRotate)) * O.rg
                   + (1. - length(O.rg)) * fRotate * O.ba) + .5;

        // Set phasorField value
        phasorField = vec4(2. * O.rg - 1., 0., 1.);

        O.b = 0.;

        // alpha = 1 on all shaded pixels
        O.a = 1.;
    }

    // Compute fragment lighting
    fragLighting = vec4(0., 0., 0., 1.);

    // Shading for shape
    vec3 direction = normalize(vec3(1., 0., 1.));
    vec3 normal = normalize(mat3(mModel) * vNormal);
    float diffuse = max(dot(normal, direction), 0.0);

    fragLighting.rgb = vec3(.2 + .8 * max(diffuse, 0.));
}
