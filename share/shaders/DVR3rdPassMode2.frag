#version 410 core

in vec4 gl_FragCoord;
flat in ivec4 provokingVertexIdx;
layout(location = 0) out vec4 color;

uniform sampler2D       backFaceTexture;
uniform sampler2D       frontFaceTexture;
uniform sampler3D       volumeTexture;
uniform usampler3D      missingValueMaskTexture; // !!unsigned integer!!
uniform sampler1D       colorMapTexture;
uniform sampler3D       zCoordsTexture;
uniform samplerBuffer   xyCoordsTexture;
uniform sampler2D       depthTexture;

uniform ivec3 volumeDims;        // number of vertices in each direction of this volume
uniform ivec2 viewportDims;      // width and height of this viewport
uniform vec4  clipPlanes[6];     // clipping planes in **un-normalized** model coordinates
uniform vec3  boxMin;            // Model space min coordinates
uniform vec3  boxMax;            // Model space max coordinates
uniform vec3  colorMapRange;

uniform float stepSize1D;        // ray casting step size
uniform bool  flags[3];
uniform float lightingCoeffs[4]; // lighting parameters

uniform mat4  MV;
uniform mat4  Projection;
uniform mat4  transposedInverseMV;

//
// Derive helper variables
//
const float ULP        = 1.2e-7f;           // 2^-23 == 1.192e-7
const float ULP10      = 1.2e-6f;
bool  fast             = flags[0];          // fast rendering mode
bool  lighting         = fast ? false : flags[1];   // no lighting in fast mode
bool  hasMissingValue  = flags[2];          // has missing values or not
float ambientCoeff     = lightingCoeffs[0];
float diffuseCoeff     = lightingCoeffs[1];
float specularCoeff    = lightingCoeffs[2];
float specularExp      = lightingCoeffs[3];
vec3  volumeDims1o     = 1.0 / vec3( volumeDims );
vec3  boxSpan          = boxMax - boxMin;

// 
// Code for triangles:
// Each triangle is represented as (v0, v1, v2)
// Triangle vertices are ordered such that (v1-v0)x(v2-v0) faces inside.
//
const int triangles[36] = int[36](
    /* front | back     | top      | bottom   | right    | left */
    7, 6, 3,   0, 1, 4,   4, 5, 7,   3, 2, 0,   5, 1, 6,   4, 7, 0,
    2, 3, 6,   5, 4, 1,   6, 7, 5,   1, 0, 2,   2, 6, 1,   3, 0, 7
    /* front | back     | top      | bottom   | right    | left */ );

//
// Input:  logical index of a vertex
// Output: user coordinates in the model space
//
vec3 GetCoordinates( const in ivec3 index )
{
    vec4 xyC     = texelFetch( xyCoordsTexture, index.y *  volumeDims.x + index.x );
    vec4 zC      = texelFetch( zCoordsTexture,  index, 0 );
    return vec3( xyC.xy, zC.x );
}

// 
// Input:  logical index of a cell
// Output: user coordinates of its 8 indices
//
void FillCellVertCoordinates( const in ivec3 cellIdx, out vec3 coord[8] )
{
    //          Y
    //        4 __________________5
    //         /|               /|
    //        / |              / |
    //       /  |             /  |
    //     7/___|___________6/   |
    //      |   |           |    |
    //      |   |           |    |
    //      |   |___________|____|  X
    //      |   /0          |   /1
    //      |  /            |  /
    //      | /             | /
    //     3|/______________|/    
    //                      2 
    //    Z
    ivec3 cubeVertIdx[8];
    ivec3 v0       = cellIdx;
    cubeVertIdx[0] = v0;
    cubeVertIdx[1] = ivec3(v0.x + 1, v0.y    , v0.z     );
    cubeVertIdx[2] = ivec3(v0.x + 1, v0.y    , v0.z + 1 );
    cubeVertIdx[3] = ivec3(v0.x    , v0.y    , v0.z + 1 );
    cubeVertIdx[4] = ivec3(v0.x    , v0.y + 1, v0.z     );
    cubeVertIdx[5] = ivec3(v0.x + 1, v0.y + 1, v0.z     );
    cubeVertIdx[6] = ivec3(v0.x + 1, v0.y + 1, v0.z + 1 );
    cubeVertIdx[7] = ivec3(v0.x    , v0.y + 1, v0.z + 1 );

    for( int i = 0; i < 8; i++ )
    {
        ivec3 index  = cubeVertIdx[i];
        vec4 xyC     = texelFetch( xyCoordsTexture, index.y *  volumeDims.x + index.x );
        vec4 zC      = texelFetch( zCoordsTexture,  index, 0 );
        coord[i].xy  = xyC.xy;
        coord[i].z   = zC.x;
    }
}

