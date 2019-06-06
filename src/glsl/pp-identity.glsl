[% PROCESS pp/color.glsl %]
[% PROCESS pp/params.glsl %]
[% PROCESS pp/lighting.glsl %]

void mainImage(out vec4 O, in vec2 U)
{
    O = texture(colorOutput, U / iResolution.xy);

    // Show only noise value
    O.rgb = O.rrr;

    O.rgb = lighting(U, O.rgb);
}
