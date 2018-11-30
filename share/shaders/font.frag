#version 330 core
in vec2 TexCoords;
out vec4 fragment;

uniform vec4 color;
uniform sampler2D text;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    fragment = color * sampled;
}