//
// Input:  Location to be evaluated in texture coordinates and model coordinates.
// Output: If this location should be skipped.
// Note:   It is skipped in two cases: 1) it represents a missing value
//                                     2) it is outside of clipping planes
//
bool ShouldSkip( const in vec3 tc, const in vec3 mc )
{
    if( hasMissingValue && (texture(missingValueMaskTexture, tc).r != 0u) )
        return true;

    vec4 positionModel = vec4( mc, 1.0 );
    float prd[6];
    for( int i = 0; i < 6; i++ )
        prd[i] = dot(positionModel, clipPlanes[i]);

    // Returns true if one product is less than zero
    return ( prd[0] < 0.0 || prd[1] < 0.0 || prd[2] < 0.0 ||
             prd[3] < 0.0 || prd[4] < 0.0 || prd[5] < 0.0  );
}

//
// Input:  Location to be evaluated in texture coordinates
// Output: Gradient at that location
//
vec3 CalculateGradient( const in vec3 tc)
{
    vec3 h0 = vec3(-0.5 ) * volumeDims1o;
    vec3 h1 = vec3( 0.5 ) * volumeDims1o;
    vec3 h  = vec3( 1.0 );

    if ((tc.x + h0.x) < 0.0) {
        h0.x = 0.0;
        h.x = 0.5;
    }
    if ((tc.x + h1.x) > 1.0) {
        h1.x = 0.0;
        h.x = 0.5;
    }
    if ((tc.y + h0.y) < 0.0) {
        h0.y = 0.0;
        h.y = 0.5;
    }
    if ((tc.y + h1.y) > 1.0) {
        h1.y = 0.0;
        h.y = 0.5;
    }
    if ((tc.z + h0.z) < 0.0) {
        h0.z = 0.0;
        h.z = 0.5;
    }
    if ((tc.z + h1.z) > 1.0) {
        h1.z = 0.0;
        h.z = 0.5;
    }

    vec3 a0, a1;
    a0.x = texture( volumeTexture, tc + vec3(h0.x,0.0,0.0) ).r;
    a1.x = texture( volumeTexture, tc + vec3(h1.x,0.0,0.0) ).r;
    a0.y = texture( volumeTexture, tc + vec3(0.0,h0.y,0.0) ).r;
    a1.y = texture( volumeTexture, tc + vec3(0.0,h1.y,0.0) ).r;
    a0.z = texture( volumeTexture, tc + vec3(0.0,0.0,h0.z) ).r;
    a1.z = texture( volumeTexture, tc + vec3(0.0,0.0,h1.z) ).r;

    return (a1-a0 / h);
}


//
// Input:  a position in the model space
// Return: depth value at that position.
//
float CalculateDepth( const in vec3 pModel )
{
    vec4    pClip =  Projection  * MV * vec4( pModel, 1.0 );
    vec3    pNdc  =  pClip.xyz   / pClip.w;
    return (gl_DepthRange.diff * 0.5 * pNdc.z + (gl_DepthRange.near + gl_DepthRange.far) * 0.5);
}


bool PosInsideOfCell( const in ivec3 cellIdx, const in vec3 pos )
{
    vec3 cubeVertCoord[8];
    FillCellVertCoordinates( cellIdx, cubeVertCoord );

    for( int i = 0; i < 12; i++ )
    {
        ivec3 tri   = ivec3( triangles[i*3], triangles[i*3+1], triangles[i*3+2] );
        vec3 posv0  = pos                     - cubeVertCoord[ tri[0] ];
        vec3 v1v0   = cubeVertCoord[ tri[1] ] - cubeVertCoord[ tri[0] ];
        vec3 v2v0   = cubeVertCoord[ tri[2] ] - cubeVertCoord[ tri[0] ];
        vec3 inward = cross( v1v0, v2v0 );  // vector pointing inside of the cell
        if( dot( posv0, inward ) < -ULP )   // pos tests to be outside of the cell
            return false;
    }
    
    return true;
}

