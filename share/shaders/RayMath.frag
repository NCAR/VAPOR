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
    return t0 <= t1;
}
#endif

bool IntersectRayPlane(vec3 o, vec3 d, vec3 v0, vec3 n, out float t)
{
    float denom = dot(n, d);
    
    if (abs(denom) > 1e-6) {
        t = dot(v0 - o, n) / denom;
        return t >= 0;
    }
    return false;
}

bool IntersectRayTriangle(vec3 o, vec3 d, vec3 v0, vec3 v1, vec3 v2, out float t)
{
    vec3 n = cross(v1-v0,v2-v0);
    
    if (IntersectRayPlane(o, d, v0, n, t)) {
        vec3 P = o + d * t;
        
        vec3 edge0 = v1-v0;
        vec3 vp0 = P - v0;
        vec3 C0 = cross(edge0, vp0);
        if (dot(n, C0) < 0) return false;
        
        vec3 edge1 = v2-v1;
        vec3 vp1 = P - v1;
        vec3 C1 = cross(edge1, vp1);
        if (dot(n, C1) < 0) return false;
        
        vec3 edge2 = v0 - v2;
        vec3 vp2 = P - v2;
        vec3 C2 = cross(edge2, vp2);
        if (dot(n, C2) < 0) return false;
        
        return true;
    }
    return false;
}

bool IntersectRayQuad(vec3 o, vec3 d, vec3 v0, vec3 v1, vec3 v2, vec3 v3, out float t)
{
    if (IntersectRayTriangle(o, d, v0, v1, v2, t)) return true;
    if (IntersectRayTriangle(o, d, v2, v3, v0, t)) return true;
    return false;
}
