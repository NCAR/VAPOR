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
uniform vec4  unitDirections[26];
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
vec3  volumeDims1o     = 1.0 / vec3( volumeDims - 1 );

// 
// Code for triangles:
// Each triangle is represented as (v0, v1, v2)
// Triangle vertices are ordered such that (v1-v0)x(v2-v0) faces inside.
//
const int triangles[36] = int[36](
    /* front   back       top        bottom     right      left */
    7, 6, 3,   0, 1, 4,   4, 5, 7,   3, 2, 0,   5, 1, 6,   4, 7, 0,
    2, 3, 6,   5, 4, 1,   6, 7, 5,   1, 0, 2,   2, 6, 1,   3, 0, 7
    /* front   back       top        bottom     right      left */ );

//
// An optimized order of cells to search: cells appear in front of this list
//   are supposed to have a higher probablity to be selected.
//
int cells[27] = int[27](0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
                        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26 );

//
// Re-order the cell indices based on the current ray direction, so that
//   cells in front are more likely to contain a given position.
//
void ReorderCells( const in vec3 rayDirection )
{
    vec3 rayDir  = normalize( rayDirection );

    // Fitness score is the cosine between rayDir and each direction.
    //   fit[i] will have the fitness value of group[i] in LocateNextCell()
    //   The current cell itself has the biggest fit value: 1.0
    float fit[27];
    fit[0]  = 1.0;
    for( int i = 0; i < 26; i++ )
        fit[ i+1 ]  = dot( rayDir, unitDirections[i] );

    // Bubble sort these fitness values, so that cell indices are reordered
    for( int i = 1; i < 26; i++ )
        for( int j = i + 1; j < 27; j++ )
        {
            if( fit[j] > fit[i] )
            {
                int tmpI   = cells[i];
                cells[i]   = cells[j];
                cells[j]   = tmpI;
                float tmpF = fit[i];
                fit[i]     = fit[j];
                fit[j]     = tmpF;
            }
        }
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
        vec4 xyC     = texelFetch( xyCoordsTexture, index.y * volumeDims.x + index.x );
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
    group[19] = ivec3( c.x - 1, c.y - 1, c.z - 1 ); // corner 0
    group[20] = ivec3( c.x + 1, c.y - 1, c.z - 1 ); // corner 1
    group[21] = ivec3( c.x + 1, c.y - 1, c.z + 1 ); // corner 2
    group[22] = ivec3( c.x - 1, c.y - 1, c.z + 1 ); // corner 3
    group[23] = ivec3( c.x - 1, c.y + 1, c.z - 1 ); // corner 4
    group[24] = ivec3( c.x + 1, c.y + 1, c.z - 1 ); // corner 5
    group[25] = ivec3( c.x + 1, c.y + 1, c.z + 1 ); // corner 6
    group[26] = ivec3( c.x - 1, c.y + 1, c.z + 1 ); // corner 7

    for( int i = 0; i < 27; i++ )
    {
        int  j = cells[i];  // Re-ordered cell indices
        if( !CellOutsideBound( group[j] ) && PosInsideOfCell( group[j], pos ) )
        {
            nextCellIdx = group[j];
            return true;
        }
    }

    return false;
}


vec3 CalculatePosTex( const ivec3 cellIdx, const vec3 pos )
{
    // First, find bounding box of this cell, in texture space.
    vec3 bboxTex[2];
    bboxTex[0] = vec3( cellIdx     ) * volumeDims1o;
    bboxTex[1] = vec3( cellIdx + 1 ) * volumeDims1o;

    // Second, find bounding box of this cell, in model space.
    vec3 vertCoords[8], bboxModel[2];
    FillCellVertCoordinates( cellIdx, vertCoords );
    bboxModel[0] = vertCoords[0];
    bboxModel[1] = vertCoords[0];
    for( int i = 1; i < 8; i++ )
    {
        bboxModel[0].x = bboxModel[0].x < vertCoords[i].x ? bboxModel[0].x : vertCoords[i].x;
        bboxModel[0].y = bboxModel[0].y < vertCoords[i].y ? bboxModel[0].y : vertCoords[i].y;
        bboxModel[0].z = bboxModel[0].z < vertCoords[i].z ? bboxModel[0].z : vertCoords[i].z;

        bboxModel[1].x = bboxModel[1].x > vertCoords[i].x ? bboxModel[1].x : vertCoords[i].x;
        bboxModel[1].y = bboxModel[1].y > vertCoords[i].y ? bboxModel[1].y : vertCoords[i].y;
        bboxModel[1].z = bboxModel[1].z > vertCoords[i].z ? bboxModel[1].z : vertCoords[i].z;
    }

    vec3 weight = (pos - bboxModel[0]) / (bboxModel[1] - bboxModel[0]);
    return mix( bboxTex[0], bboxTex[1], weight );
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

    // Find a good order to test the 27 cells based on the direction of this ray.
    ReorderCells( rayDirModel );

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
    vec3 step1Tex = CalculatePosTex( step1CellIdx, step1Model );
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

        vec3 step2Tex      = CalculatePosTex( step2CellIdx, step2Model );
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

