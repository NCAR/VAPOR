#version 330 core

uniform mat4 MVP;

layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 vTextureCoord;

out vec2 TextureCoord;

void main()
{
	gl_Position = MVP * vec4(vertex, 1.0);
	TextureCoord = vTextureCoord;
}
