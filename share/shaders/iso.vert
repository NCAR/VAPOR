#version 330 core

layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 vTextureCoord;

out vec2 ST;

void main()
{
	gl_Position = vec4(vertex, 0.0, 1.0);
	ST = vTextureCoord;
}