bool CellOutsideBound( const in ivec3 cellIdx )
{
    if( cellIdx.x < 0 || cellIdx.x > volumeDims.x - 2 || 
        cellIdx.y < 0 || cellIdx.y > volumeDims.y - 2 ||
        cellIdx.z < 0 || cellIdx.z > volumeDims.z - 2   )
        return true;
    else
        return false;
}

bool CellOnBoundary( const in ivec3 cellIdx )
{
    if( cellIdx.x == 0 || cellIdx.x == volumeDims.x - 2 || 
        cellIdx.y == 0 || cellIdx.y == volumeDims.y - 2 ||
        cellIdx.z == 0 || cellIdx.z == volumeDims.z - 2   )
        return true;
    else
        return false;
}

bool LocateNextCell( const in ivec3 currentCellIdx, const in vec3 pos, out ivec3 nextCellIdx )
{
    ivec3 c  = currentCellIdx;
    ivec3 group[27];

    group[0]  = c;           // first cell in this group is the current cell itself.
    group[1]  = ivec3( c.x - 1, c.yz );              // next 6 cells are adjacent to
    group[2]  = ivec3( c.x + 1, c.yz );              // the current cell by face
    group[3]  = ivec3( c.x,     c.y - 1, c.z );
    group[4]  = ivec3( c.x,     c.y + 1, c.z );
    group[5]  = ivec3( c.xy,             c.z - 1 );
    group[6]  = ivec3( c.xy,             c.z + 1 );

    // Next 12 cells are adjacent to the current one by edge
    group[7]  = ivec3( c.x,     c.y + 1, c.z + 1 ); // top-front
    group[8]  = ivec3( c.x - 1, c.y + 1, c.z     ); // top-left
    group[9]  = ivec3( c.x    , c.y + 1, c.z - 1 ); // top-back
    group[10] = ivec3( c.x + 1, c.y + 1, c.z     ); // top-right
    group[11] = ivec3( c.x,     c.y - 1, c.z + 1 ); // bottom-front
    group[12] = ivec3( c.x - 1, c.y - 1, c.z     ); // bottom-left
    group[13] = ivec3( c.x,     c.y - 1, c.z - 1 ); // bottom-back
    group[14] = ivec3( c.x + 1, c.y - 1, c.z     ); // bottom-right
    group[15] = ivec3( c.x - 1, c.y,     c.z + 1 ); // front-left
    group[16] = ivec3( c.x - 1, c.y,     c.z - 1 ); // left-back
    group[17] = ivec3( c.x + 1, c.y,     c.z - 1 ); // back-right
    group[18] = ivec3( c.x + 1, c.y,     c.z + 1 ); // right-front

    // Next 8 cells are adjacent to the current one by corner
    group[19] = ivec3( c.x - 1, c.y + 1, c.z + 1 ); // corner 7
    group[20] = ivec3( c.x - 1, c.y + 1, c.z - 1 ); // corner 4
    group[21] = ivec3( c.x + 1, c.y + 1, c.z - 1 ); // corner 5
    group[22] = ivec3( c.x + 1, c.y + 1, c.z + 1 ); // corner 6
    group[23] = ivec3( c.x - 1, c.y - 1, c.z + 1 ); // corner 3
    group[24] = ivec3( c.x - 1, c.y - 1, c.z - 1 ); // corner 0
    group[25] = ivec3( c.x + 1, c.y - 1, c.z - 1 ); // corner 1
    group[26] = ivec3( c.x + 1, c.y - 1, c.z + 1 ); // corner 2

    for( int i = 0; i < 27; i++ )
        if( !CellOutsideBound( group[i] ) && PosInsideOfCell( group[i], pos ) )
        {
            nextCellIdx = group[i];
            return true;
        }

    return false;
}


