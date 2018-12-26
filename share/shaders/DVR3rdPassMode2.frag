#version 410 core

in vec4 gl_FragCoord;
flat in ivec4 provokingVertexIdx;
layout(location = 0) out vec4 color;

uniform sampler2D       backFaceTexture;
uniform sampler2D       frontFaceTexture;
uniform sampler3D       volumeTexture;
uniform usampler3D      missingValueMaskTexture; // !!unsigned integer!!
uniform sampler1D       colorMapTexture;
uniform samplerBuffer   xyCoordsTexture;
uniform samplerBuffer   zCoordsTexture;
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
// Code for faces 
//
const int FaceFront    = 0;
const int FaceBack     = 1;
const int FaceTop      = 2;
const int FaceBottom   = 3;
const int FaceRight    = 4;
const int FaceLeft     = 5;

// 
// Code for triangles:
// Each triangle is represented as (v0, v1, v2)
// Triangle vertices are ordered such that (v1-v0)x(v2-v0) faces inside.
//
const int triangles[36] = int[36](
                          7, 6, 3,  2, 3, 6,    // two triangles of the front face
                          0, 1, 4,  5, 4, 1,    // two triangles of the back face
                          4, 5, 7,  6, 7, 5,    // two triangles of the top face
                          3, 2, 0,  1, 0, 2,    // two triangles of the bottom face
                          5, 1, 6,  2, 6, 1,    // two triangles of the right face
                          4, 7, 0,  3, 0, 7 );  // two triangles of the left face


//
// Input:  logical index of a vertex
// Output: user coordinates in the model space
//
vec3 GetCoordinates( const in ivec3 index )
{
    int xyOffset = index.y *  volumeDims.x + index.x;
    int zOffset  = index.z * (volumeDims.x * volumeDims.y) + xyOffset;
    vec4 xyC     = texelFetch( xyCoordsTexture, xyOffset );
    vec4 zC      = texelFetch( zCoordsTexture,  zOffset );
    return vec3( xyC.xy, zC.x );
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
    //                     2
    //     Z
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

    vec3 cubeVertCoord[8];
    for( int i = 0; i < 8; i++ )
        cubeVertCoord[i] = GetCoordinates( cubeVertIdx[i] );

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

bool CellOnBound( const in ivec3 cellIdx )
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
            step1CellIdx = correctIdx;
        else 
            discard;    // this case always happens on the boundary.
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

        // keep up cell index as well
        step1CellIdx = step2CellIdx;
    }

    if( earlyTerm == 2 && !CellOnBound( step1CellIdx ) )
        color = vec4( 0.9, 0.2, 0.2, 1.0);

}

