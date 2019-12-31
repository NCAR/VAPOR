#version 330 core

uniform sampler1D colormap;
uniform float minLUTValue;
uniform float maxLUTValue;

in float fValue;
in float fMissing;
out vec4 FragColor;

void main() {
    //FragColor = fColor;
    
    float s = (fValue - minLUTValue) / (maxLUTValue - minLUTValue);
    vec4 color = texture(colormap, s);
    FragColor = color;
    
    if (fMissing != 0.f)
        discard;
}
