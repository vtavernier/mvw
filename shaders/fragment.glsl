#pragma shadertoy part glsl:header begin
#version 300 es
#pragma shadertoy part end

precision highp float;
precision highp int;
precision highp sampler2D;

in vec2 vtexCoord;
in vec3 vNormal;
in vec3 vPosition;

layout(location = 0) out vec4 fragColor;

#pragma shadertoy part *:defines

// Standard shadertoy uniforms
uniform vec3 iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int iFrame;
uniform float iFrameRate;
uniform float iChannelTime[4];
uniform vec3 iChannelResolution[4];
uniform vec4 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;
uniform vec4 iDate;
uniform float iSampleRate;

// Geometry viewer uniforms
uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProj;

uniform bool bWireframe;

uniform vec3 bboxMax;
uniform vec3 bboxMin;

uniform bool dQuad;

#pragma shadertoy part buffer:inputs
#pragma shadertoy part buffer:sources

void main(void) {
    if (bWireframe) {
        fragColor = vec4(1.);
    } else {
        fragColor = vec4(0., 0., 0., 1.);
        mainImage(fragColor, vtexCoord.xy);
    }
}
