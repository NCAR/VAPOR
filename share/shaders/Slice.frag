#version 330 core

uniform sampler1D colormap;
uniform sampler2D dataValues;

uniform float minLUTValue;
uniform float maxLUTValue;
uniform float constantOpacity;

in  vec2 fTexCoord;
out vec4 fragColor;

void main(void)
{
	if (minLUTValue >= maxLUTValue) discard;

    float missing = texture(dataValues, fTexCoord).g;
    if (missing != 0.f) discard; 

    float value = texture(dataValues, fTexCoord).r;
    //if (isnan(value)) discard;

    float normalized = (value - minLUTValue) / (maxLUTValue - minLUTValue);

    vec4  color = texture(colormap, normalized);
    fragColor = vec4(color.rgb, color.a*constantOpacity);
    
    if (color.a <  0.001)
        discard;
}
