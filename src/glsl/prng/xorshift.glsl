// xorshift pseudo-random generator
struct prng_state { uvec2 s_; };

uvec2 next(inout uvec2 s) {
    uint t = s.x ^ (s.x << 23);
    s.x = s.y;
    s.y = (s.y ^ (s.y >> 24)) ^ (t ^ (t >> 3));
    return s;
}

void prng_seed(inout prng_state this_, uint seed) {
    this_.s_ = hash2(seed << 1 | uvec2(0, 1));
}

float prng_rand1(inout prng_state this_) {
    return float(next(this_.s_).y) / float(4294967295u);
}

vec2 prng_rand2(inout prng_state this_) {
    return vec2(next(this_.s_)) / float(4294967295u);
}
