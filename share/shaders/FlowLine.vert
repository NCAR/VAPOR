#version 410 core

// Input vertex position (.xyz) and colormap value (.w)
layout(location = 0) in vec4 vertInfo;

uniform mat4 MV;
uniform mat4 Projection;

out float scalarV;
out vec3  vertModel;

void main(void)
{
    gl_Position = Projection * MV * vec4( vertInfo.xyz, 1.0 );
    scalarV     = vertInfo.w;
    vertModel   = vertInfo.xyz;
}
