void mainImage(out vec4 O, in vec2 U)
{
    // Initial return value
    O = vec4(.5, .5, .5, 1.);

    // Shading for shape
    vec3 direction = normalize(vec3(1., 0., 1.));
    vec3 normal = normalize(mat3(mModel) * vNormal);
    float diffuse = max(dot(normal, direction), 0.0);

    O.rgb *= (.2 + .8 * max(diffuse, 0.));
}
