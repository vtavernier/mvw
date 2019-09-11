float eb(float r) {
    float e;

    // Truncate kernel so it fits in a cell
    if (r > 1.) {
        e = 0.;
    } else {
[% IF kernel == "kaiser" %]
        e = 0.402 + 0.498 * cos(2. * M_PI * (r / 8.))
                  + 0.099 * cos(4. * M_PI * (r / 8.))
                  + cos(6. * M_PI * (r / 8.));
[% ELSIF kernel == "trunc" %]
        e = (exp(-M_PI * r * r) - exp(-M_PI)) / (1. - exp(-M_PI));
[% ELSE %]
        e = exp(-M_PI * r * r);
[% END %]
    }

    return e;
}

[% MACRO cgabor_kernel BLOCK %]
vec4 $name($postype x, $postype xi, float K, float F0, $postype w0, $postype w1, $postype tile_size, float phase) {
    float r = length((x - xi) / tile_size);
    vec2 p1 = 2. * M_PI * F0 * vec2(dot(x - xi, w0), dot(-xi, w0)) + phase;
    vec2 p2 = 2. * M_PI * F0 * vec2(dot(x - xi, w1), dot(-xi, w1)) + phase + M_PI / 2.;

    return K * eb(r) * vec4(cos(p1.x), sin(p1.x), cos(p2.x), sin(p2.x));
}
[% END %]

[% cgabor_kernel(name="ch2",postype="vec2") %]
[% cgabor_kernel(name="ch3",postype="vec3") %]

[% MACRO gabor_kernel BLOCK %]
float $name($postype x, float K, float F0, $postype w0, $postype tile_size, float phase) {
    float r = length(x);

    return K * eb(r) * sin(2. * M_PI * F0 * dot(x * tile_size, w0) + phase);
}
[% END %]

[% gabor_kernel(name="h2",postype="vec2") %]
[% gabor_kernel(name="h3",postype="vec3") %]

[% MACRO disk_kernel BLOCK %]
float $name($postype x, float K, float F0, $postype w0, $postype tile_size, float phase) {
    float r = length(x);

    return K * smoothstep(.1, 0., r / 2.) * gSplats;
}
[% END %]

[% disk_kernel(name="dk2",postype="vec2") %]
[% disk_kernel(name="dk3",postype="vec3") %]
