#version 440

precision highp float;
precision highp int;
precision highp sampler2D;

uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProj;

uniform bool bWireframe;

uniform vec3 bboxMax;
uniform vec3 bboxMin;

uniform bool dQuad;

uniform vec3 iResolution;

// Vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

// Texture coord for fragment
out vec2 vtexCoord;
out vec3 vNormal;
out vec3 vPosition;

void main() {
    vtexCoord = texCoord;
    vNormal = normal;
    vPosition = position;

    if (dQuad) {
        // Fix quad aspect ratio
        float ratio = iResolution.x / iResolution.y;
        vPosition.x *= ratio;
        vtexCoord.x *= ratio;

        gl_Position = vec4(position, 1.0);
    } else {
        gl_Position = mProj * mView * mModel * vec4(position, 1.0);
    }
}
