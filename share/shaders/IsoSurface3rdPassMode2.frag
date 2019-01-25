#version 410 core

in vec4 gl_FragCoord;
flat in ivec4 provokingVertexIdx;
layout(location = 0) out vec4 color;

uniform sampler2D       backFaceTexture;
uniform sampler2D       frontFaceTexture;
uniform sampler3D       volumeTexture;
uniform usampler3D      missingValueMaskTexture; // !!unsigned integer!!
uniform sampler1D       colorMapTexture;
uniform sampler3D       vertCoordsTexture;
uniform sampler3D       secondVarDataTexture;
uniform usampler3D      secondVarMaskTexture;   // !!unsigned integer!!

uniform ivec3 volumeDims;        // number of vertices in each direction of this volume
uniform ivec2 viewportDims;      // width and height of this viewport
uniform vec4  clipPlanes[6];     // clipping planes in **un-normalized** model coordinates
uniform vec3  colorMapRange;
uniform ivec3 entryCellIdx;

uniform float stepSize1D;        // ray casting step size
uniform float lightingCoeffs[4]; // lighting parameters
uniform bool  flags[4];
uniform bool  use2ndVar;

uniform int   numOfIsoValues;    // how many iso values are valid in isoValues array?
uniform float isoValues[4];      // currently set there are at most 4 iso values.

uniform mat4  MV;
uniform mat4  inversedMV;
uniform mat4  Projection;

//
// Derive helper variables
//
const float ULP        = 1.2e-7f;           // 2^-23 == 1.192e-7
const float ULP10      = 1.2e-6f;
const float ULP100     = 1.2e-5f;
const float Opaque     = 0.999;
bool  lighting         = flags[1];
bool  eyeInsideVolume  = flags[2];
bool  hasMissingValue  = flags[3];          // has missing values or not
float ambientCoeff     = lightingCoeffs[0];
float diffuseCoeff     = lightingCoeffs[1];
float specularCoeff    = lightingCoeffs[2];
float specularExp      = lightingCoeffs[3];
vec3  volumeDims1o     = 1.0 / vec3( volumeDims - 1 );
ivec3 volumeDimsm2     = volumeDims - 2;
mat4  transposedInverseMV = transpose( inversedMV );

// 
// Code for triangles:
// Each triangle is represented as (v0, v1, v2)
// Triangle vertices are ordered such that (v1-v0)x(v2-v0) faces inside.
//
const int Global_Triangles[36] = int[36](
    /* front   back       top        bottom     right      left */
    7, 6, 3,   0, 1, 4,   4, 5, 7,   3, 2, 0,   5, 1, 6,   4, 7, 0,
    2, 3, 6,   5, 4, 1,   6, 7, 5,   1, 0, 2,   2, 6, 1,   3, 0, 7
    /* front   back       top        bottom     right      left */ );

// 
// Input:  logical index of a cell
// Output: eye coordinates of its 8 indices
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
    cubeVertIdx[6] = v0 + 1;
    cubeVertIdx[7] = ivec3(v0.x    , v0.y + 1, v0.z + 1 );

    for( int i = 0; i < 8; i++ )
    {
        coord[i]   = (texelFetch( vertCoordsTexture,  cubeVertIdx[i], 0 )).xyz;
    }
}


//
// Input:  Location to be evaluated in texture coordinates and eye coordinates.
// Output: If this location should be skipped.
// Note:   It is skipped in two cases: 1) it represents a missing value
//                                     2) it is outside of clipping planes
//
bool ShouldSkip( const in vec3 tc, const in vec3 ec )
{
    if( hasMissingValue && (texture(missingValueMaskTexture, tc).r != 0u) )
        return true;

    if( use2ndVar && (texture(secondVarMaskTexture, tc).r != 0u) )
        return true;

    vec4 positionModel = inversedMV * vec4(ec, 1.0);
    for( int i = 0; i < 6; i++ )
    {
        if( dot(positionModel, clipPlanes[i]) < 0.0 )
            return true;
    }

    // Now we know that this position shouldn't be skipped!
    return false;
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

    return (a1 - a0 / h);
}


//
// Input:  a position in the eye space
// Return: depth value at that position.
//
float CalculateDepth( const in vec3 pEye )
{
    vec4    pClip =  Projection  * vec4( pEye, 1.0 );
    vec3    pNdc  =  pClip.xyz   / pClip.w;
    return  0.5 * fma( gl_DepthRange.diff, pNdc.z, (gl_DepthRange.near + gl_DepthRange.far) );
}


