#version 330 core

#include RayMath.frag

uniform mat4 MVP;
// uniform vec2 resolution;
uniform vec3 cameraPos;
uniform vec3 dataBoundsMin;
uniform vec3 dataBoundsMax;
uniform float LUTMin;
uniform float LUTMax;

uniform ivec3 coordDims;

uniform sampler3D data;
uniform sampler1D LUT;
uniform sampler3D coords;

in vec2 ST;

out vec4 fragColor;


void main(void)
{
    
    vec2 screen = ST*2-1;
    vec4 world = inverse(MVP) * vec4(screen, 1, 1);
    world /= world.w;
    vec3 dir = normalize(world.xyz - cameraPos);
    
    vec4 accum = vec4(0);
    float t0, t1, tp;
    
    bool intersectBox = IntersectRayBoundingBox(cameraPos, dir, dataBoundsMin, dataBoundsMax, t0, t1);
    
    if (intersectBox) {
        
        for (int y = 0; y < coordDims[1]-1; y++) {
            for (int x = 0; x < coordDims[0]-1; x++) {
                float t;
                if (IntersectRayQuad(cameraPos, dir,
                    texture(coords, vec3(x  , y  , 0)/coordDims).xyz,
                    texture(coords, vec3(x+1, y  , 0)/coordDims).xyz,
                    texture(coords, vec3(x+1, y+1, 0)/coordDims).xyz,
                    texture(coords, vec3(x  , y+1, 0)/coordDims).xyz,
                    t)) {
                        float dataNorm = (texture(data, (vec3(x,y,0)+vec3(0.5))/(coordDims-1)).r - LUTMin) / (LUTMax - LUTMin);
                        fragColor = texture(LUT, dataNorm);
                        return;
                }
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
    }
        
    if (accum.a < 0.1)
        discard;
}
