#version 330 core

#include FlowInclude.geom

#define REFINEMENT 10

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 256) out;


in  float gValue[];
in  float clip[];
out float fValue;
out vec3 fNormal;


uniform mat4 P;
uniform mat4 MV;
uniform vec3 scales;
uniform float radius = 0.05;
uniform int glyphStride = 1;


void EmitWorld(const vec3 v);
void CylinderVert(const vec3 v, const vec3 d, const float value);
void Face(const vec3 a, const vec3 b, const vec3 c);
void SubDiv(const vec3 a, const vec3 b, const vec3 c);


void main()
{
    if (gl_PrimitiveIDIn % glyphStride != 0)
        return;

    if (clip[1] < 0.0 || clip[2] < 0.0)
        return;

    fValue = gValue[1];
    vec3 o = gl_in[1].gl_Position.xyz;

    for (int ti = 0; ti < REFINEMENT; ti++) {
        float t  = float(ti  )/float(REFINEMENT) * PI;
        float tn = float(ti+1)/float(REFINEMENT) * PI;

        for (int ai = 0; ai < REFINEMENT+1; ai++) {
            float a = float(ai)/float(REFINEMENT) * 2.f * PI;

            vec3 d = vec3(
                sin(t)*cos(a),
                sin(t)*sin(a),
                cos(t)
            );
            vec3 dn = vec3(
                sin(tn)*cos(a),
                sin(tn)*sin(a),
                cos(tn)
            );

            fNormal = d;
            EmitWorld(o + radius * d / scales);

            fNormal = dn;
            EmitWorld(o + radius * dn / scales);
        }
        EndPrimitive();
    }
} 


void EmitWorld(const vec3 v)
{
    gl_Position = P * MV * vec4(v, 1.0f);
    EmitVertex();
}


