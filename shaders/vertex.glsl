#version 440

precision highp float;
precision highp int;
precision highp sampler2D;

#pragma shadertoy part geometry:uniforms

// Vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

// Texture coord for fragment
out vec2 vtexCoord;
out vec3 vNormal;
out vec3 vWorldNormal;
out vec3 vPosition;

void main() {
    vtexCoord = texCoord;
    vNormal = normal;
    vWorldNormal = (mModel * vec4(normal, 0.0)).xyz;
    vPosition = position;

    gl_Position = mProj * mView * mModel * vec4(position, 1.0);
}
