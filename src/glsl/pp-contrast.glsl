m4_include(pp/params.glsl)
m4_include(pp/lighting.glsl)

//! float cFilterLod min=1.0 max=8.0 fmt="%2.2f" cat="Contrast correction" unm="C. LOD" def=2.0
uniform float cFilterLod;

//! bool cAdaptiveWindow cat="Contrast correction" unm="Adaptive window" def=1
uniform bool cAdaptiveWindow;

//! bool cRenormalize cat="Contrast correction" unm="Renormalize" def=1
uniform bool cRenormalize;

void mainImage(out vec4 O, in vec2 U)
{
    vec4 c = texture(iChannel0, U / iResolution.xy);
    vec4 fc = textureLod(iChannel0, U / iResolution.xy, (cAdaptiveWindow ? pow(c.g, 2.0) : 1.0) * cFilterLod);

    if (dGrid) {
        O = c;
        O.g = 0.;
        return;
    }

    if (cRenormalize) {
        O = vec4(2. * (c.rrr - .5) / sqrt(fc.b), c.a);
        O.rgb = .5 * O.rgb + .5;
    } else {
        O = c.rrra;
    }

    // Apply lighting component
    O.rgb = lighting(U, O.rgb);
}
