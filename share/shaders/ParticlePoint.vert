#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in float vValue;

out float gValue;

void main() {
    gValue = vValue;
    gl_Position = vec4(vPos, 1.0f);
}
