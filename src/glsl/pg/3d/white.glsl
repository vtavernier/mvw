struct pg_state {
    prng_state state;
    int splats;
};

void pg_seed(inout pg_state this_, ivec3 nc, ivec3 tile_count, int random_seed, int expected_splats)
{
    uint seed = uint(nc.z * tile_count.x * tile_count.y + nc.y * tile_count.x + nc.x + random_seed);
    prng_seed(this_.state, seed);

    // The expected number of points for given splats is splats
    this_.splats = expected_splats;
[% IF white_poisson %]

    this_.splats = prng_poisson(this_.state, expected_splats);
[% END %]
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
