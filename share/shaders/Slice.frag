#version 330 core

uniform sampler1D colormap;
uniform sampler2D dataValues;
uniform float minLUTValue;
uniform float maxLUTValue;
uniform float constantOpacity;

//in vec2 vertexData;
in vec2 fTexCoord;

out vec4 fragColor;

void main(void)
{
	if (minLUTValue >= maxLUTValue) discard;

    float value = texture(dataValues, fTexCoord).r;
    //float red = texture(colormap, fTexCoord.x).r;
    //float green = texture(colormap, fTexCoord.x).g;
    //float blue = texture(colormap, fTexCoord.x).b;
    //fragColor = vec4(red , green, blue, 1);


    float normalized = (value - minLUTValue) / (maxLUTValue - minLUTValue);
    //fragColor = vec4(normalized, 0, 0, 1);
    //fragColor = vec4(normalized, 0, 0, 1);

    vec4  color = texture(colormap, normalized);
    //vec4  color = texture(colormap, fTexCoord.x);
    fragColor = color;
    //fragColor = vec4(color.rgb, color.a*constantopacity);




    //fragColor = vec4(fTexCoord.x, fTexCoord.y, 0, 1);
    //fragColor = vec4(value, 0, 0, 1);//constantOpacity);
}
