#include VolumeBase.frag

// This file implements the cell traversal and provides a framework
// for rendering a curvilinear grid.
// It defines a Traverse function which needs to be
// overridden that does the rendering

uniform ivec3 coordDims;
uniform float unitDistance;
uniform float unitOpacityScalar;
uniform int BBLevels;

vec3 coordDimsF = vec3(coordDims);
ivec3 cellDims = coordDims - 1;

uniform sampler3D coords;
uniform sampler2DArray boxMins;
uniform sampler2DArray boxMaxs;
uniform isampler2D levelDims;

#define FI_LEFT  0
#define FI_RIGHT 1
#define FI_UP    2
#define FI_DOWN  3
#define FI_FRONT 4
#define FI_BACK  5
#define FI_NONE 99

#define F_LEFT  ivec3(-1, 0, 0)
#define F_RIGHT ivec3( 1, 0, 0)
#define F_UP    ivec3( 0, 0, 1)
#define F_DOWN  ivec3( 0, 0,-1)
#define F_FRONT ivec3( 0,-1, 0)
#define F_BACK  ivec3( 0, 1, 0)
#define F_NONE  ivec3(-1,-1,-1)

#define CI_LEFT  0
#define CI_RIGHT 1
#define CI_UP    2
#define CI_DOWN  3
#define CI_FRONT 4
#define CI_BACK  5
#define CI_LEFT_FRONT  6
#define CI_LEFT_BACK   7
#define CI_RIGHT_FRONT 8
#define CI_RIGHT_BACK  9
#define CI_UP_FRONT   10
#define CI_UP_BACK    11
#define CI_DOWN_FRONT  12
#define CI_DOWN_BACK   13
#define CI_UP_LEFT    14
#define CI_UP_RIGHT   15
#define CI_DOWN_LEFT   16
#define CI_DOWN_RIGHT  17
#define CI_UP_LEFT_FRONT   18
#define CI_UP_LEFT_BACK    19
#define CI_UP_RIGHT_FRONT  20
#define CI_UP_RIGHT_BACK   21
#define CI_DOWN_LEFT_FRONT  22
#define CI_DOWN_LEFT_BACK   23
#define CI_DOWN_RIGHT_FRONT 24
#define CI_DOWN_RIGHT_BACK  25
#define CI_NONE  99

#define C_LEFT  ivec3(-1, 0, 0)
#define C_RIGHT ivec3( 1, 0, 0)
#define C_UP    ivec3( 0, 0, 1)
#define C_DOWN  ivec3( 0, 0,-1)
#define C_FRONT ivec3( 0,-1, 0)
#define C_BACK  ivec3( 0, 1, 0)
#define C_LEFT_FRONT  (C_LEFT+C_FRONT)
#define C_LEFT_BACK   (C_LEFT+C_BACK)
#define C_RIGHT_FRONT (C_RIGHT+C_FRONT)
#define C_RIGHT_BACK  (C_RIGHT+C_BACK)
#define C_UP_FRONT    (C_UP+C_FRONT)
#define C_UP_BACK     (C_UP+C_BACK)
#define C_DOWN_FRONT  (C_DOWN+C_FRONT)
#define C_DOWN_BACK   (C_DOWN+C_BACK)
#define C_UP_LEFT     (C_UP+C_LEFT)
#define C_UP_RIGHT    (C_UP+C_RIGHT)
#define C_DOWN_LEFT   (C_DOWN+C_LEFT)
#define C_DOWN_RIGHT  (C_DOWN+C_RIGHT)
#define C_UP_LEFT_FRONT    (C_UP+C_LEFT+C_FRONT)
#define C_UP_LEFT_BACK     (C_UP+C_LEFT+C_BACK)
#define C_UP_RIGHT_FRONT   (C_UP+C_RIGHT+C_FRONT)
#define C_UP_RIGHT_BACK    (C_UP+C_RIGHT+C_BACK)
#define C_DOWN_LEFT_FRONT  (C_DOWN+C_LEFT+C_FRONT)
#define C_DOWN_LEFT_BACK   (C_DOWN+C_LEFT+C_BACK)
#define C_DOWN_RIGHT_FRONT (C_DOWN+C_RIGHT+C_FRONT)
#define C_DOWN_RIGHT_BACK  (C_DOWN+C_RIGHT+C_BACK)
#define C_NONE  ivec3(-1,-1,-1)

