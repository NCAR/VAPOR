#version 330 core

#include RayMath.frag

#define EPSILON 1.19e-07

uniform mat4 MVP;
// uniform vec2 resolution;
uniform vec3 cameraPos;
uniform vec3 dataBoundsMin;
uniform vec3 dataBoundsMax;
uniform float LUTMin;
uniform float LUTMax;

uniform ivec3 coordDims;
uniform float unitDistance;

vec3 coordDimsF = vec3(coordDims);
ivec3 cellDims = coordDims - 1;

uniform sampler3D data;
uniform sampler1D LUT;
uniform sampler3D coords;

in vec2 ST;

out vec4 fragColor;

#define F_LEFT  ivec3(-1, 0, 0)
#define F_RIGHT ivec3( 1, 0, 0)
#define F_UP    ivec3( 0, 0, 1)
#define F_DOWN  ivec3( 0, 0,-1)
#define F_FRONT ivec3( 0,-1, 0)
#define F_BACK  ivec3( 0, 1, 0)

ivec3 GetFaceFromFaceIndex(int i)
{
    if (i == 0) return F_LEFT;
    if (i == 1) return F_RIGHT;
    if (i == 2) return F_UP;
    if (i == 3) return F_DOWN;
    if (i == 4) return F_FRONT;
    if (i == 5) return F_BACK;
}

vec4 DEBUG_GetFaceColor(ivec3 face)
{
    if (face == F_LEFT)  return vec4(0,0,1,1); // Blue
    if (face == F_RIGHT) return vec4(0,1,0,1); // Green
    if (face == F_UP)    return vec4(0,1,1,1); // Cyan
    if (face == F_DOWN)  return vec4(1,0,0,1); // Red
    if (face == F_FRONT) return vec4(1,0,1,1); // Purple
    if (face == F_BACK)  return vec4(1,1,0,1); // Yellow
}

void GetFaceCoordinateIndices(ivec3 cell, ivec3 face, out ivec3 i0, out ivec3 i1, out ivec3 i2, out ivec3 i3)
{
    // CCW
    if (face == F_DOWN) {
        i0 = cell + ivec3(0, 0, 0);
        i1 = cell + ivec3(0, 1, 0);
        i2 = cell + ivec3(1, 1, 0);
        i3 = cell + ivec3(1, 0, 0);
    }
    else if (face == F_UP) {
        i0 = cell + ivec3(0, 0, 1);
        i1 = cell + ivec3(1, 0, 1);
        i2 = cell + ivec3(1, 1, 1);
        i3 = cell + ivec3(0, 1, 1);
    }
    else if (face == F_LEFT) {
        i0 = cell + ivec3(0, 0, 0);
        i1 = cell + ivec3(0, 0, 1);
        i2 = cell + ivec3(0, 1, 1);
        i3 = cell + ivec3(0, 1, 0);
    }
    else if (face == F_RIGHT) {
        i0 = cell + ivec3(1, 0, 0);
        i1 = cell + ivec3(1, 1, 0);
        i2 = cell + ivec3(1, 1, 1);
        i3 = cell + ivec3(1, 0, 1);
    }
    else if (face == F_FRONT) {
        i0 = cell + ivec3(0, 0, 0);
        i1 = cell + ivec3(1, 0, 0);
        i2 = cell + ivec3(1, 0, 1);
        i3 = cell + ivec3(0, 0, 1);
    }
    else if (face == F_BACK) {
        i0 = cell + ivec3(0, 1, 0);
        i1 = cell + ivec3(0, 1, 1);
        i2 = cell + ivec3(1, 1, 1);
        i3 = cell + ivec3(1, 1, 0);
    }
}

void GetFaceVertices(ivec3 cellIndex, ivec3 face, out vec3 v0, out vec3 v1, out vec3 v2, out vec3 v3)
{
    ivec3 i0, i1, i2, i3;
    GetFaceCoordinateIndices(cellIndex, face, i0, i1, i2, i3);
    v0 = texture(coords, i0/coordDimsF).xyz;
    v1 = texture(coords, i1/coordDimsF).xyz;
    v2 = texture(coords, i2/coordDimsF).xyz;
    v3 = texture(coords, i3/coordDimsF).xyz;
}

