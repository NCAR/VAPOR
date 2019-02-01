#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38
#define DBL_MAX 1.7976931348623158e+308
#define DBL_MIN 2.2250738585072014e-308

#define IntersectRayBoundingBoxImplementation 2
#if IntersectRayBoundingBoxImplementation == 1
bool IntersectRayBoundingBox(vec3 o, vec3 d, vec3 boxMin, vec3 boxMax, out float t0, out float t1)
{
    t0 = 0, t1 = FLT_MAX;
    vec3 tNear = (boxMin - o) / d;
    vec3 tFar =  (boxMax - o) / d;
    
    for (int i = 0; i < 3; ++i) {
        float tNear = tNear[i];
        float tFar = tFar[i];
        
        if (tNear > tFar) {
            float temp = tNear;
            tNear = tFar;
            tFar = temp;
        }
        
        if (tNear > t0) t0 = tNear;
        if (tFar  < t1) t1 = tFar;
        if (t0 > t1) return false;
    }
    return true;
}
#elif IntersectRayBoundingBoxImplementation == 2
bool IntersectRayBoundingBox(vec3 o, vec3 d, vec3 boxMin, vec3 boxMax, out float t0, out float t1)
{
    vec3 tMin = (boxMin - o) / d;
    vec3 tMax = (boxMax - o) / d;
    vec3 bt1 = min(tMin, tMax);
    vec3 bt2 = max(tMin, tMax);
    t0 = max(max(max(bt1.x, bt1.y), bt1.z), 0);
    t1 = min(min(bt2.x, bt2.y), bt2.z);
    return t0 < t1;
}
#endif
