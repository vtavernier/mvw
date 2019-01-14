float h(vec2 x, float K, float F0, vec2 w0, vec2 tile_size) {
    float r = length(x);

    return K * exp(-M_PI * r * r) * cos(2. * M_PI * F0 * dot(x / tile_size, w0));
}