float GetDataCoordinateSpace(vec3 coordinates)
{
    return texture(data, (coordinates)/(coordDims-1)).r;
}

float GetDataForCoordIndex(ivec3 coordIndex)
{
    return GetDataCoordinateSpace(vec3(coordIndex)+vec3(0.5));
}

float NormalizeData(float data)
{
    return (data - LUTMin) / (LUTMax - LUTMin);
}

vec4 GetColorForNormalizedData(float normalizedData)
{
    return texture(LUT, normalizedData);
}

vec4 GetAverageColorForCoordIndex(ivec3 coordIndex)
{
    return GetColorForNormalizedData(NormalizeData(GetDataForCoordIndex(coordIndex)));
}

bool IntersectRayCellFace(vec3 o, vec3 d, ivec3 cellIndex, ivec3 face, out float t)
{
    vec3 v0, v1, v2, v3;
    GetFaceVertices(cellIndex, face, v0, v1, v2, v3);
    return IntersectRayQuad(o, d, v0, v1, v2, v3, t);
}

vec3 GetTriangleNormal(vec3 v0, vec3 v1, vec3 v2)
{
    return cross(v1-v0, v2-v0);
}

vec3 GetCellFaceNormal(ivec3 cellIndex, ivec3 face)
{
    vec3 v0, v1, v2, v3;
    GetFaceVertices(cellIndex, face, v0, v1, v2, v3);
    
    return (GetTriangleNormal(v0, v1, v2) + GetTriangleNormal(v0, v2, v3)) / 2.0f;
}

bool FindCellExit(vec3 origin, vec3 dir, float t0, ivec3 currentCell, ivec3 entranceFace, out ivec3 exitFace, out float t1)
{
    for (int i = 0; i < 6; i++) {
        ivec3 testFace = GetFaceFromFaceIndex(i);
        
        if (testFace == entranceFace)
            continue;
            
        if (IntersectRayCellFace(origin, dir, currentCell, testFace, t1)) {
            if (t1 - t0 > EPSILON) {
                exitFace = testFace;
                return true;
            }
        }
    }
    return false;
}

// Recommended to provide entranceFace to guarentee no self-intersection
bool FindCellExit(vec3 origin, vec3 dir, float t0, ivec3 currentCell, out ivec3 exitFace, out float t1)
{
    return FindCellExit(origin, dir, t0, currentCell, ivec3(0), exitFace, t1);
}

bool FindNextCell(vec3 origin, vec3 dir, float t0, ivec3 currentCell, ivec3 entranceFace, out ivec3 nextCell, out ivec3 exitFace, out float t1)
{
    if (FindCellExit(origin, dir, t0, currentCell, entranceFace, exitFace, t1)) {
        nextCell = currentCell + exitFace;
        if (any(lessThan(nextCell, ivec3(0))) || any(greaterThanEqual(nextCell, cellDims)))
            return false;
        return true;
    }
    return false;
}

bool FindNextCell(vec3 origin, vec3 dir, float t0, ivec3 currentCell, out ivec3 nextCell, out float t1)
{
    ivec3 null;
    return FindNextCell(origin, dir, t0, currentCell, ivec3(0), nextCell, null, t1);
}

void BlendToBack(inout vec4 accum, vec4 color)
{
    accum.rgb += color.rgb * color.a * (1-accum.a);
    accum.a += color.a * (1-accum.a);
}

// In the above descrete blending equation we have (c = color.a):
//
// a_n+1 = a_n + c_n * (1-a_n)
//
// This can be rearranged to:
//
// a_n+1 = a_n(1-c_n)+c_n
//
// Integrating we get:
//
//              f n
//             -|  c_n
// a_n = 1 - e^ j 0
//
// And the for a constant c, the above integral evaluates to the linear function below
//
float IntegrateConstantAlpha(float a, float distance)
{
    return 1 - exp(-a * distance);
}

