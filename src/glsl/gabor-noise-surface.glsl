m4_include(core/math.glsl)
m4_include(prng/none.glsl)
m4_include(prng/poisson.glsl)
m4_define(WHITE_POISSON,1)
m4_include(pg/2d/white.glsl)
m4_include(gabor/kernel.glsl)

#define W0VEC(w0) vec2(cos(w0), sin(w0))

void mainImage(out vec4 O, in vec2 U)
{
    O = vec4(vec3(h(U, 1.0, 1.0, W0VEC(3.14 / 4), vec2(5.))), 1.0);
}
