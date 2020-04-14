#version 330 core

uniform bool constantColorEnabled = false;
uniform vec3 constantColor = vec3(1.0f);
uniform vec3 lightDir = vec3(0, 0, -1);
uniform bool lightingEnabled = true;

uniform sampler1D LUT;
uniform vec2 mapRange;

in float fValue;
in float fFade;
in  vec3 fNormal;
out vec4 fragment;

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

        float diffuse = max(dot(normal, lightDir), 0.0);
        color.rgb *= max(diffuse, 0.2f);
    }

    color.a *= fFade;
    if (fFade < 0.05)
        discard;

    fragment = color;
}
