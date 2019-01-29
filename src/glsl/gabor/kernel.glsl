float eb(float r) {
    float e;

    // Truncate kernel so it fits in a cell
    if (r > 1.) {
        e = 0.;
    } else {
        m4_ifelse(KKAISER_BESSEL, 1, `
        e = 0.402 + 0.498 * cos(2. * M_PI * (r / 8.))
                  + 0.099 * cos(4. * M_PI * (r / 8.))
                  + cos(6. * M_PI * (r / 8.));
        ', KTRUNC, 1, `
        e = (exp(-M_PI * r * r) - exp(-M_PI)) / (1. - exp(-M_PI));
        ', `
        e = exp(-M_PI * r * r);
        ')
    }

    return e;
}

m4_define(GABOR_KERNEL,`float $1($2 x, float K, float F0, $2 w0, $2 tile_size, float phase) {
    float r = length(x);

    return K * eb(r) * sin(2. * M_PI * F0 * dot(x / tile_size, w0) + phase);
}')

GABOR_KERNEL(h2,vec2)
GABOR_KERNEL(h3,vec3)

m4_define(DISK_KERNEL,`float $1($2 x, float K, float F0, $2 w0, $2 tile_size, float phase) {
    float r = length(x);

    return K * smoothstep(.1, 0., r / 2.) * gSplats;
}')

DISK_KERNEL(dk2,vec2)
DISK_KERNEL(dk3,vec3)

