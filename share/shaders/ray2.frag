#version 330 core

#include RayMath.frag

uniform mat4 MVP;
// uniform vec2 resolution;
uniform vec3 cameraPos;
uniform vec3 dataBoundsMin;
uniform vec3 dataBoundsMax;
uniform float LUTMin;
uniform float LUTMax;

uniform sampler3D data;
uniform sampler1D LUT;
uniform sampler3D coordLUT;

in vec2 ST;

out vec4 fragColor;

void main(void)
{
    vec2 screen = ST*2-1;
    vec4 world = inverse(MVP) * vec4(screen, 1, 1);
    world /= world.w;
    vec3 dir = normalize(world.xyz - cameraPos);
    
    vec4 accum = vec4(0);
    float t0, t1;
    
    if (IntersectRayBoundingBox(cameraPos, dir, dataBoundsMin, dataBoundsMax, t0, t1)) {
        
        float step = max(((t1-t0)/100.f)*1.01, (dataBoundsMax[2]-dataBoundsMin[2])/100.f);
        
		int stepi = 0;
        for (float t = t0; t < t1 && stepi < 100; t += step, stepi++) {
            vec3 hit = cameraPos + dir * t;
            vec3 coordSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
			vec3 dataSTR = texture(coordLUT, coordSTR).rgb;

			vec4 color = vec4(dataSTR, 1);

			if (dataSTR[0] == -1) {
				// color = vec4(1,0,0,1);
				color = vec4(0);
			} else {
				// color = vec4(1);
				float dataNorm = (texture(data, dataSTR).r - LUTMin) / (LUTMax - LUTMin);
				color = texture(LUT, dataNorm);
			}
            
            accum.rgb += color.rgb * color.a * (1-accum.a);
            accum.a += color.a * (1-accum.a);
            
            if (accum.a > 0.999)
                break;
        }
        
        fragColor = accum;
    }
        
    if (accum.a < 0.1)
        discard;
}
