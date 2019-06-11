[% PROCESS pp/color.glsl %]
[% PROCESS pp/params.glsl %]
[% PROCESS pp/lighting.glsl %]
[% PROCESS core/math.glsl %]
[% PROCESS MATLAB_hsv.frag | replace('colormap', 'cm_matlab_hsv') %]

//! phasorField binding=phasorField

//! bool dPhase def=false cat="Debug" unm="Show phase field"
uniform bool dPhase;

void mainImage(out vec4 O, in vec2 U)
{
    float a = .2;
    vec4 C = O = texture(colorOutput, U / iResolution.xy);
    vec4 P = texture(phasorField, U / iResolution.xy); // phasor field

    O.rg = 2. * O.rg - 1.;

    // Noise value
    C.rgb = .5 + .5 * vec3(O.g / length(O.rg));

    if (dPhase) {
        float phase = mod(atan(P.g, P.r), 2. * M_PI) / (2. * M_PI);
        O.rgb = C.rgb * a + (1. - a) * (.45 * length(P.rg) + .5) * cm_matlab_hsv(phase).rgb;
    } else {
        // Show only noise value
        O.rgb = C.rgb;
    }

    O.rgb = lighting(U, O.rgb);
}
