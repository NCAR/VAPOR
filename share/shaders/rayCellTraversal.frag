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
uniform sampler3D coords;

in vec2 ST;

out vec4 fragColor;


bool IntersectRayPlane(vec3 o, vec3 d, vec3 v0, vec3 n, out float t)
{
	 float denom = dot(n, d);
	
	if (denom > 1e-6) {
		t = dot(v0 - o, n) / denom;
		return t >= 0;
	}
	return false;
}


bool IntersectRayQuad(vec3 o, vec3 d, vec3 v1, vec3 v2, vec3 v3, vec3 v4, out float t)
{
	vec3 n = cross(v2-v1, v3-v1);
	
	// if (abs(dot(n, d)) < 
	return false;
}


void main(void)
{
    vec2 screen = ST*2-1;
    vec4 world = inverse(MVP) * vec4(screen, 1, 1);
    world /= world.w;
    vec3 dir = normalize(world.xyz - cameraPos);
    
    vec4 accum = vec4(0);
    float t0, t1, tp;
    
    if (IntersectRayBoundingBox(cameraPos, dir, dataBoundsMin, dataBoundsMax, t0, t1)) {

		if (IntersectRayPlane(cameraPos, dir, vec3(1), vec3(0,0,-1), tp)) {
			if (tp < t0) { 
				fragColor = vec4(vec3(1), 1);
				return;
			}
		}
        
        float step = max(((t1-t0)/100.f)*1.01, (dataBoundsMax[2]-dataBoundsMin[2])/100.f);
        
		int stepi = 0;
        for (float t = t0; t < t1 && stepi < 100; t += step, stepi++) {
            vec3 hit = cameraPos + dir * t;
            vec3 coordSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
			vec3 dataSTR = texture(coords, coordSTR).rgb;

			vec4 color = vec4(dataSTR/3, 1);
            
            accum.rgb += color.rgb * color.a * (1-accum.a);
            accum.a += color.a * (1-accum.a);
            
            if (accum.a > 0.999)
                break;
        }
        
        fragColor = accum;
    } else {
	}
        
    if (accum.a < 0.1)
        discard;
}
