struct pg_state {
    int seed;
    int splats;
    int current;
};

void pg_seed(inout pg_state this_, ivec3 nc, ivec3 tile_count, int random_seed, int expected_splats)
{
    // Pick row for the current cell
    this_.seed = int(nc.z * tile_count.x * tile_count.y + nc.y * tile_count.x + nc.x);

    // No Poisson when based on data
    this_.splats = expected_splats;

    // Column offset
    this_.current = 0;
}

int pg_splats(in pg_state this_)
{
    return this_.splats;
}

void pg_point4(inout pg_state this_, out vec4 pt)
{
    ivec2 loc = ivec2(this_.current, this_.seed);
    pt = 2. * texelFetch(pointData, loc, 0) - 1.;
    this_.current += 1;
}