// face   fast  slow
// DOWN    0     1
// UP      0     1
// LEFT    1     2
// RIGHT   1     2
// FRONT   0     2
// BACK    0     2

int GetFastDimForFaceIndex(int i)
{
    if (i == FI_LEFT || i == FI_RIGHT)
        return 1;
    return 0;
}

int GetSlowDimForFaceIndex(int i)
{
    if (i == FI_DOWN || i == FI_UP)
        return 1;
    return 2;
}

ivec3 GetFaceFromFaceIndex(int i)
{
    if (i == 0) return F_LEFT;
    if (i == 1) return F_RIGHT;
    if (i == 2) return F_UP;
    if (i == 3) return F_DOWN;
    if (i == 4) return F_FRONT;
    if (i == 5) return F_BACK;
}

ivec3 GetNeighborFromNeighborCellIndex(const int i)
{
    if (i == CI_LEFT)             return C_LEFT;
    if (i == CI_RIGHT)            return C_RIGHT;
    if (i == CI_UP)               return C_UP;
    if (i == CI_DOWN)             return C_DOWN;
    if (i == CI_FRONT)            return C_FRONT;
    if (i == CI_BACK)             return C_BACK;
    if (i == CI_LEFT_FRONT)       return C_LEFT_FRONT;
    if (i == CI_LEFT_BACK)        return C_LEFT_BACK;
    if (i == CI_RIGHT_FRONT)      return C_RIGHT_FRONT;
    if (i == CI_RIGHT_BACK)       return C_RIGHT_BACK;
    if (i == CI_UP_FRONT)         return C_UP_FRONT;
    if (i == CI_UP_BACK)          return C_UP_BACK;
    if (i == CI_DOWN_FRONT)       return C_DOWN_FRONT;
    if (i == CI_DOWN_BACK)        return C_DOWN_BACK;
    if (i == CI_UP_LEFT)          return C_UP_LEFT;
    if (i == CI_UP_RIGHT)         return C_UP_RIGHT;
    if (i == CI_DOWN_LEFT)        return C_DOWN_LEFT;
    if (i == CI_DOWN_RIGHT)       return C_DOWN_RIGHT;
    if (i == CI_UP_LEFT_FRONT)    return C_UP_LEFT_FRONT;
    if (i == CI_UP_LEFT_BACK)     return C_UP_LEFT_BACK;
    if (i == CI_UP_RIGHT_FRONT)   return C_UP_RIGHT_FRONT;
    if (i == CI_UP_RIGHT_BACK)    return C_UP_RIGHT_BACK;
    if (i == CI_DOWN_LEFT_FRONT)  return C_DOWN_LEFT_FRONT;
    if (i == CI_DOWN_LEFT_BACK)   return C_DOWN_LEFT_BACK;
    if (i == CI_DOWN_RIGHT_FRONT) return C_DOWN_RIGHT_FRONT;
    if (i == CI_DOWN_RIGHT_BACK)  return C_DOWN_RIGHT_BACK;
}

