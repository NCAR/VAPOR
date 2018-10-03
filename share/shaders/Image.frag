#version 330 core

uniform sampler2D sampler;
uniform float constantOpacity;

in vec2 TextureCoord;

out vec4 fragColor;

void main(void)
{
	vec4 color = texture(sampler, TextureCoord);

	// Legacy vapor behavior
	if (color.a <= 0.1)
		discard;

	fragColor = vec4(color.rgb, color.a * constantOpacity);
}
