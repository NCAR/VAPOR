#version 410 core

layout(location = 0) out vec4 color;

in float scalarV;
in vec4  vertClip;

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

    // Let's calculate and set the depth of this fragment
    vec3 vertNdc = vertClip.xyz / vertClip.w;
    gl_FragDepth = 0.5 * fma( gl_DepthRange.diff, vertNdc.z, (gl_DepthRange.near + gl_DepthRange.far) );
}

