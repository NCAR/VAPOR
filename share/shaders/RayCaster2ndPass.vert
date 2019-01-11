#version 410 core

// Input vertex positions, model coordinate
layout(location = 0) in vec3 vertexPosition;

uniform mat4 MV;
uniform mat4 Projection;

// Output vertex positions in eye coordinates
out vec4 vEye;

void main(void)
{
    vEye        = MV * vec4( vertexPosition, 1.0 );
    gl_Position = Projection * vEye;
}
