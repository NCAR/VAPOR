#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in float vValue;

out float gValue;

uniform vec4 clippingPlanes[6];
// out float gl_ClipDistance[6];
out float clip;

void main() {
    gValue = vValue;
    gl_Position = vec4(vPos, 1.0f);

    clip = 0;
	for (int i = 0; i < 6; i++)
		clip -= 0.0 > dot(vec4(vPos, 1.0), clippingPlanes[i]) ? 1.0 : 0.0;
}
