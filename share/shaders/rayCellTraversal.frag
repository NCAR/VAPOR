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
    return texture(data, (coordinates+vec3(0.5))/(coordDims-1)).r;
}

float GetDataForCoordIndex(ivec3 coordIndex)
{
    return GetDataCoordinateSpace(vec3(coordIndex));
}

float NormalizeData(float data)
{
    return (data - LUTMin) / (LUTMax - LUTMin);
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

void Traverse(vec3 origin, vec3 dir, float t0, ivec3 currentCell, ivec3 entranceFace, out float t1)
{
    ivec3 nextCell;
    ivec3 exitFace;
    bool hasNext = true;
    float tStart = t0;
    
    int i;
    for (i = 0; i < 10 && hasNext; i++) {
        hasNext = FindNextCell(cameraPos, dir, t0, currentCell, entranceFace, nextCell, exitFace, t1);
        
        currentCell = nextCell;
        entranceFace = -exitFace;
        t0 = t1;
    }
    
    fragColor = vec4(vec3((i)/6.0), 1);
}

bool IsRayEnteringCell(vec3 d, ivec3 cellIndex, ivec3 face)
{
    vec3 n = GetCellFaceNormal(cellIndex, face);
    return dot(d, n) < 0;
}

bool FindInitialCell(vec3 origin, vec3 dir, float t0, out ivec3 cellIndex, out ivec3 entranceFace, out float t1)
{
    for (int y = 0; y < cellDims[1]; y++) {
        for (int x = 0; x < cellDims[0]; x++) {
            if (IntersectRayCellFace(cameraPos, dir, ivec3(x, y, 0), F_DOWN, t1)) {
                cellIndex = ivec3(x,y,0);
                entranceFace = F_DOWN;
                
                if (IsRayEnteringCell(dir, cellIndex, entranceFace))
                    return true;
            }
        }
    }
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
            // fragColor = vec4(1,0,0,1);
            // return;
            Traverse(cameraPos, dir, t0, initialCell, entranceFace, t1);
        }
        return;
    }
        
    if (accum.a < 0.1)
        discard;
}
