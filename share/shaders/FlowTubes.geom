#version 330 core

#include FlowInclude.geom

#define REFINEMENT 7

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 16) out; // 7 * 2 + 2


in  float gValue[];
in  float clip[];
out float fValue;
out float fFade;
out vec3 fNormal;


uniform mat4 P;
uniform mat4 MV;
uniform vec3 scales;
uniform float radius = 0.05;

void EmitWorld(const vec3 v);
void CylinderVert(const vec3 v, const vec3 d, const float fade, const float value);


void main()
{
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

    vec2 fade = CalculateFade();

    for (int i = 0; i < REFINEMENT + 1; i++) {
        float t = float(i)/float(REFINEMENT) * 2.f * PI;
        vec3 D[2];
        for (int j = 0; j < 2; j++)
            D[j] = XH[j]*cos(t) + YH[j]*sin(t);

        CylinderVert(V[1], D[0], fade[0],  gValue[1]);
        CylinderVert(V[2], D[1], fade[1],  gValue[2]);
    }
    EndPrimitive();
} 


void EmitWorld(const vec3 v)
{
    gl_Position = P * MV * vec4(v, 1.0f);
    EmitVertex();
}


void CylinderVert(const vec3 v, const vec3 d, const float fade, const float value)
{
    fNormal = d;
    fValue = value;
    fFade = fade;
    vec3 offset = d*radius;
    offset /= scales;
    EmitWorld(v + offset);
}

