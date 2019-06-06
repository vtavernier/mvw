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
    O = texture(colorOutput, U / iResolution.xy);
    vec4 P = texture(phasorField, U / iResolution.xy); // phasor field

    O.rg = 2. * O.rg - 1.;

    if (dPhase) {
        float phase = mod(atan(P.g, P.r), 2. * M_PI) / (2. * M_PI);
        O.rgb = length(P.rg) * cm_matlab_hsv(phase).rgb;
    } else {
        // Show only noise value
        O.rgb = .5 + .5 * vec3(O.g / length(O.rg));
    }

    O.rgb = lighting(U, O.rgb);
}
