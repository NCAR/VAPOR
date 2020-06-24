// Basic fragment shader. Maps the fValue onto a color using the LUT.
#version 330 core

uniform bool constantColorEnabled = false;
uniform vec3 constantColor = vec3(1.0f);
uniform float radius = 0.05f;
uniform float border = 0.f;
uniform vec3 borderColor = vec3(1.0f);
uniform bool antiAlias = false;

uniform sampler1D LUT;
uniform vec2 mapRange;

in float fValue;
in float t;

out vec4 fragment;


void main()
{
    if (constantColorEnabled)
        fragment = vec4(constantColor, 1.0f);
    else {
        vec4 c = texture(LUT, (fValue - mapRange.x) / (mapRange.y - mapRange.x));

        if (antiAlias) {
            float S = 12;
            S *= (radius + border) / 0.1;
            c.rgb = mix(c.rgb, borderColor, clamp((abs(t)-radius/(radius+border))*S, 0.f, 1.f));
            c.a *= min(((1-abs(t))*S), 1);
        } else {
            if (abs(t) > radius/(radius+border))
                c = vec4(borderColor, 1.f);
        }


        fragment = c;
    }
}
