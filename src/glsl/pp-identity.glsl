uniform bool dQuad;

void mainImage(out vec4 O, in vec2 U)
{
    O = texture(iChannel0, U / iResolution.xy);

    // Show only noise value
    O.rgb = O.rrr;

    if (!dQuad) O.rgb *= texture(iChannel1, U / iResolution.xy).rgb;
}
