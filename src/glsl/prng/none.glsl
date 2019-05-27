// dummy "pseudo-random" generator
struct prng_state { uint x_; };

void prng_seed(inout prng_state this_, uint seed) {
}

float prng_rand1(inout prng_state this_) {
    return 0.;
}

vec2 prng_rand2(inout prng_state this_) {
    return vec2(0.);
}

[% SET prng_none = 1 %]
