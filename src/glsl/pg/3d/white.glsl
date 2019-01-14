struct pg_state {
    prng_state state;
    int splats;
};

void pg_seed(inout pg_state this_, ivec3 nc, ivec3 tile_count, int random_seed, int expected_splats)
{
    uint seed = uint(nc.z * tile_count.y * tile_count.z + nc.x * tile_count.x + nc.y + 1 + random_seed);
    prng_seed(this_.state, seed);

    // The expected number of points for given splats is splats
    this_.splats = expected_splats;

    m4_ifelse(WHITE_POISSON, 1, `this_.splats = prng_poisson(this_.state, expected_splats);')
}

int pg_splats(in pg_state this_)
{
    return this_.splats;
}

void pg_point4(inout pg_state this_, out vec4 pt)
{
    // Generate random position
    pt.xy = 2. * prng_rand2(this_.state) - 1.;
    pt.zw = 2. * prng_rand2(this_.state) - 1.;
}
