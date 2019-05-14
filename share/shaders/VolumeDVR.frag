#pragma auto_version

#include VolumeBase.frag

/*
float Shadow(vec3 to) {
    float t = 0, t0, t1;
    IntersectRayBoundingBox(to, -lightDir, 0, vec3(0), vec3(1), t0, t1);
    //return t1;
    
    float acc = 0;
    for (; t < t1; t+= 0.05) {
        float dataNorm = (texture(data, to-t*lightDir).r - LUTMin) / (LUTMax - LUTMin);
        float opacity = texture(LUT, dataNorm).a;
        acc += opacity * (1-acc);
    }
    return 1-acc;
}
 */

float IntegrateConstantAlpha(float a, float distance)
{
    return 1 - exp(-a * distance);
}

void main(void)
{
    vec3 dir, rayLightingNormal;
    float sceneDepthT;
    GetRayParameters(dir, rayLightingNormal, sceneDepthT);
    
    vec4 accum = vec4(0);
    float t0, t1;
    
    if (IntersectRayBoundingBox(cameraPos, dir, 0, userExtsMin, userExtsMax, t0, t1)) {
        
        int STEPS;
        float integratePart = 1/7.0;
        float noise = 1;
        if (fast) {
            STEPS = 100;
            integratePart = 1;
            noise = fract(sin(gl_FragCoord.x * 12.989 + gl_FragCoord.y * 78.233) * 43758.5453) * 0.1 + 1; // Compensate for lower sampling rate
        } else
            STEPS = 700;
        float step = noise * max(((t1-t0)/float(STEPS))*1.01, (dataBoundsMax[2]-dataBoundsMin[2])/float(STEPS));
        
        t1 = min(t1, sceneDepthT);
        int i = 0;
        for (float t = t0; t < t1; t += step) {
            
            vec3 hit = cameraPos + dir * t;
            vec3 dataSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
            float dataNorm = (texture(data, dataSTR).r - LUTMin) / (LUTMax - LUTMin);
            vec4 color = texture(LUT, dataNorm);
            vec3 normal = GetNormal(dataSTR);
			
            color.rgb *= PhongLighting(normal, rayLightingNormal);
            
            color.a = IntegrateConstantAlpha(color.a, integratePart);
            
            if (ShouldRenderSample(dataSTR))
                BlendToBack(accum, PremultiplyAlpha(color));
            
            if (accum.a > ALPHA_BREAK) {
                t1 = t;
                break;
            }
                
            // Failsafe
            if (i++ > STEPS)
                break;
        }
        
        gl_FragDepth = CalculateDepth(cameraPos + dir*t1);
        fragColor = accum;
    }
    if (accum.a < ALPHA_DISCARD)
        discard;
}
