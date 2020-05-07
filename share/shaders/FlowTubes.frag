#version 330 core

uniform bool constantColorEnabled = false;
uniform vec3 constantColor = vec3(1.0f);
uniform vec3 lightDir = vec3(0, 0, -1);
uniform bool lightingEnabled = true;

uniform sampler1D LUT;
uniform vec2 mapRange;
uniform vec3 scales;

in float fValue;
in float fFade;
in vec3  fNormal;
out vec4 fragment;

uniform float phongAmbient;
uniform float phongDiffuse;
uniform float phongSpecular;
uniform float phongShininess;
float PhongLighting(vec3 normal, vec3 viewDir)
{
	if (!lightingEnabled)
		return 1.0;
    
    vec3 lightDir = viewDir;

    float diffuse = abs(dot(normal, -lightDir)) * phongDiffuse;

    float specularStrength = phongSpecular;
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = pow(abs(dot(viewDir, reflectDir)), phongShininess);
    float specular = specularStrength * spec;

    return max(phongAmbient + diffuse + specular, phongAmbient);
}

void main() {
    vec4 color;
    if (constantColorEnabled)
        color = vec4(constantColor, 1.0f);
    else
        color = texture(LUT, (fValue - mapRange.x) / (mapRange.y - mapRange.x));

    if (lightingEnabled) {
		vec3 normal;
		if (gl_FrontFacing)
			normal = -fNormal;
		else 
			normal = fNormal;

		vec3 ld = normalize(lightDir * scales);
        // float diffuse = max(dot(normal, ld), 0.0);
        // color.rgb *= max(diffuse, 0.2f);
        color.rgb *= PhongLighting(normal, ld);
    }

    color.a *= fFade;
    if (fFade < 0.05)
        discard;

    fragment = color;
}

