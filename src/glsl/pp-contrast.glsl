m4_include(pp/params.glsl)
m4_include(pp/lighting.glsl)

//! float cFilterLod min=1.0 max=8.0 fmt="%2.2f" cat="Contrast correction" unm="C. LOD" def=2.0
uniform float cFilterLod;

void mainImage(out vec4 O, in vec2 U)
{
    vec4 c = texture(iChannel0, U / iResolution.xy);
    vec4 fc = textureLod(iChannel0, U / iResolution.xy, cFilterLod);

    if (dGrid) {
        O = c;
        O.g = 0.;
        return;
    }

    O = vec4(2. * (c.rrr - .5) / sqrt(fc.b), c.a);
    O.rgb = .5 * O.rgb + .5;

    // Apply lighting component
    O.rgb = lighting(U, O.rgb);
}
