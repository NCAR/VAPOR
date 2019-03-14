#version 410 core

#include VolumeBase.frag

uniform float isoValue[4];

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
		vec3 initialSample = ((cameraPos + dir * t0) - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
        float ld = texture(data, initialSample).r;
		bool lastShouldRender = ShouldRenderSample(initialSample);
        
        t1 = min(t1, sceneDepthT);
        int i = 0;
        for (float t = t0; t < t1; t += step) {
            vec3 hit = cameraPos + dir * t;
            vec3 dataSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
            float dv = texture(data, dataSTR).r;
			bool shouldRender = ShouldRenderSample(dataSTR);
            
			if (shouldRender && lastShouldRender) {
				if ((ld < isoValue[0] && dv >= isoValue[0]) || (ld > isoValue[0] && dv <= isoValue[0])) {
					float lt = t - step;
					float t = lt + step*(isoValue[0]-ld)/(dv-ld);
					
					vec3 hit = cameraPos + dir * t;
					vec3 dataSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
					float data = texture(data, dataSTR).r;
					float dataNorm = (data - LUTMin) / (LUTMax - LUTMin);
					vec4 color = vec4(1);
					vec3 normal = GetNormal(dataSTR);
					
					color.rgb *= PhongLighting(normal, dir);
					
					BlendToBack(accum, PremultiplyAlpha(color));
				}
			}
            ld = dv;
			lastShouldRender = shouldRender;
            
            if (accum.a > ALPHA_BREAK)
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
