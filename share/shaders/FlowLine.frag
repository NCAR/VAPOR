#version 410 core

layout(location = 0) out vec4 color;

in float            scalarV;
in vec3             vertModel;

uniform sampler1D   colorMapTexture;
uniform vec3        colorMapRange;
uniform bool        singleColor;


void main(void)
{
    if( singleColor )
    {
        color = texelFetch( colorMapTexture, 0, 0 );
    }
    else
    {
        float valTranslate = (scalarV - colorMapRange.x) / colorMapRange.z;
        color = texture( colorMapTexture, valTranslate );
    }
}