bool SmartLocateNextCell( const in ivec3 currentCellIdx, const in vec3 pos, 
                          const in vec3  previousPos,    out ivec3 nextCellIdx )
{
    vec3  cubeVertCoord[8];
    FillCellVertCoordinates( currentCellIdx, cubeVertCoord );
    vec3  rayDir = pos - previousPos;
    float vfit[8], fit[27];  // fitness value for 8 vertices and 27 cells.
    for(  int i = 0; i < 8; i++ )
        vfit[i] = dot( rayDir, cubeVertCoord[i] - previousPos );

    ivec3 c  = currentCellIdx;
    ivec3 group[27];

    // First cell in this group is the current cell itself.
    group[0]  = c;
    float vfitSum = 0.0;
    for( int i = 0; i < 8; i++ )    
        vfitSum += vfit[i];
    fit  [0]  = vfitSum / 8.0;
    
    // Next 6 cells are adjacent to the current cell by face
    group[1]  = ivec3( c.x - 1, c.yz );                     // left
    group[2]  = ivec3( c.x + 1, c.yz );                     // right
    group[3]  = ivec3( c.x,     c.y - 1, c.z );             //bottom
    group[4]  = ivec3( c.x,     c.y + 1, c.z );             // top
    group[5]  = ivec3( c.xy,             c.z - 1 );         // back
    group[6]  = ivec3( c.xy,             c.z + 1 );         // front
    fit  [1]  = (vfit[0] + vfit[3] + vfit[4] + vfit[7]) * 0.25;
    fit  [2]  = (vfit[1] + vfit[2] + vfit[5] + vfit[6]) * 0.25;
    fit  [3]  = (vfit[0] + vfit[1] + vfit[2] + vfit[3]) * 0.25;
    fit  [4]  = (vfit[4] + vfit[5] + vfit[6] + vfit[7]) * 0.25;
    fit  [5]  = (vfit[0] + vfit[1] + vfit[4] + vfit[5]) * 0.25;
    fit  [6]  = (vfit[2] + vfit[3] + vfit[6] + vfit[7]) * 0.25;

    // Next 12 cells are adjacent to the current one by edge
    group[7]  = ivec3( c.x,     c.y + 1, c.z + 1 ); // top-front
    group[8]  = ivec3( c.x - 1, c.y + 1, c.z     ); // top-left
    group[9]  = ivec3( c.x    , c.y + 1, c.z - 1 ); // top-back
    group[10] = ivec3( c.x + 1, c.y + 1, c.z     ); // top-right
    group[11] = ivec3( c.x,     c.y - 1, c.z + 1 ); // bottom-front
    group[12] = ivec3( c.x - 1, c.y - 1, c.z     ); // bottom-left
    group[13] = ivec3( c.x,     c.y - 1, c.z - 1 ); // bottom-back
    group[14] = ivec3( c.x + 1, c.y - 1, c.z     ); // bottom-right
    group[15] = ivec3( c.x - 1, c.y,     c.z + 1 ); // front-left
    group[16] = ivec3( c.x - 1, c.y,     c.z - 1 ); // left-back
    group[17] = ivec3( c.x + 1, c.y,     c.z - 1 ); // back-right
    group[18] = ivec3( c.x + 1, c.y,     c.z + 1 ); // right-front
    fit  [7]  = (vfit[6] + vfit[7]) * 0.5;
    fit  [8]  = (vfit[4] + vfit[7]) * 0.5;
    fit  [9]  = (vfit[4] + vfit[5]) * 0.5;
    fit  [10] = (vfit[5] + vfit[6]) * 0.5;
    fit  [11] = (vfit[2] + vfit[3]) * 0.5;
    fit  [12] = (vfit[0] + vfit[3]) * 0.5;
    fit  [13] = (vfit[0] + vfit[1]) * 0.5;
    fit  [14] = (vfit[1] + vfit[2]) * 0.5;
    fit  [15] = (vfit[3] + vfit[7]) * 0.5;
    fit  [16] = (vfit[0] + vfit[4]) * 0.5;
    fit  [17] = (vfit[1] + vfit[5]) * 0.5;
    fit  [18] = (vfit[2] + vfit[6]) * 0.5;

    // Next 8 cells are adjacent to the current one by corner
    group[19] = ivec3( c.x - 1, c.y + 1, c.z + 1 ); // corner 7
    group[20] = ivec3( c.x - 1, c.y + 1, c.z - 1 ); // corner 4
    group[21] = ivec3( c.x + 1, c.y + 1, c.z - 1 ); // corner 5
    group[22] = ivec3( c.x + 1, c.y + 1, c.z + 1 ); // corner 6
    group[23] = ivec3( c.x - 1, c.y - 1, c.z + 1 ); // corner 3
    group[24] = ivec3( c.x - 1, c.y - 1, c.z - 1 ); // corner 0
    group[25] = ivec3( c.x + 1, c.y - 1, c.z - 1 ); // corner 1
    group[26] = ivec3( c.x + 1, c.y - 1, c.z + 1 ); // corner 2
    fit  [19] = vfit[7];
    fit  [20] = vfit[4];
    fit  [21] = vfit[5];
    fit  [22] = vfit[6];
    fit  [23] = vfit[3];
    fit  [24] = vfit[0];
    fit  [25] = vfit[1];
    fit  [26] = vfit[2];

    int sortedIdx[27];
    for( int i = 0; i < 27; i++ )
        sortedIdx[i] = i;

    // Bubble sort
    for( int i = 0; i < 26; i++ )
        for( int j = i + 1; j < 27; j++ )
        {
            if( fit[j] > fit[i] )
            {
                int   tmpI   = sortedIdx[i];
                sortedIdx[i] = sortedIdx[j];
                sortedIdx[j] = tmpI;
                float tmpF   = fit[i];
                fit[i]       = fit[j];
                fit[j]       = tmpF;
            }
        }

    for( int i = 0; i < 27; i++ )
    {
        int realIdx = sortedIdx[i];
        if( !CellOutsideBound( group[realIdx] ) && PosInsideOfCell( group[realIdx], pos ) )
        {
            nextCellIdx = group[realIdx];
            return true;
        }
    }

    return false;
}


