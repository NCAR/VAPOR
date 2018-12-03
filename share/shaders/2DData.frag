#version 330 core

uniform sampler1D colormap;
uniform float minLUTValue;
uniform float maxLUTValue;

uniform float constantOpacity;

in vec2 vertexData;

out vec4 fragColor;

void main(void)
{
	if (minLUTValue >= maxLUTValue) discard;

	vec2 texel = vertexData;

	// Check for missing value
	if (texel.y != 0.0) discard;

	float s = (texel.x - minLUTValue) / (maxLUTValue - minLUTValue);
	vec4 color = texture(colormap, s);

	fragColor = vec4(color.rgb, color.a * constantOpacity);
}
