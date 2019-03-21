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
    float step = unitDistanceScaled/8;
    float stepOpacityValue = 1/8.0;
    
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
            vec4 acc2 = vec4(0);
            for (float t = step * (floor(t0/step)+1); t < t1; t+= step) {
//            for (float t = t0; t < t1; t+= step) {
                vec3 hit = mix(entranceCoord, exitCoord, (t-t0)/(t1-t0));
                vec4 color = GetColorAtCoord(hit);
                vec3 normal = GetNormalAtCoord(hit);
                color.rgb *= PhongLighting(normal, dir);
                BlendToBack(acc2, PremultiplyAlpha(color));
            }
            vec4 color = acc2;
            if (ShouldRenderCell(currentCell))
                BlendToBack(accum, color);
                
                
                
                
            
            
            
#elif 1
            vec4 acc2 = vec4(0);
            vec4 c = GetColorAtCoord(entranceCoord);
            vec3 normal = GetNormalAtCoord(entranceCoord);
            c.rgb *= PhongLighting(normal, dir);
            l = min(step - mod(t0, step), t1-t0)/step;
            c.a = IntegrateConstantAlpha(c.a, l * stepOpacityValue);
            BlendToBack(acc2, PremultiplyAlpha(c));
            
            for (float t = step * (floor(t0/step)+1); t < t1; t+= step) {
                vec3 hit = mix(entranceCoord, exitCoord, (t-t0)/(t1-t0));
                vec4 color = GetColorAtCoord(hit);
                vec3 normal = GetNormalAtCoord(hit);
                color.rgb *= PhongLighting(normal, dir);
                float l = min(step, t1-t)/step;
                color.a = IntegrateConstantAlpha(color.a, l * stepOpacityValue);
                BlendToBack(acc2, PremultiplyAlpha(color));
            }
            
            vec4 color = acc2;
            if (ShouldRenderCell(currentCell))
            BlendToBack(accum, color);
            
            
            
            
            
            
            
#else
            vec4 color = GetAverageColorForCoordIndex(currentCell);
            
            vec3 normal = GetNormal((vec3(currentCell)+vec3(0.5))/cellDims);
            color.rgb *= PhongLighting(normal, dir);
            
            color.a = IntegrateConstantAlpha(color.a, l);
            
            if (ShouldRenderCell(currentCell))
                BlendToBack(accum, PremultiplyAlpha(color));
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