int GetFaceIndexFromFace(ivec3 face)
{
    if (face == F_LEFT)  return FI_LEFT;
    if (face == F_RIGHT) return FI_RIGHT;
    if (face == F_UP)    return FI_UP;
    if (face == F_DOWN)  return FI_DOWN;
    if (face == F_FRONT) return FI_FRONT;
    if (face == F_BACK)  return FI_BACK;
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

void GetFaceCoordinateIndices(ivec3 cell, ivec3 face, OUT ivec3 i0, OUT ivec3 i1, OUT ivec3 i2, OUT ivec3 i3)
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

void GetFaceCoordsAndVertices(ivec3 cellIndex, ivec3 face, OUT ivec3 i0, OUT ivec3 i1, OUT ivec3 i2, OUT ivec3 i3, OUT vec3 v0, OUT vec3 v1, OUT vec3 v2, OUT vec3 v3)
{
    GetFaceCoordinateIndices(cellIndex, face, i0, i1, i2, i3);
    v0 = texelFetch(coords, i0, 0).xyz;
    v1 = texelFetch(coords, i1, 0).xyz;
    v2 = texelFetch(coords, i2, 0).xyz;
    v3 = texelFetch(coords, i3, 0).xyz;
}

void GetFaceVertices(ivec3 cellIndex, ivec3 face, OUT vec3 v0, OUT vec3 v1, OUT vec3 v2, OUT vec3 v3)
{
    ivec3 i0, i1, i2, i3;
    GetFaceCoordsAndVertices(cellIndex, face, i0, i1, i2, i3, v0, v1, v2, v3);
}

float GetDataCoordinateSpace(vec3 coordinates)
{
    return texture(data, coordinates/coordDimsF).r;
}

float GetDataForCoordIndex(ivec3 coordIndex)
{
    vec3 coord = vec3(coordIndex)+vec3(0.5);
    return texture(data, (coord)/(coordDims-1)).r;
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

vec4 GetColorAtCoord(vec3 coord)
{
    // return GetColorForNormalizedData(NormalizeData(GetDataCoordinateSpace(coord)));
    return GetColorForNormalizedCoord(coord/coordDimsF);
}

vec3 GetNormalAtCoord(vec3 coord)
{
    return GetNormal(coord/coordDimsF);
}

bool DoesCellHaveMissingData(ivec3 cellCoord)
{
    vec3 coord = vec3(cellCoord)+vec3(0.5);
    return DoesSampleHaveMissingData((coord)/(coordDims-1));
}

bool IntersectRayCellFace(vec3 o, vec3 d, float rt0, ivec3 cellIndex, ivec3 face, OUT float t, OUT vec3 dataCoordinate)
{
    ivec3 i0, i1, i2, i3;
    vec3 v0, v1, v2, v3;
    GetFaceCoordsAndVertices(cellIndex, face, i0, i1, i2, i3, v0, v1, v2, v3);
    
    vec4 weights;
    if (IntersectRayQuad(o, d, rt0, v0, v1, v2, v3, t, weights)) {
        dataCoordinate = (weights.x*i0 + weights.y*i1 + weights.z*i2 + weights.w*i3 + vec3(0.5));
        return true;
    }
    return false;
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

bool FindCellExit(vec3 origin, vec3 dir, float t0, ivec3 currentCell, ivec3 entranceFace, bool allowThinCells, OUT ivec3 exitFace, OUT vec3 exitCoord, OUT float t1)
{
    exitFace = F_NONE;
    for (int i = 0; i < 6; i++) {
        ivec3 testFace = GetFaceFromFaceIndex(i);
        
        if (testFace == entranceFace)
            continue;
            
        if (IntersectRayCellFace(origin, dir, t0, currentCell, testFace, t1, exitCoord)) {
            // There are cases where very thin cells will result in the same t
            if (t1 - t0 > EPSILON) {
                exitFace = testFace;
                return true;
            } else if (allowThinCells && (t1 - t0 >= 0)) {
                exitFace = testFace;
            }
        }
    }
    if (exitFace != F_NONE)
        return true;
    return false;
}

bool FindCellExit(vec3 origin, vec3 dir, float t0, ivec3 currentCell, ivec3 entranceFace, OUT ivec3 exitFace, OUT vec3 exitCoord, OUT float t1)
{
    return FindCellExit(origin, dir, t0, currentCell, entranceFace, true, exitFace, exitCoord, t1);
}

bool IsCellInBounds(ivec3 cellIndex)
{
    return !(any(lessThan(cellIndex, ivec3(0))) || any(greaterThanEqual(cellIndex, cellDims)));
}

bool SearchNeighboringCells(vec3 origin, vec3 dir, float t0, ivec3 currentCell, OUT ivec3 nextCell, OUT ivec3 exitFace, OUT vec3 exitCoord, OUT float t1)
{
    for (int sideID = 0; sideID < 26; sideID++) {
        ivec3 side = GetNeighborFromNeighborCellIndex(sideID);
        ivec3 testCell = currentCell + side;
        if (IsCellInBounds(testCell)) {
            if (FindCellExit(origin, dir, t0, testCell, -side, false, exitFace, exitCoord, t1)) {
                nextCell = testCell + exitFace;
                return true;
            }
        }
    }
    return false;
}

bool FindNextCell(vec3 origin, vec3 dir, float t0, ivec3 currentCell, ivec3 entranceFace, OUT ivec3 nextCell, OUT ivec3 exitFace, OUT vec3 exitCoord, OUT float t1)
{
    if (FindCellExit(origin, dir, t0, currentCell, entranceFace, exitFace, exitCoord, t1)) {
        nextCell = currentCell + exitFace;
        if (!IsCellInBounds(nextCell))
            return false;
        return true;
    } else {
        return SearchNeighboringCells(origin, dir, t0, currentCell, nextCell, exitFace, exitCoord, t1);
    }
}

float IntegrateConstantAlpha(float a, float distance)
{
    return 1 - exp(-a * distance);
}

// Only accurate for tetrahedra
// Its an approximation for cubes
float IntegrateAbsorption(float a, float b, float distance)
{
    return 1 - exp(-distance * (a+b)/2);
}

bool ShouldRenderCell(const ivec3 cellIndex)
{
    if (hasMissingData)
        if (DoesCellHaveMissingData(cellIndex))
            return false;
    return true;
}

bool IsRayEnteringCell(vec3 d, ivec3 cellIndex, ivec3 face)
{
    vec3 n = GetCellFaceNormal(cellIndex, face);
    return dot(d, n) < 0;
}

void GetSideCellBBox(ivec3 cellIndex, int sideID, int fastDim, int slowDim, OUT vec3 bmin, OUT vec3 bmax)
{
    ivec3 index = ivec3(cellIndex[fastDim], cellIndex[slowDim], sideID);
    bmin = texelFetch(boxMins, index, 0).rgb;
    bmax = texelFetch(boxMaxs, index, 0).rgb;
}

bool IntersectRaySideCellBBox(vec3 origin, vec3 dir, float rt0, ivec3 cellIndex, int sideID, int fastDim, int slowDim)
{
    vec3 bmin, bmax;
    float t0, t1;
    GetSideCellBBox(cellIndex, sideID, fastDim, slowDim, bmin, bmax);
    if (IntersectRayBoundingBox(origin, dir, rt0, bmin, bmax, t0, t1)) {
        return true;
    }
    return false;
}

void GetSideCellBBoxDirect(int x, int y, int sideID, int level, OUT vec3 bmin, OUT vec3 bmax)
{
    ivec3 index = ivec3(x, y, sideID);
    bmin = texelFetch(boxMins, index, level).rgb;
    bmax = texelFetch(boxMaxs, index, level).rgb;
}

bool IntersectRaySideCellBBoxDirect(vec3 origin, vec3 dir, float rt0, int x, int y, int sideID, int level)
{
    vec3 bmin, bmax;
    float t0, t1;
    GetSideCellBBoxDirect(x, y, sideID, level, bmin, bmax);
    if (IntersectRayBoundingBox(origin, dir, rt0, bmin, bmax, t0, t1)) {
        return true;
    }
    return false;
}

ivec2 GetBBoxArrayDimensions(int sideID, int level)
{
    return texelFetch(levelDims, ivec2(sideID, level), 0).rg;
}

bool IsFaceThatPassedBBAnInitialCell(vec3 origin, vec3 dir, float t0, ivec3 index, ivec3 side, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
    float tFace;
    vec3 null;
    if (IntersectRayCellFace(origin, dir, t0, index, side, tFace, null)) {
        if (IsRayEnteringCell(dir, index, side)) {
            // Only update initial cell values if this is the closest cell
            if (tFace < t1) {
                cellIndex = index;
                entranceFace = side;
                t1 = tFace;
            }
            return true;
        }
    }
    return false;
}


// The nvidia compiler optimizer cannot handle deep nested loops
// unroll none improves compile time
// inline none breaks the compiler
// Side note: This is supposed to be an inline change but it is in fact a source-wide flag

// #pragma optionNV(inline none)
#pragma optionNV(unroll none)

#ifdef NVIDIA
#include BBTraversalAlgorithmsNV.frag
#else
#include BBTraversalAlgorithms.frag
#endif

int SearchSideForInitialCellBasic(vec3 origin, vec3 dir, float t0, int sideID, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
    int fastDim = GetFastDimForFaceIndex(sideID);
    int slowDim = GetSlowDimForFaceIndex(sideID);
    ivec3 side = GetFaceFromFaceIndex(sideID);
    ivec3 index = (side+1)/2 * (cellDims-1);
    
    for (index[slowDim] = 0; index[slowDim] < cellDims[slowDim]; index[slowDim]++) {
        for (index[fastDim] = 0; index[fastDim] < cellDims[fastDim]; index[fastDim]++) {
            
            if (IntersectRaySideCellBBox(origin, dir, t0, index, sideID, fastDim, slowDim)) {
                if (IsFaceThatPassedBBAnInitialCell(origin, dir, t0, index, side, cellIndex, entranceFace, t1))
                    return 1;
            }
        }
    }
    return 0;
}

// Does not work on nvidia
// #define SearchSideForInitialCellWithOctree_NLevels(N, origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1) SearchSideForInitialCellWithOctree_ ## N ## Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1)

int SearchSideForInitialCell(vec3 origin, vec3 dir, float t0, int sideID, OUT ivec3 cellIndex, OUT ivec3 entranceFace, inout float t1)
{
    int fastDim = GetFastDimForFaceIndex(sideID);
    int slowDim = GetSlowDimForFaceIndex(sideID);

#if   BB_LEVELS == 1
    return SearchSideForInitialCellWithOctree_1Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#elif BB_LEVELS == 2
    return SearchSideForInitialCellWithOctree_2Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#elif BB_LEVELS == 3
    return SearchSideForInitialCellWithOctree_3Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#elif BB_LEVELS == 4
    return SearchSideForInitialCellWithOctree_4Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#elif BB_LEVELS == 5
    return SearchSideForInitialCellWithOctree_5Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#elif BB_LEVELS == 6
    return SearchSideForInitialCellWithOctree_6Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#elif BB_LEVELS == 7
    return SearchSideForInitialCellWithOctree_7Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#elif BB_LEVELS == 8
    return SearchSideForInitialCellWithOctree_8Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#elif BB_LEVELS == 9
    return SearchSideForInitialCellWithOctree_9Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#elif BB_LEVELS == 10
    return SearchSideForInitialCellWithOctree_10Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#elif BB_LEVELS == 11
    return SearchSideForInitialCellWithOctree_11Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#elif BB_LEVELS == 12
    return SearchSideForInitialCellWithOctree_12Levels(origin, dir, t0, sideID, fastDim, slowDim, cellIndex, entranceFace, t1);
#endif
}

int FindInitialCell(vec3 origin, vec3 dir, float t0, OUT ivec3 cellIndex, OUT ivec3 entranceFace, OUT float t1)
{
    t1 = FLT_MAX;
    int intersections = 0;
    for (int side = 0; side < 6; side++)
        intersections += SearchSideForInitialCell(origin, dir, t0, side, cellIndex, entranceFace, t1);
    return intersections;
}

vec4 Traverse(vec3 origin, vec3 dir, vec3 rayLightingNormal, float tMin, float tMax, float t0, ivec3 currentCell, ivec3 entranceFace, OUT float t1);

void main(void)
{
    vec3 eye, dir, rayLightingNormal;
    float sceneDepthT;
    GetRayParameters(eye, dir, rayLightingNormal, sceneDepthT);
    
    float t0, t1, tp;
    
    bool intersectBox = IntersectRayBoundingBox(eye, dir, 0, userExtsMin, userExtsMax, t0, t1);
    float tMin = t0, tMax = min(t1, sceneDepthT);
    
    if (intersectBox) {
        ivec3 initialCell;
        ivec3 entranceFace;
        float t0 = -FLT_MAX;
        float t1;
        vec4 accum = vec4(0);
        int intersections;
        int i = 0;
        do {
            intersections = FindInitialCell(eye, dir, t0, initialCell, entranceFace, t1);
            
            if (intersections > 0) {
                vec4 color = Traverse(eye, dir, rayLightingNormal, tMin, tMax, t1, initialCell, entranceFace, t1);
                BlendToBack(accum, color);
            }
            
            // Failsafe to prevent infinite recursion due to float precision error
            if (i++ > 8 || t1-t0 <= EPSILON)
                break;

			if (accum.a > ALPHA_BREAK)
				break;
                
            t0 = t1;
            
        } while (intersections > 1);
        
        if (accum.a < ALPHA_DISCARD) {
            // discard; // There is a bug on in the 2015 15" AMD MBP laptops where this does not work with larger(?) datasets
            fragColor = vec4(0);
            gl_FragDepth = 1;
            return;
        }
        fragColor = accum;
        return;
    }
    discard;
}
