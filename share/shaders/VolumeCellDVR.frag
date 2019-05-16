#pragma auto_version

#include VolumeCellBase.frag

vec4 RenderCellSampling(const vec3 dir, const vec3 entranceCoord, const vec3 exitCoord, const float t0, const float t1, const float step)
{
    vec4 acc2 = vec4(0);
    for (float t = step * (floor(t0/step)+1); t < t1; t+= step) {
        vec3 hit = mix(entranceCoord, exitCoord, (t-t0)/(t1-t0));
        vec4 color = GetColorAtCoord(hit);
        vec3 normal = GetNormalAtCoord(hit);
        color.rgb *= PhongLighting(normal, dir);
        BlendToBack(acc2, PremultiplyAlpha(color));
    }
    return acc2;
}

vec4 RenderCellSmartSampling(const vec3 dir, vec3 rayLightingNormal, const vec3 entranceCoord, const vec3 exitCoord, const float tStart, const float tEnd, const float t0, const float t1, const float step, const float stepOpacityUnit)
{
    vec4 acc2 = vec4(0);
    vec3 hit = mix(entranceCoord, exitCoord, (tStart-t0)/(t1-t0));
    vec4 c = GetColorAtCoord(hit);
    vec3 normal = GetNormalAtCoord(hit);
    c.rgb *= PhongLighting(normal, rayLightingNormal);
    float l = min(step - mod(tStart, step), tEnd-tStart)/step;
    c.a = IntegrateConstantAlpha(c.a, l * stepOpacityUnit);
    BlendToBack(acc2, PremultiplyAlpha(c));
    
    for (float t = step * (floor(tStart/step)+1); t < tEnd; t+= step) {
        vec3 hit = mix(entranceCoord, exitCoord, (t-t0)/(t1-t0));
        vec4 color = GetColorAtCoord(hit);
        vec3 normal = GetNormalAtCoord(hit);
        color.rgb *= PhongLighting(normal, rayLightingNormal);
        float l = min(step, t1-t)/step;
        color.a = IntegrateConstantAlpha(color.a, l * stepOpacityUnit);
        BlendToBack(acc2, PremultiplyAlpha(color));
    }
    
    return acc2;
}

vec4 RenderCellConstant(const vec3 dir, const vec3 rayLightingNormal, const ivec3 currentCell, const float t0, const float t1, const float unitDistanceScaled)
{
    float l = (t1-t0)/unitDistanceScaled;
    vec4 color = GetAverageColorForCoordIndex(currentCell);
    
    vec3 normal = GetNormal((vec3(currentCell)+vec3(0.5))/cellDims);
    color.rgb *= PhongLighting(normal, rayLightingNormal);
    
    color.a = IntegrateConstantAlpha(color.a, l * unitOpacityScalar);
    
    return PremultiplyAlpha(color);
}

vec4 Traverse(vec3 origin, vec3 dir, vec3 rayLightingNormal, float tMin, float tMax, float t0, ivec3 currentCell, ivec3 entranceFace, OUT float t1)
{
    vec3 entranceCoord;
    ivec3 nextCell;
    ivec3 exitFace;
    vec3 exitCoord;
    bool hasNext = true;
    float tStart = t0;
    ivec3 initialCell = currentCell;
    float unitDistanceScaled = unitDistance / length(dir * scales);
    float step = unitDistanceScaled/7.f * GetSamplingNoise();
    float stepOpacityUnit = unitOpacityScalar/7.f;
    if (fast) {
        step = unitDistanceScaled * GetSamplingNoise();
        stepOpacityUnit = unitOpacityScalar;
    }
    
    int i = 0;
    vec4 accum = vec4(0);
    float a = 0;
    
    float null;
    IntersectRayCellFace(origin, dir, -FLT_MAX, currentCell, entranceFace, null, entranceCoord);
    
    while (hasNext) {
        if (t0 > tMax)
            break;
        
        hasNext = FindNextCell(origin, dir, t0, currentCell, entranceFace, nextCell, exitFace, exitCoord, t1);
        
        if (t0 >= tMin || (t0 <= tMin && tMin < t1)) {
            float tEnd = min(t1, tMax);
            float tStart = max(t0, tMin);
            
            if (ShouldRenderCell(currentCell)) {
#if 1
                BlendToBack(accum, RenderCellSmartSampling(dir, rayLightingNormal, entranceCoord, exitCoord, tStart, tEnd, t0, t1, step, stepOpacityUnit));
#else
                BlendToBack(accum, RenderCellConstant     (dir, rayLightingNormal, currentCell, tStart, tEnd, unitDistanceScaled));
#endif
            }
        }
        
        currentCell = nextCell;
        entranceFace = -exitFace;
        entranceCoord = exitCoord;
        t0 = t1;
        i++;
        
        if (accum.a > ALPHA_BREAK || i > 4096)
            break;
    }
    
    gl_FragDepth = CalculateDepth(cameraPos + dir*t1);
    return accum;
}
