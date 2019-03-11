#version 410 core

#include VolumeCellBase.frag

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
    
    while (hasNext) {
        if (t0 > tMax)
            break;
        
        hasNext = FindNextCell(origin, dir, t0, currentCell, entranceFace, nextCell, exitFace, exitCoord, t1);
        
        if (t0 >= tMin || (t0 <= tMin && tMin < t1)) {
            float l = (t1-t0)/unitDistanceScaled;
#if 0
            vec4 A = GetColorAtCoord(entranceCoord);
            vec4 B = GetColorAtCoord(exitCoord);
            float a = IntegrateAbsorption(A.a, B.a, l);
            vec4 color = (A+B)/2;
            // vec4 color = GetAverageColorForCoordIndex(currentCell);
            color.a = a;
#else
            vec4 color = GetAverageColorForCoordIndex(currentCell);
            color.a = IntegrateConstantAlpha(color.a, l);
#endif

            if (ShouldRenderCell(currentCell))
                BlendToBack(accum, PremultiplyAlpha(color));
        }
        
        currentCell = nextCell;
        entranceFace = -exitFace;
        entranceCoord = exitCoord;
        t0 = t1;
        i++;
        
//        if (i > 500) {
//            return vec4(1,0,1,1);
//        }
        if (accum.a > ALPHA_BREAK)
            break;
    }
    return accum;
}
