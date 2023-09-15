#version 330 core

uniform bool lightingEnabled;
uniform bool textureEnabled;
uniform vec3 lightDir;

uniform sampler2D sampler;

in  vec4 fColor;
in  vec3 fNormal;
in  vec2 fTextureCoords;
out vec4 fragment;

void main() {
    vec4 color = fColor;
	if (textureEnabled) {
		color *= texture(sampler, fTextureCoords);
	}
    if (lightingEnabled) {
		vec3 normal;
		if (gl_FrontFacing)
			normal = fNormal;
		else 
			normal = -fNormal;

        float diffuse = max(dot(normal, -lightDir), 0.0);
        color.rgb *= max(diffuse, 0.2f);
    }
    fragment = color;
}
