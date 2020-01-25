#pragma auto_version

#include VolumeCellBase.frag
#include VolumeIsoInclude.frag

void TestIsoSample(const vec3 rayLightingNormal, const float isoValue, const float dv, const float ld, const float t, const float lt, const float t0, const float t1, const vec3 entranceCoord, const vec3 exitCoord, inout vec4 accum)
{
    if ((ld < isoValue && dv >= isoValue) || (ld > isoValue && dv <= isoValue)) {
        
        float ti = mix(lt, t, (isoValue-ld)/(dv-ld));
        vec3 hit = mix(entranceCoord, exitCoord, (ti-t0)/(t1-t0));
        
        vec3 isoSampleSTR = hit/coordDimsF;
        vec4 color = GetIsoSurfaceColor(isoSampleSTR);
        vec3 normal = GetNormal(isoSampleSTR);
        
        color.rgb *= PhongLighting(normal, rayLightingNormal);
        
        BlendToBack(accum, PremultiplyAlpha(color));
        
        if (accum.a > ALPHA_BREAK)
            gl_FragDepth = CalculateDepth(hit);
    }
}

void SampleCell(const vec3 rayLightingNormal, const vec3 entranceCoord, const vec3 exitCoord, const float t0, const float t1, const float ts, const float te, inout vec4 accum)
{
    if (accum.a > ALPHA_BREAK)
        return;
    
    vec3 hitS = mix(entranceCoord, exitCoord, (ts-t0)/(t1-t0));
    float ds = GetDataCoordinateSpace(hitS);
    
    vec3 hitE = mix(entranceCoord, exitCoord, (te-t0)/(t1-t0));
    float de = GetDataCoordinateSpace(hitE);
    
    if (isoEnabled[0]) TestIsoSample(rayLightingNormal, isoValue[0], de, ds, te, ts, t0, t1, entranceCoord, exitCoord, accum);
    if (isoEnabled[1]) TestIsoSample(rayLightingNormal, isoValue[1], de, ds, te, ts, t0, t1, entranceCoord, exitCoord, accum);
    if (isoEnabled[2]) TestIsoSample(rayLightingNormal, isoValue[2], de, ds, te, ts, t0, t1, entranceCoord, exitCoord, accum);
    if (isoEnabled[3]) TestIsoSample(rayLightingNormal, isoValue[3], de, ds, te, ts, t0, t1, entranceCoord, exitCoord, accum);
}

vec4 Traverse(vec3 origin, vec3 dir, vec3 rayLightingNormal, float tMin, float tMax, float t0, ivec3 currentCell, ivec3 entranceFace, OUT float t1)
{
    vec3 entranceCoord;
    ivec3 nextCell;
    ivec3 exitFace;
    vec3 exitCoord;
    bool hasNext = true;
    ivec3 initialCell = currentCell;
    float unitDistanceScaled = unitDistance / length(dir * scales);
    float step = unitDistanceScaled/7/samplingRateMultiplier * GetSamplingNoise();
    if (fast)
        step = unitDistanceScaled * GetSamplingNoise();
    
    int i = 0;
    vec4 accum = vec4(0);
    float a = 0;
    
    float lt;
    IntersectRayCellFace(origin, dir, -FLT_MAX, currentCell, entranceFace, lt, entranceCoord);
    FindNextCell(origin, dir, t0, currentCell, entranceFace, nextCell, exitFace, exitCoord, t1);
    
    while (hasNext) {
        if (t0 > tMax)
            break;
        
        hasNext = FindNextCell(origin, dir, t0, currentCell, entranceFace, nextCell, exitFace, exitCoord, t1);
        
        if (t0 >= tMin || (t0 <= tMin && tMin < t1)) {
            float tEnd = min(t1, tMax);
            float tStart = max(t0, tMin);

			if (ShouldRenderCell(currentCell))
                SampleCell(rayLightingNormal, entranceCoord, exitCoord, t0, t1, tStart, tEnd, accum);
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
