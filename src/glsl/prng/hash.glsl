// hash pseudo-random generator
struct prng_state { uint x_; uint c_; };

void prng_seed(inout prng_state this_, uint seed) {
    this_.x_ = hash(seed);
    this_.c_ = 0;
}

float prng_rand1(inout prng_state this_) {
    return hash(this_.x_ ^ ((this_.c_ += 1) << 8)) / float(4294967295u);
}

vec2 prng_rand2(inout prng_state this_) {
    return vec2(prng_rand1(this_), prng_rand1(this_));
}

#pragma glslify: export(prng_state)
#pragma glslify: export(prng_seed)
#pragma glslify: export(prng_rand1)
#pragma glslify: export(prng_rand2)