void Traverse(vec3 origin, vec3 dir, float t0, ivec3 currentCell, ivec3 entranceFace, out float t1)
{
    ivec3 nextCell;
    ivec3 exitFace;
    bool hasNext = true;
    float tStart = t0;
    
    int i = 0;
    vec4 accum = vec4(0);
    float a = 0;
    
    while (hasNext) {
        hasNext = FindNextCell(origin, dir, t0, currentCell, entranceFace, nextCell, exitFace, t1);
        
        vec4 color = GetAverageColorForCoordIndex(currentCell);
        
        color.a = IntegrateConstantAlpha(color.a, (t1-t0)/unitDistance);
        BlendToBack(accum, color);
        
        currentCell = nextCell;
        entranceFace = -exitFace;
        t0 = t1;
        i++;
        
        // if (i > 1) break;
        if (accum.a > 0.995)
            break;
    }
    fragColor = accum;
}

bool IsRayEnteringCell(vec3 d, ivec3 cellIndex, ivec3 face)
{
    vec3 n = GetCellFaceNormal(cellIndex, face);
    return dot(d, n) < 0;
}

bool SearchSideForInitialCell(vec3 origin, vec3 dir, float t0, ivec3 side, int fastDim, int slowDim, out ivec3 cellIndex, out ivec3 entranceFace, out float t1)
{
    ivec3 index = (side+1)/2 * (cellDims-1);
    
    // Perfomance improvement possible
    for (index[slowDim] = 0; index[slowDim] < cellDims[slowDim]; index[slowDim]++) {
        for (index[fastDim] = 0; index[fastDim] < cellDims[fastDim]; index[fastDim]++) {
            if (IntersectRayCellFace(origin, dir, index, side, t1)) {
                if (IsRayEnteringCell(dir, index, side)) {
                    cellIndex = index;
                    entranceFace = side;
                    return true;
                }
            }
        }
    }
    return false;
}

bool FindInitialCell(vec3 origin, vec3 dir, float t0, out ivec3 cellIndex, out ivec3 entranceFace, out float t1)
{
    
    if (SearchSideForInitialCell(origin, dir, t0, F_DOWN, 0, 1, cellIndex, entranceFace, t1)) return true;
    if (SearchSideForInitialCell(origin, dir, t0, F_UP, 0, 1, cellIndex, entranceFace, t1)) return true;
    if (SearchSideForInitialCell(origin, dir, t0, F_LEFT, 1, 2, cellIndex, entranceFace, t1)) return true;
    if (SearchSideForInitialCell(origin, dir, t0, F_RIGHT, 1, 2, cellIndex, entranceFace, t1)) return true;
    if (SearchSideForInitialCell(origin, dir, t0, F_FRONT, 0, 2, cellIndex, entranceFace, t1)) return true;
    if (SearchSideForInitialCell(origin, dir, t0, F_BACK, 0, 2, cellIndex, entranceFace, t1)) return true;
    return false;
}


void main(void)
{
    vec2 screen = ST*2-1;
    vec4 world = inverse(MVP) * vec4(screen, 1, 1);
    world /= world.w;
    vec3 dir = normalize(world.xyz - cameraPos);
    
    vec4 accum = vec4(0);
    float t0, t1, tp;
    
    bool intersectBox = IntersectRayBoundingBox(cameraPos, dir, dataBoundsMin, dataBoundsMax, t0, t1);
    
    if (intersectBox) {
        
        ivec3 initialCell;
        ivec3 entranceFace;
        float t0;
        float t1;
        if (FindInitialCell(cameraPos, dir, 0, initialCell, entranceFace, t0)) {
            fragColor = vec4(vec3(t0/5),1);
            // return;
            
            Traverse(cameraPos, dir, t0, initialCell, entranceFace, t1);
        } else
            discard;
        return;
    }
        
    if (accum.a < 0.1)
        discard;
}
