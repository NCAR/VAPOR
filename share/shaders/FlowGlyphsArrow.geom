// This shader generates a cone at every vertex v_n pointing in the direction of 
// the average normal between v_n-1 to v_n and v_n to v_n+1. The cone is generated
// as a triangle strip, generated from the bottom circle and top vertex. This
// geometry is generated at the origin and transformed with a simple affine tranformation
// using basis vectors.
// 
// EmitWorld(vec3 v) projects v and emits it.
// CylinderVert(v, ...) is a helper function that generates a point on a circle
// given the parameters and passes it to EmitWorld.

#version 330 core

#include FlowInclude.geom

#define REFINEMENT 7

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 32) out; // (7 * 2 + 2) * 2


in  float gValue[];
in  float clip[];
out float fValue;
out vec3 fNormal;


uniform mat4 P;
uniform mat4 MV;
uniform vec3 scales;
uniform float radius = 0.05;
uniform int glyphStride = 1;
uniform bool showOnlyLeadingSample = false;


void EmitWorld(const vec3 v);
void CylinderVert(const vec3 v, const vec3 d, const float value);


void main()
{
	// Discard skipped samples
	if (showOnlyLeadingSample) {
		if (gl_PrimitiveIDIn < nVertices-4)
			return;
	} else {
		if (gl_PrimitiveIDIn % glyphStride != 0)
			return;
	}

	// Need to manually clip when using geometry shader
    if (clip[1] < 0.0 || clip[2] < 0.0)
        return;

    vec3 V[4];
    for (int i = 0; i < 4; i++)
        V[i] = gl_in[i].gl_Position.xyz;

    vec3 VN[3];
    for (int i = 0; i < 3; i++)
        VN[i] = normalize(V[i+1] - V[i]);

    vec3 N[2];
    vec3 up = vec3(0, 0, 1);
    bool parallelToUpChanged = false;
    for (int i = 0; i < 2; i++) {
        N[i] = normalize((VN[i] + VN[i+1])/2);
        if (1 - dot(N[i], up) < FLT_EPSILON)
            parallelToUpChanged = true;
    }

	// Basis vectors are generated against an arbritray up which needs to be different from N
    if (parallelToUpChanged)
        up = vec3(1, 0, 0);

	// Generate basis vectors
    vec3 XH[2];
    vec3 YH[2];
    for (int i = 0; i < 2; i++) {
        XH[i] = normalize(cross(N[i], up));
        YH[i] = normalize(cross(XH[i], N[i]));
    }

    fValue = gValue[1];
    vec3 top = V[1] + N[0] * radius * 1.618 / scales;

	// Plot geometry sides
    for (int i = 0; i < REFINEMENT + 1; i++) {
        float t = float(i)/float(REFINEMENT) * 2.f * PI;
        vec3 d;
        d = XH[0]*cos(t) + YH[0]*sin(t);
        vec3 bottom = V[1] + d * radius / scales;

        // fNormal = normalize((d + N[0])/2.0f);
        fNormal = normalize(cross(top-bottom, cross(d, N[0])));

        EmitWorld(bottom);
        EmitWorld(top);
    }
    EndPrimitive();

    fNormal = -N[0];

    // Plot geometry bottom
    for (int i = 0; i < REFINEMENT + 1; i++) {
        float t = float(i)/float(REFINEMENT) * 2.f * PI;
        vec3 d;
        d = XH[0]*cos(t) + YH[0]*sin(t);

        EmitWorld(V[1]);
        EmitWorld(V[1] + d * radius / scales);
    }
    EndPrimitive();
} 


void EmitWorld(const vec3 v)
{
    gl_Position = P * MV * vec4(v, 1.0f);
    EmitVertex();
}


void CylinderVert(const vec3 v, const vec3 d, const float value)
{
    fNormal = d;
    EmitWorld(v + d * radius);
}
