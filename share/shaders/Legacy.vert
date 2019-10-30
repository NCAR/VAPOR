#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec4 vColor;
layout (location = 3) in vec2 vTextureCoords;

out vec3 fNormal;
out vec4 fColor;
out vec2 fTextureCoords;

uniform mat4 P;
uniform mat4 MV;


void main() {
    gl_Position = P * MV * vec4(vPos, 1.0f);
//    fNormal = mat3(transpose(inverse(MV))) * vNormal;
    fNormal = vNormal;
    fColor = vColor;
	fTextureCoords = vTextureCoords;
}
