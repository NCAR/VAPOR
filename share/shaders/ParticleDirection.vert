#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in float vValue;

out vec3 gNorm;
out float gValue;

#ifdef DYNAMIC_RADIUS
layout (location = 3) in float vRadius;
out float gRadius;
#endif

void main() {
    gValue = vValue;
    gNorm = vNorm;
    gl_Position = vec4(vPos, 1.0f);

    #ifdef DYNAMIC_RADIUS
        gRadius = vRadius;
    #endif
}
