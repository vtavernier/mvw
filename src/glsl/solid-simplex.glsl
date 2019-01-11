#pragma glslify: snoise3 = require(glsl-noise/simplex/3d)

void mainImage(out vec4 O, in vec2 U)
{
    O = vec4(vec3(.5 + .5 * snoise3(5. * vPosition)), 1.0);
}