//
// Input:  a cell index and a position in the eye space
// Return: if this position is inside of this cell.
//
bool PosInsideOfCell( const in ivec3 cellIdx, const in vec3 pos )
{
    vec3 cubeVertCoord[8];
    FillCellVertCoordinates( cellIdx, cubeVertCoord );

    // First locate and compare with the bounding box.
    vec3 minCoord = cubeVertCoord[0], maxCoord = cubeVertCoord[0];
    for( int i = 1; i < 8; i++ )
    {
        minCoord   = min( minCoord, cubeVertCoord[i] );
        maxCoord   = max( maxCoord, cubeVertCoord[i] );
    }
    bvec3 tooSmall = lessThan(    pos, minCoord );
    bvec3 tooBig   = greaterThan( pos, maxCoord );
    if( any( tooSmall ) || any( tooBig ) )
        return false;

    // Second compare with the 12 triangles.
    int tri[3];
    for( int i = 0; i < 12; i++ )
    {
        tri[0]      = Global_Triangles[ i * 3     ]; 
        tri[1]      = Global_Triangles[ i * 3 + 1 ];
        tri[2]      = Global_Triangles[ i * 3 + 2 ];
        vec3 posv0  = pos                     - cubeVertCoord[ tri[0] ];
        vec3 v1v0   = cubeVertCoord[ tri[1] ] - cubeVertCoord[ tri[0] ];
        vec3 v2v0   = cubeVertCoord[ tri[2] ] - cubeVertCoord[ tri[0] ];
        vec3 inward = cross( v1v0, v2v0 );      // vector pointing inside of the cell
        if( dot( posv0, inward ) < -ULP100  )   // pos tests to be outside of the cell
            return false;
    }
    
    return true;
}


bool CellOutsideBound( const in ivec3 cellIdx )
{
    bvec3 tooSmall = lessThan(    cellIdx, ivec3(0) );
    bvec3 tooBig   = greaterThan( cellIdx, volumeDimsm2 );
    if( any( tooSmall ) || any( tooBig ) )
        return true;
    else
        return false;
}

bool CellOnBoundary( const in ivec3 cellIdx )
{
    // Assume the input is guaranteed to be inside of the volume
    bvec3 onSmallSide = equal( cellIdx, ivec3(0) );
    bvec3 onBigSide   = equal( cellIdx, volumeDimsm2 );
    if( any( onSmallSide ) || any( onBigSide ) )
        return true;
    else
        return false;
}


//
// Input:  a current cell index, and a position.
// Output: which neighbor cell contains this position.
// Return: True if the neighbor cell was found; False if not.
//
bool LocateNextCell( const in ivec3 currentCellIdx, const in vec3 pos, out ivec3 nextCellIdx )
{
    // First test if pos is in the current cell. There's a good chance it is!
    if( PosInsideOfCell( currentCellIdx, pos ) )
    {
        nextCellIdx = currentCellIdx;
        return true;
    }

    // Then we search its surrounding cells
    ivec3 c  = currentCellIdx;
    ivec3 group[26];
    group[0]  = ivec3( c.x - 1, c.yz );  // First 6 cells are adjacent to
    group[1]  = ivec3( c.x + 1, c.yz );  // the current cell by a face
    group[2]  = ivec3( c.x,     c.y - 1, c.z );
    group[3]  = ivec3( c.x,     c.y + 1, c.z );
    group[4]  = ivec3( c.xy,             c.z - 1 );
    group[5]  = ivec3( c.xy,             c.z + 1 );

    // Next 12 cells are adjacent to the current one by edge
    group[6]  = ivec3( c.x,     c.y + 1, c.z + 1 ); // top-front
    group[7]  = ivec3( c.x - 1, c.y + 1, c.z     ); // top-left
    group[8]  = ivec3( c.x    , c.y + 1, c.z - 1 ); // top-back
    group[9]  = ivec3( c.x + 1, c.y + 1, c.z     ); // top-right
    group[10] = ivec3( c.x,     c.y - 1, c.z + 1 ); // bottom-front
    group[11] = ivec3( c.x - 1, c.y - 1, c.z     ); // bottom-left
    group[12] = ivec3( c.x,     c.y - 1, c.z - 1 ); // bottom-back
    group[13] = ivec3( c.x + 1, c.y - 1, c.z     ); // bottom-right
    group[14] = ivec3( c.x - 1, c.y,     c.z + 1 ); // front-left
    group[15] = ivec3( c.x - 1, c.y,     c.z - 1 ); // left-back
    group[16] = ivec3( c.x + 1, c.y,     c.z - 1 ); // back-right
    group[17] = ivec3( c.x + 1, c.y,     c.z + 1 ); // right-front

    // Next 8 cells are adjacent to the current one by corner
    group[18] = ivec3( c.x - 1, c.y - 1, c.z - 1 ); // corner 0
    group[19] = ivec3( c.x + 1, c.y - 1, c.z - 1 ); // corner 1
    group[20] = ivec3( c.x + 1, c.y - 1, c.z + 1 ); // corner 2
    group[21] = ivec3( c.x - 1, c.y - 1, c.z + 1 ); // corner 3
    group[22] = ivec3( c.x - 1, c.y + 1, c.z - 1 ); // corner 4
    group[23] = ivec3( c.x + 1, c.y + 1, c.z - 1 ); // corner 5
    group[24] = ivec3( c.x + 1, c.y + 1, c.z + 1 ); // corner 6
    group[25] = ivec3( c.x - 1, c.y + 1, c.z + 1 ); // corner 7

    for( int i = 0; i < 26; i++ )
    {
        if( !CellOutsideBound( group[i] ) && PosInsideOfCell( group[i], pos ) )
        {
            nextCellIdx = group[i];
            return true;
        }
    }

    return false;
}


