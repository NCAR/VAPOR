#version 330 core

#include FlowInclude.geom

#define REFINEMENT 16

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 34) out;

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

    fValue = gValue[1];

    vec4 o = gl_in[1].gl_Position;

    for (int i = 0; i < REFINEMENT+1; i++) {
        float theta = float(i)/float(REFINEMENT) * 2.f * PI;
        vec4 d = vec4(cos(theta), sin(theta), 0.0, 0.0);
        d.x /= aspect;

        t = 0;
        gl_Position = o;
        EmitVertex();

        t = 1;
        gl_Position = o+d*radius*2;
        EmitVertex();
    }
    EndPrimitive();

} 