bool SmarterLocateNextCell( const in ivec3 currentCellIdx, const in vec3 pos, 
                            const in vec3  previousPos,    out ivec3 nextCellIdx )
{
    vec3  cubeVertCoord[8];
    FillCellVertCoordinates( currentCellIdx, cubeVertCoord );
    vec3  rayDir = pos - previousPos;
    vec3  vertVec[8];
    for(  int i = 0; i < 8; i++ )
        vertVec[i] = cubeVertCoord[i] - previousPos;

    ivec3 c  = currentCellIdx;
    ivec3 group[27];
    float fit[27];  // fitness value for 27 cells.

    // First cell in this group is the current cell itself.
    group[0]  = c;
    fit  [0]  = 0.0;
    
    // Next 6 cells are adjacent to the current cell by face
    group[1]  = ivec3( c.x - 1, c.yz );                     // left
    group[2]  = ivec3( c.x + 1, c.yz );                     // right
    group[3]  = ivec3( c.x,     c.y - 1, c.z );             //bottom
    group[4]  = ivec3( c.x,     c.y + 1, c.z );             // top
    group[5]  = ivec3( c.xy,             c.z - 1 );         // back
    group[6]  = ivec3( c.xy,             c.z + 1 );         // front
    fit  [1]  = dot(rayDir, (vertVec[0] + vertVec[3] + vertVec[4] + vertVec[7]) * 0.25);
    fit  [2]  = dot(rayDir, (vertVec[1] + vertVec[2] + vertVec[5] + vertVec[6]) * 0.25);
    fit  [3]  = dot(rayDir, (vertVec[0] + vertVec[1] + vertVec[2] + vertVec[3]) * 0.25);
    fit  [4]  = dot(rayDir, (vertVec[4] + vertVec[5] + vertVec[6] + vertVec[7]) * 0.25);
    fit  [5]  = dot(rayDir, (vertVec[0] + vertVec[1] + vertVec[4] + vertVec[5]) * 0.25);
    fit  [6]  = dot(rayDir, (vertVec[2] + vertVec[3] + vertVec[6] + vertVec[7]) * 0.25);

    // Next 12 cells are adjacent to the current one by edge
    group[7]  = ivec3( c.x,     c.y + 1, c.z + 1 ); // top-front
    group[8]  = ivec3( c.x - 1, c.y + 1, c.z     ); // top-left
    group[9]  = ivec3( c.x    , c.y + 1, c.z - 1 ); // top-back
    group[10] = ivec3( c.x + 1, c.y + 1, c.z     ); // top-right
    group[11] = ivec3( c.x,     c.y - 1, c.z + 1 ); // bottom-front
    group[12] = ivec3( c.x - 1, c.y - 1, c.z     ); // bottom-left
    group[13] = ivec3( c.x,     c.y - 1, c.z - 1 ); // bottom-back
    group[14] = ivec3( c.x + 1, c.y - 1, c.z     ); // bottom-right
    group[15] = ivec3( c.x - 1, c.y,     c.z + 1 ); // front-left
    group[16] = ivec3( c.x - 1, c.y,     c.z - 1 ); // left-back
    group[17] = ivec3( c.x + 1, c.y,     c.z - 1 ); // back-right
    group[18] = ivec3( c.x + 1, c.y,     c.z + 1 ); // right-front
    fit  [7]  = dot(rayDir, (vertVec[6] + vertVec[7]) * 0.5);
    fit  [8]  = dot(rayDir, (vertVec[4] + vertVec[7]) * 0.5);
    fit  [9]  = dot(rayDir, (vertVec[4] + vertVec[5]) * 0.5);
    fit  [10] = dot(rayDir, (vertVec[5] + vertVec[6]) * 0.5);
    fit  [11] = dot(rayDir, (vertVec[2] + vertVec[3]) * 0.5);
    fit  [12] = dot(rayDir, (vertVec[0] + vertVec[3]) * 0.5);
    fit  [13] = dot(rayDir, (vertVec[0] + vertVec[1]) * 0.5);
    fit  [14] = dot(rayDir, (vertVec[1] + vertVec[2]) * 0.5);
    fit  [15] = dot(rayDir, (vertVec[3] + vertVec[7]) * 0.5);
    fit  [16] = dot(rayDir, (vertVec[0] + vertVec[4]) * 0.5);
    fit  [17] = dot(rayDir, (vertVec[1] + vertVec[5]) * 0.5);
    fit  [18] = dot(rayDir, (vertVec[2] + vertVec[6]) * 0.5);

    // Next 8 cells are adjacent to the current one by corner
    group[19] = ivec3( c.x - 1, c.y + 1, c.z + 1 ); // corner 7
    group[20] = ivec3( c.x - 1, c.y + 1, c.z - 1 ); // corner 4
    group[21] = ivec3( c.x + 1, c.y + 1, c.z - 1 ); // corner 5
    group[22] = ivec3( c.x + 1, c.y + 1, c.z + 1 ); // corner 6
    group[23] = ivec3( c.x - 1, c.y - 1, c.z + 1 ); // corner 3
    group[24] = ivec3( c.x - 1, c.y - 1, c.z - 1 ); // corner 0
    group[25] = ivec3( c.x + 1, c.y - 1, c.z - 1 ); // corner 1
    group[26] = ivec3( c.x + 1, c.y - 1, c.z + 1 ); // corner 2
    fit  [19] = dot(rayDir, vertVec[7]);
    fit  [20] = dot(rayDir, vertVec[4]);
    fit  [21] = dot(rayDir, vertVec[5]);
    fit  [22] = dot(rayDir, vertVec[6]);
    fit  [23] = dot(rayDir, vertVec[3]);
    fit  [24] = dot(rayDir, vertVec[0]);
    fit  [25] = dot(rayDir, vertVec[1]);
    fit  [26] = dot(rayDir, vertVec[2]);

    int sortedIdx[27];
    for( int i = 0; i < 27; i++ )
        sortedIdx[i] = i;

    // Bubble sort
    for( int i = 0; i < 26; i++ )
        for( int j = i + 1; j < 27; j++ )
        {
            if( fit[j] > fit[i] )
            {
                int   tmpI   = sortedIdx[i];
                sortedIdx[i] = sortedIdx[j];
                sortedIdx[j] = tmpI;
                float tmpF   = fit[i];
                fit[i]       = fit[j];
                fit[j]       = tmpF;
            }
        }

    for( int i = 0; i < 27; i++ )
    {
        int realIdx = sortedIdx[i];
        if( !CellOutsideBound( group[realIdx] ) && PosInsideOfCell( group[realIdx], pos ) )
        {
            nextCellIdx = group[realIdx];
            return true;
        }
    }

    return false;
}

