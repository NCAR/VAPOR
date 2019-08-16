#version 330 core

uniform sampler2D colorBuffer;
uniform sampler2D depthBuffer;

in vec2 ST;

out vec4 fragColor;

void main(void)
{
    fragColor = texture(colorBuffer, ST);
    gl_FragDepth = texture(depthBuffer, ST).r;
}
