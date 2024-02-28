// This shader generates tubes between every pair of vertices in a line. 
// The tube is generated as a triangle strip. This geometry is generated at the origin
// and transformed with a simple affine transformation using basis vectors.
// 
// EmitWorld(vec3 v) projects v and emits it.
// CylinderVert(v, ...) is a helper function that generates a point on a circle
// given the parameters and passes it to EmitWorld.

#version 330 core

#include FlowInclude.geom

#define REFINEMENT 7

layout (points) in;
layout (triangle_strip, max_vertices = 16) out; // 7 * 2 + 2
//layout (line_strip, max_vertices = 64) out; // 7 * 2 + 2
//layout (points, max_vertices = 64) out; // 7 * 2 + 2


in  vec3 gNorm[];
in  float gValue[];
out float fValue;
out vec3 fNormal;


#ifdef DYNAMIC_RADIUS
in float gRadius[];
#endif


uniform mat4 P;
uniform mat4 MV;
uniform vec3 scales;
uniform float radius = 0.05;
uniform float dirScale = 0.05;

void EmitWorld(const vec3 v);
void CylinderVert(const vec3 v, const vec3 d, const float value, const float radius);


void main()
{gs
    //fValue = gValue[0];
    //EmitWorld(gl_in[0].gl_Position.xyz);
    //EmitWorld(gl_in[0].gl_Position.xyz + gNorm[0] * dirScale);
    //EndPrimitive();
    //return;

    vec3 V[2];
    V[0] = gl_in[0].gl_Position.xyz;

    vec3 N = gNorm[0];
    V[1] = V[0] + N * dirScale;

    vec3 up = vec3(0, 0, 1);
    bool parallelToUpChanged = false;
    if (1 - dot(N, up) < FLT_EPSILON)
        parallelToUpChanged = true;

    if (parallelToUpChanged)
        up = vec3(1, 0, 0);

    vec3 XH;
    vec3 YH;
    XH = normalize(cross(N, up));
    YH = normalize(cross(XH, N));

    #ifdef DYNAMIC_RADIUS
        float finalRadius = radius * gRadius[0];
    #else
        float finalRadius = radius;
    #endif

    vec2 fade = CalculateFade();

    //fNormal = vec3(1,0,0); fValue = 1; EmitWorld(gl_in[0].gl_Position.xyz); EmitWorld(gl_in[1].gl_Position.xyz); EndPrimitive(); return;

    for (int i = 0; i < REFINEMENT + 1; i++) {
        float t = float(i)/float(REFINEMENT) * 2.f * PI;
        vec3 D;
        D = XH*cos(t) + YH*sin(t);

        CylinderVert(V[0], D, gValue[0], finalRadius);
        CylinderVert(V[1], D, gValue[0], finalRadius);
    }
    EndPrimitive();
} 


void EmitWorld(const vec3 v)
{
    gl_Position = P * MV * vec4(v, 1.0f);
    EmitVertex();
}


void CylinderVert(const vec3 v, const vec3 d, const float value, const float radius)
{
    fNormal = d;
    fValue = value;
    vec3 offset = d*radius;
    offset /= scales;
    EmitWorld(v + offset);
}

