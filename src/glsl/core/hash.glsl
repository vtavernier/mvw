// Hashing function
uvec4 hash4(uvec4 x);

uint hash(uint x) {
    return hash4(uvec4(x)).x;
}

uvec2 hash2(uvec2 x) {
    return hash4(uvec4(x.xyxy)).xy;
}

uvec4 hash4(uvec4 x) {
    // Wang hash
    x = (x ^ 61u) ^ (x >> 16u);
    x *= 9u;
    x = x ^ (x >> 4);
    x *= 0x27d4eb2du;
    x = x ^ (x >> 15);
    return x;
}
