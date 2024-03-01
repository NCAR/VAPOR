// This generates a billboard with a width and height equal to the diameter the sphere

#version 330 core

#define REFINEMENT 10

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;


in  float gValue[];
out float fValue;
out vec2 fC;


uniform mat4 P;
uniform mat4 MV;
uniform vec3 cameraPos;
uniform vec3 scales;
uniform float radius = 0.05;


void EmitWorld(const vec3 v);


void main()
{
    fValue = gValue[0];
    vec3 o = gl_in[0].gl_Position.xyz;

	// Plane perpendicular to camera normal
	vec3 up = vec3(0.0, 0.0, 1.0);
	vec3 toCamera = normalize(cameraPos - o);
	vec3 xh = normalize(cross(up, toCamera));
	vec3 yh = normalize(cross(toCamera, xh));
	xh /= scales;
	yh /= scales;

	// Billboard
	fC = vec2(-1.0, -1.0); EmitWorld(o + radius * (-xh + -yh));
	fC = vec2( 1.0, -1.0); EmitWorld(o + radius * ( xh + -yh));
	fC = vec2(-1.0,  1.0); EmitWorld(o + radius * (-xh +  yh));
	fC = vec2( 1.0,  1.0); EmitWorld(o + radius * ( xh +  yh));
	EndPrimitive();
} 


void EmitWorld(const vec3 v)
{
    gl_Position = P * MV * vec4(v, 1.0f);
    EmitVertex();
}
