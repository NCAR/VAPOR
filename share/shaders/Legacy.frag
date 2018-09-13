#version 330 core

uniform bool lightingEnabled;
uniform vec3 lightDir;

in  vec3 fNormal;
in  vec4 fColor;
out vec4 fragment;

void main() {
    vec3 color;
    if (lightingEnabled) {
		vec3 normal;
		if (gl_FrontFacing)
			normal = fNormal;
		else 
			normal = -fNormal;

        float diffuse = max(dot(normal, -lightDir), 0.0);
        color = fColor.rgb * (diffuse + 0.2);
    } else {
        color = fColor.rgb;
    }
    fragment = vec4(color, fColor.a);
}
