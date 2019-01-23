void mainImage(out vec4 O, in vec2 U)
{
    vec4 c = texture(iChannel0, U / iResolution.xy);
    vec4 fc = textureLod(iChannel0, U / iResolution.xy, cFilterLod);

    O = vec4(2. * (c.rgb - .5) / sqrt(fc.a),
             1.);

    O.rgb = .5 * O.rgb + .5;
}
