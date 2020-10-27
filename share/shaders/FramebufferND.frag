#version 330 core

// Draw framebuffer while translating OSPRay depth buffer to OpenGL depth buffer.

uniform mat4 MVP;
uniform vec3 camPos;
uniform vec3 camDir;
uniform sampler2D colorBuffer;
uniform sampler2D depthBuffer;

in vec2 ST;

out vec4 fragColor;

float CalculateDepth(float d)
{
    vec4 ndc = MVP * vec4(camPos + camDir*d, 1);
    ndc.xyz /= ndc.w;
    return 0.5 * (gl_DepthRange.diff * ndc.z + (gl_DepthRange.near + gl_DepthRange.far));
}

void main(void)
{
    fragColor = texture(colorBuffer, ST);
    gl_FragDepth = CalculateDepth(texture(depthBuffer, ST).r);
}
