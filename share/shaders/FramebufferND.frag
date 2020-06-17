#version 330 core

uniform sampler2D colorBuffer;

in vec2 ST;

out vec4 fragColor;

void main(void)
{
    fragColor = texture(colorBuffer, ST);
}
