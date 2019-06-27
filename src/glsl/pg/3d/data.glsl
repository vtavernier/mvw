struct pg_state {
    int seed;
    int splats;
    int current;
    ivec3 tile_count;
};

void pg_seed(inout pg_state this_, ivec3 nc, ivec3 tile_count, int random_seed, int expected_splats)
{
    // Wrap-around
    if (nc.x < 0) nc.x = tile_count.x + nc.x;
    if (nc.y < 0) nc.y = tile_count.y + nc.y;
    if (nc.z < 0) nc.z = tile_count.z + nc.z;
    nc = nc % tile_count;

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
