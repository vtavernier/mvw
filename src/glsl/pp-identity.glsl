void mainImage(out vec4 O, in vec2 U)
{
    vec4 c = texture(iChannel0, U / iResolution.xy);
    O = vec4(c.rgb, 1.);
}
