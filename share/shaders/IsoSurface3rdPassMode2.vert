#version 410 core

// Input vertex positions, **un-normalized** model coordinate
layout(location = 0) in vec3 vertexPosition;

// Input vertex indices in X, Y, Z directions
layout(location = 1) in ivec4 vertexLogicalIdx;

uniform mat4 MV;
uniform mat4 Projection;

flat out ivec4 provokingVertexIdx;

void main(void)
{
    gl_Position        = Projection * MV * vec4( vertexPosition, 1.0 );
    provokingVertexIdx = vertexLogicalIdx;
}
