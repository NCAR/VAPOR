#version 330 core

#define PI (3.1415926)

uniform bool constantColorEnabled = false;
uniform vec3 constantColor = vec3(1.0f);
uniform vec3 lightDir = vec3(0, 0, -1);
uniform bool lightingEnabled = true;

uniform sampler1D LUT;
uniform vec2 mapRange;
uniform vec3 scales;

in float fValue;
in  vec2 fC;
out vec4 fragment;

uniform float phongAmbient;
uniform float phongDiffuse;
uniform float phongSpecular;
uniform float phongShininess;
float PhongLighting(float theta)
{
	if (!lightingEnabled)
		return 1.0;

	float ct = cos(theta);
    float diffuse = ct * phongDiffuse;
    float specular = phongSpecular * pow(ct, phongShininess);
    return max(phongAmbient + diffuse + specular, phongAmbient);
}

void main() {
    vec4 color;
    if (constantColorEnabled)
        color = vec4(constantColor, 1.0f);
    else
        color = texture(LUT, (fValue - mapRange.x) / (mapRange.y - mapRange.x));

	float a = length(fC);
	float t = acos(a);
	float t2 = PI/2.0 - t;

	if (a > 1)
		discard;

	color.rgb *= PhongLighting(t2);
    fragment = color;
}
