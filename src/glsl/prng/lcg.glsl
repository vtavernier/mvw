// LCG pseudo-random generator
struct prng_state { uint x_; };

void prng_seed(inout prng_state this_, uint seed) {
    this_.x_ = hash(seed);
}

float prng_rand1(inout prng_state this_) {
    return (this_.x_ *= 3039177861u) / float(4294967295u);
}

vec2 prng_rand2(inout prng_state this_) {
    return vec2(prng_rand1(this_), prng_rand1(this_));
}