vec3 CalculateCellCenterTex( const ivec3 cellIdx )
{
    // only get coordinates for 4 vertices
    ivec3 vertIdx[4];
    vertIdx[0] = cellIdx;
    vertIdx[1] = ivec3(cellIdx.x + 1, cellIdx.y    , cellIdx.z     );
    vertIdx[2] = ivec3(cellIdx.x    , cellIdx.y    , cellIdx.z + 1 );
    vertIdx[3] = ivec3(cellIdx.x    , cellIdx.y + 1, cellIdx.z     );

    vec3  vertCoord[4];
    for( int i = 0; i < 4; i++ )
        vertCoord[i] = GetCoordinates( vertIdx[i] );
        
    vec3 centerModel;
    centerModel.x = (vertCoord[1].x + vertCoord[0].x) * 0.5;
    centerModel.y = (vertCoord[3].y + vertCoord[0].y) * 0.5;
    centerModel.z = (vertCoord[2].z + vertCoord[0].z) * 0.5;

    return (centerModel - boxMin) / boxSpan;
}


void main(void)
{
    gl_FragDepth        = 1.0;
    color               = vec4( 0.0 );
    vec3  lightDirEye   = vec3(0.0, 0.0, 1.0); 

    // Get texture coordinates of this frament
    vec2 fragTex        = gl_FragCoord.xy / vec2( viewportDims );

    vec3 stopModel      = texture( backFaceTexture,  fragTex ).xyz;
    vec3 startModel     = texture( frontFaceTexture, fragTex ).xyz;
    vec3 rayDirModel    = stopModel - startModel;
    float rayDirLength  = length( rayDirModel );
    if(   rayDirLength  < ULP )
        discard;

    float nStepsf       = rayDirLength / stepSize1D;
    int   nSteps        = int(nStepsf) + 1;
    vec3  stepSize3D    = rayDirModel  / nStepsf;
    vec3  step1Model    = startModel + 0.01 * stepSize3D;

    // Test 1st step if inside of the cell, and correct it if not.
    ivec3 step1CellIdx  = provokingVertexIdx.xyz;
    if( !PosInsideOfCell( step1CellIdx, step1Model ) )
    {
        ivec3 correctIdx;
        if( LocateNextCell( step1CellIdx, step1Model, correctIdx ) )
        {
            step1CellIdx = correctIdx;
            //color = vec4( 0.2, 0.9, 0.2, 1.0 ); // green
        }
        else 
        {
            //color = vec4( 0.8, 0.2, 0.2, 1.0 ); // red
            discard;    // this case always happens on the boundary.
        }
    }
    
    // Give color to step 1
    vec3 step1Tex = CalculateCellCenterTex( step1CellIdx );
    if( !ShouldSkip( step1Tex, step1Model ) )
    {   
        float step1Value   = texture( volumeTexture, step1Tex ).r;
        float valTranslate = (step1Value - colorMapRange.x) / colorMapRange.z;
              color        = texture( colorMapTexture, valTranslate );
              color.rgb   *= color.a;
    }

    // Let's do a ray casting!
    vec3  step2Model       = step1Model;
    int   earlyTerm        = 0;         // why this ray got an early termination?
    ivec3 step2CellIdx;
    for( int stepi = 1; stepi <= nSteps; stepi++ )
    {
        if( color.a > 0.999 )
        {
            earlyTerm      = 1;
            break;
        }
    
        step2Model         = startModel + stepSize3D * float( stepi );
    
        if( !LocateNextCell(  step1CellIdx, step2Model, step2CellIdx ) )
        //if( !SmarterLocateNextCell(  step1CellIdx, step2Model, step1Model, step2CellIdx ) )
        {
            earlyTerm      = 2;
            break;
        }

        vec3 step2Tex      = CalculateCellCenterTex( step2CellIdx );
        float step2Value   = texture( volumeTexture, step2Tex ).r;
        float valTranslate = (step2Value - colorMapRange.x) / colorMapRange.z;
        vec4  backColor    = texture( colorMapTexture, valTranslate );
        color.rgb += (1.0 - color.a) * backColor.a * backColor.rgb;
        color.a   += (1.0 - color.a) * backColor.a;

        // keep up step 1 values as well
        step1CellIdx = step2CellIdx;
        step1Model   = step2Model;
    }

    if( earlyTerm == 2 && !CellOnBoundary( step1CellIdx ) )
        color = vec4( 0.9, 0.2, 0.2, 1.0);

}

