//! bool dLighting cat="Rendering" unm="Lighting" def=1
uniform bool dLighting;

vec3 lighting(vec2 U, vec3 I) {
    if (!dQuad && dLighting)
        return I * texture(iChannel1, U / iResolution.xy).rgb;
    return I;
}
