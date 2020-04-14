#version 330 core
#define FLT_EPSILON 1.19e-07
#define PI float(3.1415926535897932384626433832795)

#define REFINEMENT 10

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 64) out;


in  float gValue[];
out float fValue;
out vec3 fNormal;


uniform mat4 P;
uniform mat4 MV;
uniform float radius = 0.05;


void EmitWorld(const vec3 v);
void CylinderVert(const vec3 v, const vec3 d, const float value);
void Face(const vec3 a, const vec3 b, const vec3 c);
void SubDiv(const vec3 a, const vec3 b, const vec3 c);


void main()
{
    fValue = gValue[1];

    vec3 V[4];

    float R = radius;
    float d = R/2.f;
    float a = R*sqrt(3.f);

    V[0] = vec3(-a/2.f, -d, -d);
    V[1] = vec3( a/2.f, -d, -d);
    V[2] = vec3(     0,  R, -d);
    V[3] = vec3(     0,  0,  R);

    Face(V[0], V[1], V[3]);
    Face(V[1], V[2], V[3]);
    Face(V[2], V[0], V[3]);
    Face(V[1], V[0], V[2]);
} 


void EmitWorld(const vec3 v)
{
    gl_Position = P * MV * vec4(v, 1.0f);
    EmitVertex();
}


void Face(const vec3 a, const vec3 b, const vec3 c)
{
    vec3 p = gl_in[1].gl_Position.xyz;

    // fNormal = normalize(((a + b + c)/3.f);
    fNormal = normalize(cross(a, b));


    EmitWorld(p + a);
    EmitWorld(p + b);
    EmitWorld(p + c);

    EndPrimitive();
}

// void SubDiv(const vec3 a, const vec3 b, const vec3 c)
// {
//     int N = 2;
// 
//     int Nx = 1;
//     int Ny = N + 1;
// 
//     for (int y = 0; y < Ny; y++) {
// 
//         float y[2];
//         y[1] = y     * (1.f/float(Ny);
//         y[0] = (y+1) * (1.f/float(Ny);
// 
//         for (int x = 0; x < Nx; x++) {
// 
//             vec3 cc = lerp(
// 
//         }
//         Nx += 2;
//         EndPrimitive();
//     }
// }
