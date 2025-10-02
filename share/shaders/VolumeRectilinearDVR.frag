#pragma auto_version

#include VolumeBase.frag

uniform ivec3 coordDims;
uniform ivec3 coordSigns;

uniform sampler2D coordLUT;

float IntegrateConstantAlpha(float a, float distance)
{
    return 1 - exp(-a * distance);
}

vec3 ReverseCoordMapping(vec3 p)
{
    return vec3(
        texture(coordLUT, vec2(p.x, 0.0)).x,
        texture(coordLUT, vec2(p.y, 0.5)).x,
        texture(coordLUT, vec2(p.z, 1.0)).x
    );
}

vec3 GetNormalWithReverseCoordMapping(vec3 p)
{
    vec3 dims = vec3(textureSize(data, 0));
    vec3 d = 1/dims * 0.5;
    vec3 s0, s1;
    s1.x = texture(data, ReverseCoordMapping(p + d*vec3(1,0,0))).r;
    s1.y = texture(data, ReverseCoordMapping(p + d*vec3(0,1,0))).r;
    s1.z = texture(data, ReverseCoordMapping(p + d*vec3(0,0,1))).r;
    s0.x = texture(data, ReverseCoordMapping(p - d*vec3(1,0,0))).r;
    s0.y = texture(data, ReverseCoordMapping(p - d*vec3(0,1,0))).r;
    s0.z = texture(data, ReverseCoordMapping(p - d*vec3(0,0,1))).r;

    // glsl::normalize does not handle 0 length vectors
    vec3 v = s1-s0;
    float l = length(v);
    if (l == 0)
        return vec3(0);
    return v/l;
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
        float integratePart = 1 / samplingRateMultiplier;
        if (fast) {
            STEPS = 100;
            integratePart = 7;
        } else {
            STEPS = int(700 * samplingRateMultiplier);
		}
        float step = GetSamplingNoise() * max(((t1-t0)/float(STEPS))*1.01, (dataBoundsMax[2]-dataBoundsMin[2])/float(STEPS));
        
        t1 = min(t1, sceneDepthT);
        int i = 0;
        for (float t = t0; t < t1; t += step) {
            
            vec3 hit = eye + dir * t;
            vec3 dataSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
            vec3 dataSTRMapped = ReverseCoordMapping(dataSTR);

            vec4 color = GetColorForNormalizedCoord(dataSTRMapped);
            vec3 normal = GetNormalWithReverseCoordMapping(dataSTR);
			
            color.rgb *= PhongLighting(normal, rayLightingNormal);
            
            color.a = IntegrateConstantAlpha(color.a * density, integratePart);
            
            if (ShouldRenderSample(dataSTRMapped))
                BlendToBack(accum, PremultiplyAlpha(color));
            
            if (accum.a > ALPHA_BREAK) {
                t1 = t;
                break;
            }
                
            // Failsafe
            if (i++ > STEPS)
                break;
        }
        
        gl_FragDepth = CalculateDepth(eye + dir*t1);
        fragColor = accum;
    } else {
        fragColor = vec4(0.f);
	}
    if (accum.a < ALPHA_DISCARD)
        gl_FragDepth = GetDepthBuffer();
}
