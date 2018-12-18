void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord.xy;
    fragColor = vec4(uv.x+0.5*sin(iTime),uv.y,0.5+0.5*sin(iTime),1.0);

    vec3 direction = normalize(vec3(1., 0., 1.));
    vec3 normal = normalize(mat3(mModel) * vNormal);
    float diffuse = max(dot(normal, direction), 0.0);

    fragColor *= max(diffuse, 0.);
}
