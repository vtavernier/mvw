m4_define(GABOR_KERNEL,`float $1($2 x, float K, float F0, $2 w0, $2 tile_size) {
    float r = length(x);

    return K * exp(-M_PI * r * r) * cos(2. * M_PI * F0 * dot(x / tile_size, w0));
}')

GABOR_KERNEL(h2,vec2)
GABOR_KERNEL(h3,vec3)