//
// Input:  a cell index, and a position of eye coordinate that's inside of this cell.
// Output: the texture coordinate of that position.
//
vec3 CalculatePosTex( const ivec3 cellIdx, const vec3 pos )
{
    // For VAPOR 3.1, we simply take the center point of the cell.
    //return ( vec3(cellIdx) + 0.5 ) * volumeDims1o;
    return vec3(cellIdx) * volumeDims1o;
}


void main(void)
{
    color               = vec4( 0.0 );
    vec3  lightDirEye   = vec3(0.0, 0.0, 1.0); 

    // Get texture coordinates of this frament
    vec2 fragTex        = gl_FragCoord.xy / vec2( viewportDims );

    vec3 stopEye        = texture( backFaceTexture,  fragTex ).xyz;
    vec3 startEye       = vec3( 0.0 );
    if( !eyeInsideVolume )
         startEye       = texture( frontFaceTexture, fragTex ).xyz;
    vec3 rayDirEye      = stopEye - startEye;
    float rayDirLength  = length( rayDirEye );
    if(   rayDirLength  < ULP10 )
        discard;

    // The incoming stepSize1D results in approximate 2 samples per cell.
    float nStepsf       = rayDirLength / stepSize1D;
    vec3  stepSize3D    = rayDirEye    / nStepsf;
    vec3  step1Eye      = startEye + 0.01 * stepSize3D;

    // Test 1st step if inside of the cell, and correct it if not.
    ivec3 step1CellIdx;
    if( eyeInsideVolume )
        step1CellIdx    = entryCellIdx;
    else
        step1CellIdx    = provokingVertexIdx.xyz;
    if( !PosInsideOfCell( step1CellIdx, step1Eye ) )
    {
        ivec3 correctIdx;
        if( LocateNextCell( step1CellIdx, step1Eye, correctIdx ) )
            step1CellIdx = correctIdx;
        else 
            discard;    // This case always happens on the boundary.
    }

    // Set depth value at the backface 
    gl_FragDepth     =  CalculateDepth( stopEye ) - ULP10;

    // Retrieve data value at step 1
    vec3 step1Tex    = CalculatePosTex( step1CellIdx, step1Eye );
    float step1Value = texture( volumeTexture, step1Tex ).r;

    // Let's do a ray casting!
    int   earlyTerm        = 0;     // 0        == termination when goes through the volume.
                                    // non-zero == early termination because of some reason.

    // We set the loop to terminate at 4 times the number of steps, in case
    //   there are many occurances of step size being halved.
    for( int stepi = 0; stepi < 4 * int(nStepsf); stepi++ )
    {
        // Early termination: enough opacity
        if( color.a > Opaque )
        {
            earlyTerm      = 1;
            break;
        }
    
        ivec3 step2CellIdx;
        vec3  step2Eye;
        bool  foundNextCell = false;
        // Try 3 different step sizes: base step size, 1/2 step size, and 1/4 step size.
        for( int i = 0; i < 3; i++ )
        {
            vec3 tmpStepSize  = stepSize3D;
            for( int j = 0; j < i; j++ )
                tmpStepSize  *= 0.5;
            step2Eye          = step1Eye + tmpStepSize;

            if( LocateNextCell( step1CellIdx, step2Eye, step2CellIdx ) )
            {
                foundNextCell = true;
                break;
            }
        }
        // Early termination: cannot find the next cell! 
        if( !foundNextCell )
        {
            earlyTerm  = 2;
            break;
        }

        // If step2 is at the same cell as step1, their data values are gonna be the same, 
        //   thus move forward directly.
        if( step2CellIdx == step1CellIdx )
        {
            step1Eye       = step2Eye;
            continue;
        }
    
        // Now we konw that step2 and step1 are at two adjacent cells
        vec3 step2Tex      = CalculatePosTex( step2CellIdx, step2Eye );
        if( ShouldSkip( step2Tex, step2Eye ) )
        {
            step1CellIdx   = step2CellIdx;
            step1Eye       = step2Eye;
            step1Tex       = step2Tex;
            continue;
        }
        float step2Value   = texture( volumeTexture, step2Tex ).r;
        
        // Test against iso values.
        for( int j = 0; j < numOfIsoValues; j++ )
        {
            float isoValJ = isoValues[j];
            if( ( isoValJ - step1Value) * (isoValJ - step2Value) < 0.0 )
            {
                float weight    = (isoValJ - step1Value) / (step2Value - step1Value);
                vec3  isoEye    = mix( step1Eye, step2Eye, weight );
                vec3  isoTex    = mix( step1Tex, step2Tex, weight );

                // Retrieve data value of the secondary variable at the same location
                //   if color mapping variable is enabled.
                if( use2ndVar )
                {
                    isoValJ     = texture( secondVarDataTexture, isoTex ).x;
                }

                float valTrans  = (isoValJ - colorMapRange.x) / colorMapRange.z;
                vec4  backColor = texture( colorMapTexture, valTrans );


                // Apply lighting.
                //   Note we do the calculation in eye space, because both the light direction
                //   and view direction are defined in the eye space.
                if( lighting && backColor.a > (1.0 - Opaque) )
                {
                    vec3 gradientModel   = CalculateGradient( isoTex );
                    if( length( gradientModel ) > ULP10 )
                    {
                        vec3 gradientEye = (transposedInverseMV * vec4(gradientModel, 0.0)).xyz;
                             gradientEye = normalize( gradientEye );
                        float diffuse    = abs( dot(lightDirEye, gradientEye) );
                        vec3 viewDirEye  = normalize( -isoEye );
                        vec3 reflectionEye = reflect( -lightDirEye, gradientEye );
                        float specular   = pow( max(0.0, dot( reflectionEye, viewDirEye )), specularExp );
                        backColor.rgb    = backColor.rgb * fma( diffuse, diffuseCoeff, ambientCoeff ) +
                                           specular * specularCoeff;
                    }
                }

                color.rgb += (1.0 - color.a) * backColor.a * backColor.rgb;
                color.a   += (1.0 - color.a) * backColor.a;

                // Apply depth no matter opacity
                gl_FragDepth  =  CalculateDepth( isoEye );
            }
        }

        // Normal termination: step 2 reaches a boundary cell.
        //   Note we only do this test after the ray steps *almost* nStepsf.
        if( (stepi > int(nStepsf) - 2) && CellOnBoundary( step2CellIdx ) )
        {
            earlyTerm = 0;  // 0 means it goes through the entire cell before termination.
            break;
        }
        else // Keep up step 1 values and get ready for the next step
        {
            step1CellIdx = step2CellIdx;
            step1Eye     = step2Eye; 
            step1Tex     = step2Tex; 
            step1Value   = step2Value;
        }
    }

    // If this pixel is transparent, we set depth to the far clipping plane.
    if( color.a < (1.0 - Opaque) )
        gl_FragDepth = 1.0;

    // Debug use only
    //if( earlyTerm == 2 && !CellOnBoundary( step1CellIdx ) )
    //    color = vec4( 0.9, 0.2, 0.2, 1.0); 
}

