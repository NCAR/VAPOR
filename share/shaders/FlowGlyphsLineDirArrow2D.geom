// This behaves the same as FlowGlyphsArrow2D.geom except the geometry is clipped against the line

#version 330 core

#include FlowInclude.geom

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 6) out;

in  float gValue[];
in  float clip[];
out float fValue;
out float fFade;
out float t;

uniform float radius = 0.05f;
uniform int glyphStride = 1;
uniform float border = 0.f;
uniform float aspect;


void main()
{
    if (gl_PrimitiveIDIn % glyphStride != 0)
        return;

    if (clip[1] < 0.0 || clip[2] < 0.0)
        return;

    vec2 vS[4];
    for (int i = 0; i < 4; i++)
        vS[i] = gl_in[i].gl_Position.xy / gl_in[i].gl_Position.w;

    vec2 vN[3];
    for (int i = 0; i < 3; i++)
        vN[i] = normalize(vS[i+1] - vS[i]);

    vec2 n[2];
    vec2 p[2];
    vec4 o[2];
    for (int i = 0; i < 2; i++) {
        n[i] = normalize((vN[i] + vN[i+1])/2);
        p[i] = vec2(-n[i].y, n[i].x);
        p[i].x /= aspect;
        o[i] = vec4(p[i], 0.0f, 0.0f) * (radius*2 + border);
    }


#define v0 gl_in[1].gl_Position
#define v1 gl_in[2].gl_Position

    float s = 1.9;
    vec2 fade = CalculateFade();

    vec4 top = v0 + vec4(n[0],0,0) * radius * 1.618 * 2;

    float tTop = length(top.xyz-v0.xyz)/length(v1.xyz-v0.xyz);
    float fTop = mix(fade[0], fade[1], tTop);
    float vTop = mix(gValue[1], gValue[2], tTop);

    gl_Position = v0 - o[0]*s; fValue = gValue[1]; fFade = fade[0]; t = -1; EmitVertex();
    gl_Position = v1 - o[1]; fFade = fade[1]; fValue = gValue[2];        t = -1; EmitVertex();
    gl_Position = v0 - o[0]; fValue = gValue[1]; fFade = fade[0]; t =  1; EmitVertex();

    EndPrimitive();

    gl_Position = v0 + o[0]*s; fValue = gValue[1]; fFade = fade[0]; t = -1; EmitVertex();
    gl_Position = v0 + o[0]; fValue = gValue[1]; fFade = fade[0]; t =  1; EmitVertex();
    gl_Position = v1 + o[1]; fFade = fade[1]; fValue = gValue[2];        t = -1; EmitVertex();

    EndPrimitive();
} 
