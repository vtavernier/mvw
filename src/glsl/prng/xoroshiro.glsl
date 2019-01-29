// xoroshiro pseudo-random generator
struct prng_state { uvec4 x_; };

uint rotl(uint x, int k) {
    return (x << k) | (x >> (32 - k));
}

// Generates 1 uniform integers, updates the state given as input
uint next(inout uvec2 s) {
    uint s0 = s.x;
    uint s1 = s.y;
    uint rs = rotl(s0 * 0x9E3779BBu, 5) * 5u;

    s1 ^= s0;
    s.x = rotl(s0, 26) ^ s1 ^ (s1 << 9);
    s.y = rotl(s1, 13);

    return rs;
}

uvec2 rotl2(uvec2 x, int k) {
    return (x << k) | (x >> (32 - k));
}

// Generates 2 uniform integers, updates the state given as input
uvec2 next2(inout uvec4 s) {
    uvec2 s0 = s.xz;
    uvec2 s1 = s.yw;
    uvec2 rs = rotl2(s0 * 0x9E3779BBu, 5) * 5u;

    s1 ^= s0;
    s.xz = rotl2(s0, 26) ^ s1 ^ (s1 << 9);
    s.yw = rotl2(s1, 13);

    return rs;
}
void prng_seed(inout prng_state this_, uint seed) {
    this_.x_ = hash4(seed << 2 | uvec4(0, 1, 2, 3));
}

// Converts an unsigned int to a float in [0,1]
float tofloat(uint u) {
    //Slower, but generates all dyadic rationals of the form k / 2^-24 equally
    //return float(u >> 8) * (1. / float(1u << 24));

    //Faster, but only generates all dyadic rationals of the form k / 2^-23 equally
    return uintBitsToFloat(0x7Fu << 23 | u >> 9) - 1.;
}

// Converts a vector of unsigned ints to floats in [0,1]
vec2 tofloat2(uvec2 u) {
    //Slower, but generates all dyadic rationals of the form k / 2^-24 equally
    //return vec2(u >> 8) * (1. / float(1u << 24));

    //Faster, but only generates all dyadic rationals of the form k / 2^-23 equally
    return uintBitsToFloat(0x7Fu << 23 | u >> 9) - 1.;
}

float prng_rand1(inout prng_state this_) {
    return tofloat(next(this_.x_.xy));
}

vec2 prng_rand2(inout prng_state this_) {
    return tofloat2(next2(this_.x_));
}
