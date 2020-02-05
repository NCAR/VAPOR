#pragma auto_version

#include VolumeBase.frag
#include VolumeIsoInclude.frag

void TestIso(vec3 eye, vec3 dir, vec3 rayLightingNormal, float value, float dv, float ld, float step, float t, inout vec4 accum)
{
	if ((ld < value && dv >= value) || (ld > value && dv <= value)) {
		float lt = t - step;
		float t = lt + step*(value-ld)/(dv-ld);
		
		vec3 hit = eye + dir * t;
		vec3 dataSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);

		vec4 color = GetIsoSurfaceColor(dataSTR);
		vec3 normal = GetNormal(dataSTR);
		
		color.rgb *= PhongLighting(normal, rayLightingNormal);
		
		BlendToBack(accum, PremultiplyAlpha(color));
	}
}

void main(void)
{
    vec3 eye, dir, rayLightingNormal;
    float sceneDepthT;
    GetRayParameters(eye, dir, rayLightingNormal, sceneDepthT);
    
    vec4 accum = vec4(0);
    float t0, t1;
    
    if (IntersectRayBoundingBox(eye, dir, 0, userExtsMin, userExtsMax, t0, t1)) {

        int STEPS;
        if (fast) {
            STEPS = 100;
        } else
            STEPS = int(700 * samplingRateMultiplier);

        float step = max(((t1-t0)/float(STEPS))*1.01, (dataBoundsMax[2]-dataBoundsMin[2])/float(STEPS));
		vec3 initialSample = ((eye + dir * t0) - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
        float ld = texture(data, initialSample).r;
		bool lastShouldRender = ShouldRenderSample(initialSample);
        
        t1 = min(t1, sceneDepthT);
        int i = 0;
        for (float t = t0; t < t1; t += step) {
            vec3 hit = eye + dir * t;
            vec3 dataSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
            float dv = texture(data, dataSTR).r;
			bool shouldRender = ShouldRenderSample(dataSTR);
            
			if (shouldRender && lastShouldRender) {
				// Unrolled intentionally
				if (isoEnabled[0]) TestIso(eye, dir, rayLightingNormal, isoValue[0], dv, ld, step, t, accum);
				if (isoEnabled[1]) TestIso(eye, dir, rayLightingNormal, isoValue[1], dv, ld, step, t, accum);
				if (isoEnabled[2]) TestIso(eye, dir, rayLightingNormal, isoValue[2], dv, ld, step, t, accum);
				if (isoEnabled[3]) TestIso(eye, dir, rayLightingNormal, isoValue[3], dv, ld, step, t, accum);
                
                if (accum.a > ALPHA_BREAK)
                    gl_FragDepth = CalculateDepth(hit);
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
        
    if (accum.a < ALPHA_DISCARD)
        gl_FragDepth = GetDepthBuffer();
}
