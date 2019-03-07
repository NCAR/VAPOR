#version 410 core

layout(location = 0) out vec4 color;

in float scalarV;

void main(void)
{
    if( scalarV < 1.0 )
        color = vec4( 1.0 );
    else
        color = vec4( 1.0, 0.2, 0.2, 1.0 );
}

