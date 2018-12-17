#version 410 core

layout(location = 0) out vec4 color;

in vec4 vModel;

void main(void)
{
    color = vModel;
}
