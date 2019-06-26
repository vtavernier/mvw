[% PROCESS pp/color.glsl %]
[% PROCESS pp/params.glsl %]
[% PROCESS pp/lighting.glsl %]
[% PROCESS core/math.glsl %]
[% PROCESS MATLAB_hsv.frag | replace('colormap', 'cm_matlab_hsv') %]

//! phasorField binding=phasorField

//! bool dPhase def=false cat="Phasor Debug" unm="Show phase field"
uniform bool dPhase;
//! bool dNoiseOverlay def=false cat="Phasor Debug" unm="Overlay noise"
uniform bool dNoiseOverlay;
//! bool dComplexOutput def=false cat="Phasor Debug" unm="Complex output"
uniform bool dComplexOutput;

void mainImage(out vec4 O, in vec2 U)
{
    float a = .2;
    vec4 C = O = texture(colorOutput, U / iResolution.xy);
    vec4 P = texture(phasorField, U / iResolution.xy); // phasor field

    if (dGrid) {
        return;
    }

    O.rg = 2. * O.rg - 1.;

    // Noise value
    float norm = length(O.rg);
    C.rgb = .5 + .5 * vec3(O.g / norm);

    if (dPhase) {
        float angle = atan(P.g, P.r);
        float phase = mod(angle, 2. * M_PI) / (2. * M_PI);

        if (dComplexOutput) {
            O.r = cos(angle);
            O.g = sin(angle);
            O.b = 0.;

            O.rg *= norm;
            O.rg = O.rg / 2. + .5;
        } else {
            O.rgb = cm_matlab_hsv(phase).rgb;
        }

        if (dNoiseOverlay) {
            O.rgb = C.rgb * a + (1. - a) * (.45 * length(P.rg) + .5) * O.rgb;
        }
    } else {
        // Show only noise value
        O.rgb = C.rgb;
    }

    O.rgb = lighting(U, O.rgb);
}
