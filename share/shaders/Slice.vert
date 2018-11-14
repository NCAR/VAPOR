#version 330 core

uniform mat4 MVP;

layout (location = 0) in vec3 vVertex;
//layout (location = 1) in vec2 vVertexData;
layout (location = 1) in vec2 vTexCoord;

out vec2 fTexCoord;

uniform vec4 clippingPlanes[6];
out float gl_ClipDistance[6];

void main()
{
    fTexCoord = vTexCoord;

	gl_Position = MVP * vec4(vVertex, 1.0);
//	vertexData = vVertexData;

	for (int i = 0; i < 6; i++)
		gl_ClipDistance[i] = dot(vec4(vVertex, 1.0), clippingPlanes[i]);
}
