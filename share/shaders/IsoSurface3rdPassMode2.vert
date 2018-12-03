#version 410 core

// Input vertex positions, **un-normalized** model coordinate
layout(location = 0) in vec3 vertexPosition;

uniform mat4 MVP;

void main(void)
{
    vec4 positionModel = vec4( vertexPosition, 1.0f );
    gl_Position        = MVP * positionModel;
}
