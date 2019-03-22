#version 410 core

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

vec4 RenderCellSmartSampling(const vec3 dir, const vec3 entranceCoord, const vec3 exitCoord, const float t0, const float t1, const float step, const float stepOpacityUnit)
{
    vec4 acc2 = vec4(0);
    vec4 c = GetColorAtCoord(entranceCoord);
    vec3 normal = GetNormalAtCoord(entranceCoord);
    c.rgb *= PhongLighting(normal, dir);
    float l = min(step - mod(t0, step), t1-t0)/step;
    c.a = IntegrateConstantAlpha(c.a, l * stepOpacityUnit);
    BlendToBack(acc2, PremultiplyAlpha(c));
    
    for (float t = step * (floor(t0/step)+1); t < t1; t+= step) {
        vec3 hit = mix(entranceCoord, exitCoord, (t-t0)/(t1-t0));
        vec4 color = GetColorAtCoord(hit);
        vec3 normal = GetNormalAtCoord(hit);
        color.rgb *= PhongLighting(normal, dir);
        float l = min(step, t1-t)/step;
        color.a = IntegrateConstantAlpha(color.a, l * stepOpacityUnit);
        BlendToBack(acc2, PremultiplyAlpha(color));
    }
    
    return acc2;
}

vec4 RenderCellConstant(const vec3 dir, const ivec3 currentCell, const float t0, const float t1, const float unitDistanceScaled)
{
    float l = (t1-t0)/unitDistanceScaled;
    vec4 color = GetAverageColorForCoordIndex(currentCell);
    
    vec3 normal = GetNormal((vec3(currentCell)+vec3(0.5))/cellDims);
    color.rgb *= PhongLighting(normal, dir);
    
    color.a = IntegrateConstantAlpha(color.a, l);
    
    return PremultiplyAlpha(color);
}

vec4 Traverse(vec3 origin, vec3 dir, float tMin, float tMax, float t0, ivec3 currentCell, ivec3 entranceFace, out float t1)
{
    vec3 entranceCoord;
    ivec3 nextCell;
    ivec3 exitFace;
    vec3 exitCoord;
    bool hasNext = true;
    float tStart = t0;
    ivec3 initialCell = currentCell;
    float unitDistanceScaled = unitDistance / length(dir * scales);
    float step = unitDistanceScaled/8;
    float stepOpacityUnit = 1/8.0;
    
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

#if 1
            if (ShouldRenderCell(currentCell))
                BlendToBack(accum, RenderCellSmartSampling(dir, entranceCoord, exitCoord, t0, tEnd, step, stepOpacityUnit));
#else
            if (ShouldRenderCell(currentCell))
                BlendToBack(accum, RenderCellConstant(dir, currentCell, t0, tEnd, unitDistanceScaled));
#endif
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
