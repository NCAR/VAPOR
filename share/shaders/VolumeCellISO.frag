#version 410 core

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
            float dv = GetDataCoordinateSpace(exitCoord);

			if (ShouldRenderCell(currentCell)) {
				if (isoEnabled[0]) TestIso(origin, dir, t0, t1, isoValue[0], dv, ld, entranceCoord, exitCoord, dir, accum);
				if (isoEnabled[1]) TestIso(origin, dir, t0, t1, isoValue[1], dv, ld, entranceCoord, exitCoord, dir, accum);
				if (isoEnabled[2]) TestIso(origin, dir, t0, t1, isoValue[2], dv, ld, entranceCoord, exitCoord, dir, accum);
				if (isoEnabled[3]) TestIso(origin, dir, t0, t1, isoValue[3], dv, ld, entranceCoord, exitCoord, dir, accum);
			}
            
            ld = dv;
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
