#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in float value;
layout (location = 2) in float missing;

out float fValue;
out float fMissing;

uniform mat4 MVP;

uniform vec4 clippingPlanes[6];
out float gl_ClipDistance[6];

void main() {
    gl_Position = MVP * vec4(vPos, 1.0);
    fValue = value;
    fMissing = missing;

	for (int i = 0; i < 6; i++)
		gl_ClipDistance[i] = dot(vec4(vPos, 1.0), clippingPlanes[i]);
}
