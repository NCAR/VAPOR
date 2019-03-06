#version 410 core

#include VolumeBase.frag

uniform float isoValue;

void main(void)
{
    vec3 dir;
    float sceneDepthT;
    GetRayParameters(dir, sceneDepthT);
    
    vec4 accum = vec4(0);
    float t0, t1;
    
    if (IntersectRayBoundingBox(cameraPos, dir, 0, userExtsMin, userExtsMax, t0, t1)) {

#define STEPS 100
        float step = max(((t1-t0)/float(STEPS))*1.01, (dataBoundsMax[2]-dataBoundsMin[2])/float(STEPS));
        float ld = texture(data, ((cameraPos + dir * t0) - dataBoundsMin) / (dataBoundsMax-dataBoundsMin)).r;
        
        int i = 0;
        for (float t = t0; t < t1; t += step) {
            vec3 hit = cameraPos + dir * t;
            vec3 dataSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
            float dv = texture(data, dataSTR).r;
            
            if ((ld < isoValue && dv >= isoValue) || (ld > isoValue && dv <= isoValue)) {
                float lt = t - step;
                float t = lt + step*(isoValue-ld)/(dv-ld);
                
                vec3 hit = cameraPos + dir * t;
                vec3 dataSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
                float data = texture(data, dataSTR).r;
                float dataNorm = (data - LUTMin) / (LUTMax - LUTMin);
                vec4 color = vec4(1);
                vec3 normal = GetNormal(dataSTR);
                
                color.rgb *= PhongLighting(normal, dir);
                
                BlendToBack(accum, PremultiplyAlpha(color));
            }
            ld = dv;
            
            if (accum.a > 0.999)
                break;
                
            // Failsafe
            if (i++ > STEPS)
                break;
        }
        
        fragColor = accum;
    }
        
    if (accum.a < 0.1)
        discard;
}
