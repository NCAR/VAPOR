#version 330 core

uniform sampler2D sampler;
uniform float near;
uniform float far;
uniform bool linearize;

in vec2 ST;

out vec4 fragColor;

void main(void)
{
	float depth = texture(sampler, ST).r;
	if (linearize)
		depth = (2.0 * near) / (far + near - depth * (far - near));
	fragColor = vec4(vec3(depth), 1.0);
}
