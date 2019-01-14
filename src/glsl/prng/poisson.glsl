// Poisson number generator
// Requires an imported prng_{state,rand1,rand2} uniform generator
// TODO: fix usage with the none generator, should return int(mean + .5)

int prng_poisson(inout prng_state this_, float mean) {
    m4_ifdef(PRNG_NONE, `return int(mean + .5)')
    int em = 0;

    if (mean < 45.)
    {
        // Knuth
        float g = exp(-mean);
        float t = prng_rand1(this_);
        while (t > g) {
            ++em;
            t *= prng_rand1(this_);
        }
    }
    else
    {
        // Gaussian approximation
        vec2 u = prng_rand2(this_);
        float v = sqrt(-2. * log(u.x)) * cos(2. * M_PI * u.y);
        em = int((v * sqrt(mean)) + mean + .5);
    }

    return em;
}
