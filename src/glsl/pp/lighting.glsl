//! bool dLighting cat="Rendering" unm="Lighting" def=1
uniform bool dLighting;

//! fragLighting binding=fragLighting

vec3 lighting(vec2 U, vec3 I) {
    if (!dQuad && dLighting)
        return I * texture(fragLighting, U / iResolution.xy).rgb;
    return I;
}
