[% PROCESS pp/params.glsl %]
[% PROCESS pp/lighting.glsl %]
[% PROCESS core/math.glsl %]
[% PROCESS MATLAB_hsv.frag | replace('colormap', 'cm_matlab_hsv') %]

//! bool dPhase def=false cat="Debug" unm="Show phase field"
uniform bool dPhase;

void mainImage(out vec4 O, in vec2 U)
{
    O = texture(iChannel0, U / iResolution.xy);

    O.rg = 2. * O.rg - 1.;

    if (dPhase) {
        O.b = atan(O.g, O.r);
        O.rgb = cm_matlab_hsv(mod(O.b, 2. * M_PI) / (2. * M_PI)).rgb;
    } else {
        // Show only noise value
        O.rgb = .5 + .5 * vec3(O.g / length(O.rg));
    }

    O.rgb = lighting(U, O.rgb);
}
