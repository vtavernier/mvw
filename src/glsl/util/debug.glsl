void debugCell(inout vec4 O, vec3 P, ivec3 ccell, ivec3 cell, vec3 center, vec3 w0) {
    pg_state pstate;

    ivec3 count = ivec3(ceil((bboxMax - bboxMin) / _TILE_SIZE));
    pg_seed(pstate, cell, count, 438023, gSplats);

    vec4 pt;
    pg_point4(pstate, pt);

    O = vec4(vec3(.75), 1.) * O + .25 * vec4(pt.rgb * .5 + .5, 1.);
}

void debugRot(inout vec4 O, vec3 P, ivec3 ccell, ivec3 cell, vec3 center, vec3 w0) {
    float phase = 2. * M_PI * gF0 * dot((P - center), W0VEC(gW0));
    O = vec4(cos(phase), sin(phase), -1., 1.);
}

[% grid3d_define(name="debugGrid",cell="debugCell") %]
[% grid3d_define(name="debugRotGrid",cell="debugRot") %]
