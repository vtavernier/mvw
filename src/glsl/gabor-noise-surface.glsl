#pragma glslify: lcg = require(./prng/lcg)
#pragma glslify: white = require(./pg/2d/white,prng_state=prng_state)
#pragma glslify: kernel = require(./gabor/kernel)

#define W0VEC(w0) vec2(cos(w0), sin(w0))

void mainImage(out vec4 O, in vec2 U)
{
    O = vec4(vec3(gn_cos(U, 1.0, 1.0, W0VEC(3.14 / 4), vec2(5.)), 1.0);
}
