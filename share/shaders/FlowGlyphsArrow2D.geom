#version 330 core

#include FlowInclude.geom

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 3) out;

in  float gValue[];
in  float clip[];
out float fValue;
out float t;

uniform float radius = 0.05f;
uniform int glyphStride = 1;
uniform bool showOnlyLeadingSample = false;
uniform float border = 0.f;
uniform float aspect;


void main()
{
	if (showOnlyLeadingSample) {
		if (gl_PrimitiveIDIn < nVertices-4)
			return;
	} else {
		if (gl_PrimitiveIDIn % glyphStride != 0)
			return;
	}

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

    vec4 top = v0 + vec4(n[0],0,0) * radius * 1.618 * 2;
    fValue = gValue[1];

    gl_Position = v0 - o[0]; t = -1; EmitVertex();
    gl_Position = top;        t = -1; EmitVertex();
    gl_Position = v0 + o[0]; t =  1; EmitVertex();

    EndPrimitive();
} 
