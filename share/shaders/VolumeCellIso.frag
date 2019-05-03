#pragma auto_version

#include VolumeCellBase.frag
#include VolumeIsoInclude.frag

void TestIso(vec3 o, vec3 d, float t0, float t1, float value, float dv, float ld, vec3 entranceCoord, vec3 exitCoord, vec3 dir, inout vec4 accum)
{
    if ((ld < value && dv >= value) || (ld > value && dv <= value)) {
        vec3 isoCoord = mix(entranceCoord, exitCoord, (value-ld)/(dv-ld));
		vec3 isoSampleSTR = isoCoord/coordDimsF;
        
        vec4 color = GetIsoSurfaceColor(isoSampleSTR);
        vec3 normal = GetNormal(isoSampleSTR);
        
        color.rgb *= PhongLighting(normal, dir);
        
        BlendToBack(accum, PremultiplyAlpha(color));
        
        if (color.a > ALPHA_BREAK) {
            float t = mix(t0, t1, (value-ld)/(dv-ld));
            vec3 hit = o + d*t;
            gl_FragDepth = CalculateDepth(hit);
        }
    }
}

void TestIsoSample(const vec3 hit, const vec3 dir, float value, float dv, float ld, inout vec4 accum)
{
    if ((ld < value && dv >= value) || (ld > value && dv <= value)) {
        
        vec3 isoSampleSTR = hit/coordDimsF;
        vec4 color = GetIsoSurfaceColor(isoSampleSTR);
        vec3 normal = GetNormal(isoSampleSTR);
        
        color.rgb *= PhongLighting(normal, dir);
        
        BlendToBack(accum, PremultiplyAlpha(color));
    }
}

void RenderCellSmartSampling(const vec3 dir, const vec3 entranceCoord, const vec3 exitCoord, const float tStart, const float tEnd, const float t0, const float t1, const float step, inout float ld, inout vec4 accum)
{
    vec3 hit = mix(entranceCoord, exitCoord, (tStart-t0)/(t1-t0));
    float dv = GetDataCoordinateSpace(hit);
    if (isoEnabled[0]) TestIsoSample(hit, dir, isoValue[0], dv, ld, accum);
    if (isoEnabled[1]) TestIsoSample(hit, dir, isoValue[1], dv, ld, accum);
    if (isoEnabled[2]) TestIsoSample(hit, dir, isoValue[2], dv, ld, accum);
    if (isoEnabled[3]) TestIsoSample(hit, dir, isoValue[3], dv, ld, accum);
    ld = dv;
    
    for (float t = step * (floor(tStart/step)+1); t < tEnd; t+= step) {
        vec3 hit = mix(entranceCoord, exitCoord, (t-t0)/(t1-t0));
        float dv = GetDataCoordinateSpace(hit);
        
        if (isoEnabled[0]) TestIsoSample(hit, dir, isoValue[0], dv, ld, accum);
        if (isoEnabled[1]) TestIsoSample(hit, dir, isoValue[1], dv, ld, accum);
        if (isoEnabled[2]) TestIsoSample(hit, dir, isoValue[2], dv, ld, accum);
        if (isoEnabled[3]) TestIsoSample(hit, dir, isoValue[3], dv, ld, accum);

        ld = dv;
    }
}

vec4 Traverse(vec3 origin, vec3 dir, float tMin, float tMax, float t0, ivec3 currentCell, ivec3 entranceFace, OUT float t1)
{
    vec3 entranceCoord;
    ivec3 nextCell;
    ivec3 exitFace;
    vec3 exitCoord;
    bool hasNext = true;
    float tStart = t0;
    ivec3 initialCell = currentCell;
    float unitDistanceScaled = unitDistance / length(dir * scales);
    float step = unitDistanceScaled/7;
    if (fast)
        step = unitDistanceScaled;
    
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
#if 1
            float tEnd = min(t1, tMax);
            float tStart = max(t0, tMin);

			if (ShouldRenderCell(currentCell)) {
                RenderCellSmartSampling(dir, entranceCoord, exitCoord, tStart, tEnd, t0, t1, step, ld, accum);
			}
#else
            float dv = GetDataCoordinateSpace(exitCoord);
            bool increasing = dv-ld > 0;
            if (ShouldRenderCell(currentCell)) {
                bool e[4] = isoEnabled;
                float v[4] = isoValue;
                
                /*
                for (int i = 0; i < 3; i++) {
                    for (int j = i+1; j < 4; j++) {
                        if ((increasing && v[i] > v[j]) || (!increasing && v[i] < v[j])) {
                            float tv = v[i];
                            bool te = e[i];
                            v[i] = v[j];
                            e[i] = e[j];
                            v[j] = tv;
                            e[j] = te;
                        }
                    }
                }
                 */

                if (e[0]) TestIso(origin, dir, t0, t1, v[0], dv, ld, entranceCoord, exitCoord, dir, accum);
                if (e[1]) TestIso(origin, dir, t0, t1, v[1], dv, ld, entranceCoord, exitCoord, dir, accum);
                if (e[2]) TestIso(origin, dir, t0, t1, v[2], dv, ld, entranceCoord, exitCoord, dir, accum);
                if (e[3]) TestIso(origin, dir, t0, t1, v[3], dv, ld, entranceCoord, exitCoord, dir, accum);
            }
            ld = dv;
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
    return accum;
}
