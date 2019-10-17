#version 410 core

layout(location = 0) out vec4 color;

in float            scalarV;
in vec3             vertModel;

uniform sampler1D   colorMapTexture;
uniform vec3        colorMapRange;
uniform bool        singleColor;
uniform vec4        clipPlanes[6];     // clipping planes in model coordinates


void main(void)
{
    // First, let's decide if we want to discard this fragment
    vec4 positionModel = vec4( vertModel, 1.0 );
    for( int i = 0; i < 6; i++ )
    {   
        if( dot(positionModel, clipPlanes[i]) < 0.0 )
        {
            discard;
            return;
        }
    } 

    // If the fragment isn't discarded, let's calculate its color
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

