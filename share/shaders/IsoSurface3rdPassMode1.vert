#version 410 core

// Input vertex positions, **un-normalized** model coordinate
layout(location = 0) in vec3 vertexPosition;

uniform mat4 MV;
uniform mat4 Projection;

void main(void)
{
    gl_Position = Projection * MV * vec4( vertexPosition, 1.0 );
}
