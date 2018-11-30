#version 330 core

uniform mat4 MVP;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 vVertexData;

out vec2 vertexData;

uniform vec4 clippingPlanes[6];
out float gl_ClipDistance[6];

void main()
{
	gl_Position = MVP * vec4(vertex, 1.0);
	vertexData = vVertexData;

	for (int i = 0; i < 6; i++)
		gl_ClipDistance[i] = dot(vec4(vertex, 1.0), clippingPlanes[i]);
}
