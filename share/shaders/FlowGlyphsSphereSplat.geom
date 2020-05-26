// This generates a billboard with a width and height equal to the diameter the sphere

#version 330 core

#include FlowInclude.geom

#define REFINEMENT 10

layout (lines_adjacency) in;
layout (triangle_strip, max_vertices = 4) out;


in  float gValue[];
in  float clip[];
out float fValue;
out vec2 fC;


uniform mat4 P;
uniform mat4 MV;
uniform vec3 cameraPos;
uniform vec3 scales;
uniform float radius = 0.05;
uniform int glyphStride = 1;
uniform bool showOnlyLeadingSample = false;


void EmitWorld(const vec3 v);
void CylinderVert(const vec3 v, const vec3 d, const float value);
void Face(const vec3 a, const vec3 b, const vec3 c);
void SubDiv(const vec3 a, const vec3 b, const vec3 c);


void main()
{
	if (showOnlyLeadingSample) {
		if (gl_PrimitiveIDIn < nVertices-4)
			return;
	} else {
		if (gl_PrimitiveIDIn % glyphStride != 0)
			return;
	}

	// Need to manually clip if using geometry shader.
    if (clip[1] < 0.0 || clip[2] < 0.0)
        return;

    fValue = gValue[1];
    vec3 o = gl_in[1].gl_Position.xyz;

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
} 


void EmitWorld(const vec3 v)
{
    gl_Position = P * MV * vec4(v, 1.0f);
    EmitVertex();
}


