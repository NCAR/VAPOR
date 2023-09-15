// This behaves the same as FlowGlyphsArrow.geom except it clips the geometry against the tube.

#version 330 core

#include FlowInclude.geom

#define REFINEMENT 7

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 32) out; // (7 * 2 + 2) * 2


in  float gValue[];
in  float clip[];
out float fValue;
out float fFade;
out vec3 fNormal;


uniform mat4 P;
uniform mat4 MV;
uniform vec3 scales;
uniform float radius = 0.05;
uniform int glyphStride = 1;


void EmitWorld(const vec3 v);
void CylinderVert(const vec3 v, const vec3 d, const float value);


void main()
{
    if (gl_PrimitiveIDIn % glyphStride != 0)
        return;

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

    if (parallelToUpChanged)
        up = vec3(1, 0, 0);

    vec3 XH[2];
    vec3 YH[2];
    for (int i = 0; i < 2; i++) {
        XH[i] = normalize(cross(N[i], up));
        YH[i] = normalize(cross(XH[i], N[i]));
    }

    fValue = gValue[1];
    vec3 top = V[1] + N[0] * radius * 1.618;

    float s = 1.7;
    vec2 fade = CalculateFade();

    for (int i = 0; i < REFINEMENT + 1; i++) {
        float t = float(i)/float(REFINEMENT) * 2.f * PI;
        vec3 d;
        d = XH[0]*cos(t) + YH[0]*sin(t);

        vec3 bottom = V[1] + d * radius * s / scales;
        vec3 top = mix(V[1], V[2], 1.0/1.218) + d * radius / scales;

        // fNormal = normalize((d + N[0])/2.0f);
        fNormal = normalize(cross(top-bottom, cross(d, N[0])));
        fFade = fade[0];
        EmitWorld(bottom);

        fFade = mix(fade[0], fade[1], 1.0/1.218);
        EmitWorld(top);
    }
    EndPrimitive();

    fNormal = -N[0];
    fFade = fade[0];

    // Bottom
    for (int i = 0; i < REFINEMENT + 1; i++) {
        float t = float(i)/float(REFINEMENT) * 2.f * PI;
        vec3 d;
        d = XH[0]*cos(t) + YH[0]*sin(t);

        EmitWorld(V[1]);
        EmitWorld(V[1] + d * radius * s / scales);
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
