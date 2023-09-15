#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in float value;

out float fValue;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(vPos, 1.0);
    fValue = value;
}
