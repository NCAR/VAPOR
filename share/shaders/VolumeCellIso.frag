#pragma auto_version

#include VolumeCellBase.frag
#include VolumeIsoInclude.frag

void TestIsoSample(const vec3 hit, const vec3 dir, vec3 rayLightingNormal, float value, float dv, float ld, inout vec4 accum)
{
    if ((ld < value && dv >= value) || (ld > value && dv <= value)) {
        
        vec3 isoSampleSTR = hit/coordDimsF;
        vec4 color = GetIsoSurfaceColor(isoSampleSTR);
        vec3 normal = GetNormal(isoSampleSTR);
        
        color.rgb *= PhongLighting(normal, rayLightingNormal);
        
        BlendToBack(accum, PremultiplyAlpha(color));
    }
}

void RenderCellSmartSampling(const vec3 origin, const vec3 dir, vec3 rayLightingNormal, const vec3 entranceCoord, const vec3 exitCoord, const float tStart, const float tEnd, const float t0, const float t1, const float step, inout float ld, inout vec4 accum)
{
    vec3 hit = mix(entranceCoord, exitCoord, (tStart-t0)/(t1-t0));
    float dv = GetDataCoordinateSpace(hit);
    if (isoEnabled[0]) TestIsoSample(hit, dir, rayLightingNormal, isoValue[0], dv, ld, accum);
    if (isoEnabled[1]) TestIsoSample(hit, dir, rayLightingNormal, isoValue[1], dv, ld, accum);
    if (isoEnabled[2]) TestIsoSample(hit, dir, rayLightingNormal, isoValue[2], dv, ld, accum);
    if (isoEnabled[3]) TestIsoSample(hit, dir, rayLightingNormal, isoValue[3], dv, ld, accum);
    ld = dv;
    if (accum.a > ALPHA_BREAK) {
        gl_FragDepth = CalculateDepth(origin + dir*tStart);
        return;
    }
    
    for (float t = step * (floor(tStart/step)+1); t < tEnd; t+= step) {
        vec3 hit = mix(entranceCoord, exitCoord, (t-t0)/(t1-t0));
        float dv = GetDataCoordinateSpace(hit);
        
        if (isoEnabled[0]) TestIsoSample(hit, dir, rayLightingNormal, isoValue[0], dv, ld, accum);
        if (isoEnabled[1]) TestIsoSample(hit, dir, rayLightingNormal, isoValue[1], dv, ld, accum);
        if (isoEnabled[2]) TestIsoSample(hit, dir, rayLightingNormal, isoValue[2], dv, ld, accum);
        if (isoEnabled[3]) TestIsoSample(hit, dir, rayLightingNormal, isoValue[3], dv, ld, accum);

        ld = dv;
        
        if (accum.a > ALPHA_BREAK) {
            gl_FragDepth = CalculateDepth(origin + dir*t);
            return;
        }
    }
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
    float step = unitDistanceScaled/7/samplingRateMultiplier * GetSamplingNoise();
    if (fast)
        step = unitDistanceScaled * GetSamplingNoise();
    
    int i = 0;
    vec4 accum = vec4(0);
    float a = 0;
    
    float null;
    IntersectRayCellFace(origin, dir, -FLT_MAX, currentCell, entranceFace, null, entranceCoord);
    float ld = GetDataCoordinateSpace(entranceCoord);
    
    while (hasNext) {
        if (t0 > tMax)
            break;
        
        hasNext = FindNextCell(origin, dir, t0, currentCell, entranceFace, nextCell, exitFace, exitCoord, t1);
        
        if (t0 >= tMin || (t0 <= tMin && tMin < t1)) {
            float tEnd = min(t1, tMax);
            float tStart = max(t0, tMin);

			if (ShouldRenderCell(currentCell)) {
                RenderCellSmartSampling(origin, dir, rayLightingNormal, entranceCoord, exitCoord, tStart, tEnd, t0, t1, step, ld, accum);
            } else {
                // Leaving missing value cell
                ld = GetDataCoordinateSpace(exitCoord);
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
    return accum;
}
