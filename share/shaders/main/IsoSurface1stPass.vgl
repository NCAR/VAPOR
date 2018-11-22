#version 410 core

// Input vertex positions, model coordinate
layout(location = 0) in vec3 vertexPosition;

uniform mat4 MV;
uniform mat4 Projection;

// Output vertex positions in the eye space
out vec4 vEye;

void main(void)
{
    gl_Position = Projection * MV * vec4( vertexPosition, 1.0 );

    vEye = MV * vec4( vertexPosition, 1.0 );
}
