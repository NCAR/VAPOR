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

uniform vec2  dataRanges[2];
uniform vec3  boxExtents[2];
uniform ivec3 volumeDims;        // number of vertices in this volume
uniform ivec2 viewportDims;      // width and height of this viewport
uniform vec4  clipPlanes[6];     // clipping planes in **un-normalized** model coordinates

uniform float stepSize1D;        // ray casting step size
uniform bool  flags[3];
uniform float lightingCoeffs[4]; // lighting parameters

uniform mat4  MV;
uniform mat4  inversedMV;

//
// Derive helper variables
//
const float EPSILON    = 5e-6f;
const float ULP        = 1.2e-7f;           // 2^-23 == 1.192e-7
bool  fast             = flags[0];          // fast rendering mode
bool  lighting         = fast ? false : flags[1];   // no lighting in fast mode
bool  hasMissingValue  = flags[2];          // has missing values or not
float ambientCoeff     = lightingCoeffs[0];
float diffuseCoeff     = lightingCoeffs[1];
float specularCoeff    = lightingCoeffs[2];
float specularExp      = lightingCoeffs[3];
// min, max, and range values of this variable. 
vec3  valueRange       = vec3(dataRanges[0], dataRanges[0].y - dataRanges[0].x); 
// min, max, and range values on this color map
vec3  colorMapRange    = vec3(dataRanges[1], dataRanges[1].y - dataRanges[1].x); 
vec3  boxMin           = boxExtents[0];     // min coordinates of the bounding box of this volume
vec3  boxMax           = boxExtents[1];     // max coordinates of the bounding box of this volume
vec3  volumeDimsf      = vec3( volumeDims );
vec3  boxSpan          = boxMax - boxMin;
mat4  transposedInverseMV = transpose(inversedMV);

struct WoopPrecalculation
{
    int   kx, ky, kz;
    float Sx, Sy, Sz;
} woopPre;  // Precalculation of constants in the Woop transformation

//
// Initialize structure woopPre;
//
void InitializeWoopPre( const in vec3 dir )
{
    if( abs(dir[0]) > abs(dir[1]) )
        woopPre.kz = 0;
    else
        woopPre.kz = 1;
    if( abs(dir[2]) > abs(dir[woopPre.kz]) )
        woopPre.kz = 2;
    woopPre.kx = (woopPre.kz + 1) % 3;
    woopPre.ky = (woopPre.kx + 1) % 3;

    if( dir[woopPre.kz] < 0.0 )
    {
        int tmp    = woopPre.kx;
        woopPre.kx = woopPre.ky;
        woopPre.ky = tmp;
    }

    woopPre.Sx = dir[woopPre.kx] / dir[woopPre.kz];
    woopPre.Sy = dir[woopPre.ky] / dir[woopPre.kz];
    woopPre.Sz = 1.0             / dir[woopPre.kz];
}

// 
// Woop ray-triangle intersection algorithm.
// Note: woopPre must be initialized by InitializeWoopPre() before invoking this function.
//
bool  WoopIntersection( const in vec3 orig, const in vec3 dir,
                        const in vec3 v0,   const in vec3 v1,   const in vec3 v2,
                        out float outT,  out float outU,  out float outV,  out float outW )
{
    vec3 A   = v0 - orig;
    vec3 B   = v1 - orig;
    vec3 C   = v2 - orig;

    float Ax = fma( -woopPre.Sx, A[ woopPre.kz ], A[ woopPre.kx ] );
    float Ay = fma( -woopPre.Sy, A[ woopPre.kz ], A[ woopPre.ky ] );
    float Bx = fma( -woopPre.Sx, B[ woopPre.kz ], B[ woopPre.kx ] );
    float By = fma( -woopPre.Sy, B[ woopPre.kz ], B[ woopPre.ky ] );
    float Cx = fma( -woopPre.Sx, C[ woopPre.kz ], C[ woopPre.kx ] );
    float Cy = fma( -woopPre.Sy, C[ woopPre.kz ], C[ woopPre.ky ] );

    float U  = fma( Cx, By, -Cy * Bx );
    float V  = fma( Ax, Cy, -Ay * Cx );
    float W  = fma( Bx, Ay, -By * Ax );

    /* Fall back to double precision
    if( U == 0.0 || V == 0.0 || W == 0.0 )
    {
        double CxBy = double(Cx)  * double(By);
        double CyBx = double(Cy)  * double(Bx);
        U           = float( CxBy - CyBx );
        double AxCy = double(Ax)  * double(Cy);
        double AyCx = double(Ay)  * double(Cx);
        V           = float( AxCy - AyCx );
        double BxAy = double(Bx)  * double(Ay);
        double ByAx = double(By)  * double(Ax);
        W           = float( BxAy - ByAx );
    }*/

    // Edge test
    if( (U < -ULP || V < -ULP || W < -ULP) && (U > ULP || V > ULP || W > ULP) )
        return false;

    // Parallel test
    float det = U + V + W;
    if( abs(det) < ULP )
        return false;

    float Az = woopPre.Sz * A[ woopPre.kz ];
    float Bz = woopPre.Sz * B[ woopPre.kz ];
    float Cz = woopPre.Sz * C[ woopPre.kz ];
    float T  = fma( U, Az, fma( V, Bz, W * Cz ) );
    outT     = T / det;
    outU     = U / det;
    outV     = V / det;
    outW     = W / det;

    return true;
}

//
// Input:  normalized value w.r.t. valueRange.
// Output: normalized value w.r.t. colorMapRange.
//
float TranslateValue( const in float value )
{
    if( colorMapRange.x != colorMapRange.y )
    {   
        float orig = value * valueRange.z + valueRange.x;
        return (orig - colorMapRange.x) / colorMapRange.z;
    }   
    else
        return value;
}

//
// Input:  logical index of a vertex
// Output: user coordinates in the eye space
//
vec3 GetCoordinates( const in ivec3 index )
{
    int xyOffset = index.y *  volumeDims.x + index.x;
    int zOffset  = index.z * (volumeDims.x * volumeDims.y) + xyOffset;
    vec4 xyC     = texelFetch( xyCoordsTexture, xyOffset );
    vec4 zC      = texelFetch( zCoordsTexture,  zOffset );
    vec4 eyeC    = MV * vec4( xyC.xy, zC.x, 1.0 );
    return eyeC.xyz;
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
    for( int i = 0; i < 6; i++ )
    {   
        if( dot(positionModel, clipPlanes[i]) < 0.0 )
            return true;
    }   

    return false;
}

//
// Input:  Location to be evaluated in texture coordinates
// Output: Gradient at that location
//
vec3 CalculateGradient( const in vec3 tc)
{
    vec3 h0 = vec3(-0.5 ) / volumeDimsf;
    vec3 h1 = vec3( 0.5 ) / volumeDimsf;
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


void  FindCellIndices(  const in ivec4 cellIdx,     // Input:  cell index
                        out ivec3 cubeVertIdx[8],   // Output: indices of 8 vertices
                        out vec3  cubeVertCoord[8], // Output: coordinates of 8 vertices
                        out ivec3 triangles[12] )   // Output: 12 triangles of this cell
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
    ivec3 V0       = cellIdx.xyz;
    cubeVertIdx[0] = V0;
    cubeVertIdx[1] = ivec3(V0.x + 1, V0.y    , V0.z     );
    cubeVertIdx[2] = ivec3(V0.x + 1, V0.y    , V0.z + 1 );
    cubeVertIdx[3] = ivec3(V0.x    , V0.y    , V0.z + 1 );
    cubeVertIdx[4] = ivec3(V0.x    , V0.y + 1, V0.z     );
    cubeVertIdx[5] = ivec3(V0.x + 1, V0.y + 1, V0.z     );
    cubeVertIdx[6] = ivec3(V0.x + 1, V0.y + 1, V0.z + 1 );
    cubeVertIdx[7] = ivec3(V0.x    , V0.y + 1, V0.z + 1 );

    for( int i = 0; i < 8; i++ )
        cubeVertCoord[i] = GetCoordinates( cubeVertIdx[i] );

    triangles[0]  =  ivec3( 7, 3, 6 );  // front face,  w == 0
    triangles[1]  =  ivec3( 2, 3, 6 );
    triangles[2]  =  ivec3( 0, 4, 1 );  // back face,   w == 1
    triangles[3]  =  ivec3( 5, 4, 1 );
    triangles[4]  =  ivec3( 4, 7, 5 );  // top face,    w == 2
    triangles[5]  =  ivec3( 6, 7, 5 );
    triangles[6]  =  ivec3( 3, 0, 2 );  // bottom face, w == 3
    triangles[7]  =  ivec3( 1, 0, 2 );
    triangles[8]  =  ivec3( 5, 1, 6 );  // right face,  w == 4
    triangles[9]  =  ivec3( 2, 1, 6 );
    triangles[10] =  ivec3( 4, 0, 7 );  // left face,   w == 5
    triangles[11] =  ivec3( 3, 0, 7 );
}

//
//   Test if the ray comes into the specified cell from the specified face.
// 
bool InputTest( const in ivec4 cellIdx, const in vec3 rayOrig, const in vec3 rayDir )
{
    ivec3 cubeVertIdx[8], triangles[12];
    vec3  cubeVertCoord[8];
    FindCellIndices( cellIdx, cubeVertIdx, cubeVertCoord, triangles );
    int tri1 = cellIdx.w * 2;
    int tri2 = cellIdx.w * 2 + 1;
    float t, u, v, w;
    bool intersect1 = WoopIntersection( rayOrig, rayDir,
                                        cubeVertCoord[ triangles[tri1][0] ],
                                        cubeVertCoord[ triangles[tri1][1] ],
                                        cubeVertCoord[ triangles[tri1][2] ],
                                        t, u, v, w                        );
    bool intersect2 = WoopIntersection( rayOrig, rayDir,
                                        cubeVertCoord[ triangles[tri2][0] ],
                                        cubeVertCoord[ triangles[tri2][1] ],
                                        cubeVertCoord[ triangles[tri2][2] ],
                                        t, u, v, w                        );
    return (intersect1 || intersect2);
}

int  FindNextCell( const in ivec4 step1CellIdx, const in vec3 rayOrig, const in vec3 rayDir,
                   out ivec4 step2CellIdx,      out float step2T )
{
    ivec3 cubeVertIdx[8], triangles[12];
    vec3  cubeVertCoord[8];
    FindCellIndices( step1CellIdx, cubeVertIdx, cubeVertCoord, triangles );

    // Test intersection with 12 triangles
    bool  intersect[12];
    float tArr[12],    uArr[12],   vArr[12],    wArr[12];
    for( int i = 0; i < 12; i++ )
    {
        intersect[i] = WoopIntersection( rayOrig,       rayDir,
                                         cubeVertCoord[ triangles[i][0] ],
                                         cubeVertCoord[ triangles[i][1] ],
                                         cubeVertCoord[ triangles[i][2] ],
                                         tArr[i], uArr[i], vArr[i], wArr[i] );
    }

    // Extract which 2 faces the ray is coming in.
    //   If the ray comes in from a face, this vector holds 2 values that are identical.
    //   Otherwise, it has 2 faces that forms an edge.
    int inFace[2];
    if( step1CellIdx.w < 10 )       // A face
    {
        inFace[0] = step1CellIdx.w;
        inFace[1] = step1CellIdx.w;
    }
    else                            // An edge
    {
        inFace[0] = step1CellIdx.w / 10 - 1;
        inFace[1] = step1CellIdx.w % 10;
    }
    
    int numInterceptFace = 0;
    for( int i = 0; i < 6; i++ )
        if( i != step1CellIdx.w && (intersect[2*i] || intersect[2*i+1]) )     
            numInterceptFace++;
    //int i = step1CellIdx.w;
    //if( intersect[2*i] || intersect[2*i+1] )            numInterceptFace++;
    return numInterceptFace;

#if 0
    if( numSmallestT == 1 )
    {
        int   f0 = exitFaces[0];
        ivec3 V0 = step1CellIdx.xyz;

        switch( f0 )
        {
            case 0:                     // exits the front face of current cell
            {                           // enters the back face of the next cell
                step2CellIdx = ivec4( cubeVertIdx[3], 1 );
                break;
            }
            case 1:                     // exits the back face of current cell
            {                           // enters the front face of the next cell
                step2CellIdx = ivec4( V0.x, V0.y, V0.z - 1, 0 );
                break;
            }
            case 4:                     // exits the right face of current cell
            {                           // enters the left face of the next cell
                step2CellIdx = ivec4( cubeVertIdx[1], 5 );
                break;
            }
            case 5:                     // exits the left face of current cell
            {                           // enters the right face of the next cell
                step2CellIdx = ivec4( V0.x - 1, V0.y, V0.z, 4 );
                break;
            }
            case 2:                     // exits the top face of current cell
            {                           // enters the bottom face of the next cell
                step2CellIdx = ivec4(  cubeVertIdx[4], 3 );
                break;
            }
            case 3:                     // exits the bottom face of current cell
            {                           // enters the top face of the next cell
                step2CellIdx = ivec4( V0.x, V0.y - 1, V0.z, 2 );
                break;
            }
            default: 
            {
                step2CellIdx = ivec4( -10 );
            }
        }
    }
    else if ( numSmallestT == 2 )
    {
        // First record the largest t value
        int   f0     = exitFaces[0];
        int   f1     = exitFaces[1];
        ivec3 V0     = step1CellIdx.xyz;

        // Second derive the next step cell
        if( f0 == 0 && f1 == 2 )            // exits edge 7-6
        {                                   // enters edge 0-1
            int back = 1, bottom = 3;
            int edge = (back + 1) * 10 + bottom;
            step2CellIdx = ivec4( cubeVertIdx[7], edge );
        }
        else if( f0 == 0 && f1 == 3 )       // exits edge 3-2
        {                                   // enters edge 4-5
            int back = 1, top = 2;
            int edge = (back + 1) * 10 + top;
            step2CellIdx = ivec4( V0.x, V0.y - 1, V0.z + 1, edge );
        }
        else if( f0 == 0 && f1 == 4 )       // exits edge 6-2
        {                                   // enters edge 4-0
            int back = 1, left = 5;
            int edge = (back + 1) * 10 + left;
            step2CellIdx = ivec4( V0.x + 1, V0.y, V0.z + 1, edge );
        }
        else if( f0 == 0 && f1 == 5 )       // exits edge 7-3
        {                                   // enters edge 5-1
            int back = 1, right = 4;
            int edge = (back + 1) * 10 + right;
            step2CellIdx = ivec4( V0.x - 1, V0.y, V0.z + 1, edge );
        }
        else if( f0 == 1 && f1 == 2 )       // exits edge 4-5
        {                                   // enters edge 3-2
            int front = 0, bottom = 3;
            int edge  = (front + 1) * 10 + bottom;
            step2CellIdx = ivec4( V0.x, V0.y + 1, V0.z - 1, edge );
        }
        else if( f0 == 1 && f1 == 3 )       // exits edge 0-1
        {                                   // enters edge 7-6
            int front = 0, top = 2;
            int edge  = (front + 1) * 10 + top;
            step2CellIdx = ivec4( V0.x, V0.y - 1, V0.z - 1, edge );
        }
        else if( f0 == 1 && f1 == 4 )       // exits edge 5-1
        {                                   // enters edge 7-3
            int front = 0, left = 5;
            int edge  = (front + 1) * 10 + left;
            step2CellIdx = ivec4( V0.x + 1, V0.y, V0.z - 1, edge );
        }
        else if( f0 == 1 && f1 == 5 )       // exits edge 4-0
        {                                   // enters edge 6-2
            int front = 0, right = 4;
            int edge  = (front + 1) * 10 + right;
            step2CellIdx = ivec4( V0.x - 1, V0.y, V0.z - 1, edge );
        }
        else if( f0 == 2 && f1 == 5 )       // exits edge 4-7
        {                                   // enters edge 1-2
            int bottom = 3, right = 4;
            int edge   = (bottom + 1) * 10 + right;
            step2CellIdx = ivec4( V0.x - 1, V0.y + 1, V0.z, edge );
        }
        else if( f0 == 2 && f1 == 4 )       // exits edge 5-6
        {                                   // enters edge 0-3
            int bottom = 3, left = 5;
            int edge   = (bottom + 1) * 10 + left;
            step2CellIdx = ivec4( V0.x + 1, V0.y + 1, V0.z, edge );
        }
        else if( f0 == 3 && f1 == 5 )       // exits edge 0-3
        {                                   // enters edge 5-6
            int top  = 2, right = 4;
            int edge = (top + 1) * 10 + right;
            step2CellIdx = ivec4( V0.x - 1, V0.y - 1, V0.z, edge );
        }
        else if( f0 == 3 && f1 == 4 )       // exits edge 1-2
        {                                   // enters edge 4-7
            int top  = 2, left = 5;
            int edge = (top + 1) * 10 + left;
            step2CellIdx = ivec4( V0.x + 1, V0.y - 1, V0.z, edge );
        }
    }
#endif

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


void main(void)
{
    vec3  lightDirEye   = vec3(0.0, 0.0, 1.0); 

    // Get texture coordinates of this frament
    vec2 fragTexture    = gl_FragCoord.xy / vec2( viewportDims );

    vec3 stopEye        = texture( backFaceTexture,  fragTexture ).xyz;
    vec3 startEye       = texture( frontFaceTexture, fragTexture ).xyz;
    vec3 rayDirEye      = stopEye - startEye;
    float rayDirLength  = length( rayDirEye );
    if(   rayDirLength  < ULP )
        discard;
    InitializeWoopPre( rayDirEye );

    ivec4 step1CellIdx  = provokingVertexIdx;
    ivec4 step2CellIdx  = ivec4( 0 );
    float step1T, step2T;

    color = vec4(0.0);
    bool inFace = InputTest( step1CellIdx, startEye, rayDirEye );
    if( !inFace )
    {
        color = vec4( 0.9, 0.2, 0.2, 1.0 );    // red
        int intersect = FindNextCell( step1CellIdx, startEye, rayDirEye, 
                                      step2CellIdx, step2T );
        if( intersect == 1 ) color.g = 0.9;             // yellow
        else if( intersect == 2 ) color.b = 0.9;        // purple
        else if( intersect > 2 ) color = vec4( 0.9 );
    }


    /* What's the color of Cell 1 
    int intersect = FindNextCell( step1CellIdx, startEye, rayDirEye, 
                                  step2CellIdx, step2T );
    vec3  step1CellCenterTexture = vec3( step1CellIdx.xyz + 1 ) / volumeDimsf;
    float step1Value    = texture( volumeTexture, step1CellCenterTexture ).r;
          color         = texture( colorMapTexture, TranslateValue( step1Value ) );
          color.rgb    *= color.a;

     if( intersect == 0 ) 
         color = vec4( 0.9, 0.2, 0.2, 1.0 );    // red
     else if( intersect == 2 ) 
         color = vec4( 0.2, 0.9, 0.2, 1.0 );    // green
     else if( intersect == 3 )
         color = vec4( 0.99, 0.99, 0.1, 1.0 );    // yellow
     else if( intersect > 3 )
         color = vec4( 0.9, 0.9, 0.9, 1.0 );    // white
    */

#if 0
    // Ray starts from an edge... Need to do something... 
    float biggestT1  = 0.0, biggestT2 = 0.0;
    const float zero = 3e-4;
    if( step2T < zero )
    {
        // try to distinguish the two cubes.
        ivec3 cubeVertIdx1[8],   cubeVertIdx2[8], triangles1[12], triangles2[12];
        vec3  cubeVertCoord1[8], cubeVertCoord2[8];
        FindCellIndices( step1CellIdx, cubeVertIdx1, cubeVertCoord1, triangles1 );
        FindCellIndices( step2CellIdx, cubeVertIdx2, cubeVertCoord2, triangles2 );
        bool  intersect1[12],    intersect2[12];
        float tArr1[12], tArr2[12], uArr1[12], uArr2[12], vArr1[12], vArr2[12];
        for( int i = 0; i < 12; i++ )
        {
            intersect1[i] = RayTriangleIntersect( startTexture, rayDirTexture,
                                                  cubeVertCoord1[ triangles1[i][0] ],
                                                  cubeVertCoord1[ triangles1[i][1] ],
                                                  cubeVertCoord1[ triangles1[i][2] ],
                                                  tArr1[i], uArr1[i], vArr1[i]     );
            intersect2[i] = RayTriangleIntersect( startTexture, rayDirTexture,
                                                  cubeVertCoord2[ triangles2[i][0] ],
                                                  cubeVertCoord2[ triangles2[i][1] ],
                                                  cubeVertCoord2[ triangles2[i][2] ],
                                                  tArr2[i], uArr2[i], vArr2[i]     );
        }
        for( int i = 0; i < 12; i++ )
        {
            if( intersect1[i] && tArr1[i] > biggestT1 )
                biggestT1 = tArr1[i]; 
            if( intersect2[i] && tArr2[i] > biggestT2 ) 
                biggestT2 = tArr2[i]; 
        }

        //   In the following case, re-assign cell 2 as cell 1, and then do one step of traverse.
        //    
        //   | Cell   |  Cell  |
        //   |  2  \  |   1    |
        //   |      \ |        |
        //   |       \|        |
        //   ---------\---------
        if( biggestT1 < biggestT2 )
        {
            int inFace0 = min( step1CellIdx.w, step2CellIdx.w );
            int inFace1 = max( step1CellIdx.w, step2CellIdx.w );
            int inEdge  = (inFace0 + 1) * 10 + inFace1;
            step1CellIdx  = step2CellIdx;
            step1CellIdx.w = inEdge;
            intersect   = FindNextCell( step1CellIdx, startTexture, rayDirTexture, 
                                        0.0, step2CellIdx, step2T, u, v );
        }
        //   In the following case, manipulate step1CellIdx.w, and then re-do one step of traverse.
        //    
        //   | Cell   | Cell   |
        //   |  2     | 1 /    |
        //   |        |  /     |
        //   |        | /      |
        //   ---------|/--------
        else if( biggestT1 > biggestT2 )
        {
            // Find the adjacent face of step2CellIdx.w in cell 1
            int adjacentW = -1;
            switch( step2CellIdx.w )
            {
                case 0: 
                    adjacentW = 1;  break;
                case 1: 
                    adjacentW = 0;  break;
                case 2: 
                    adjacentW = 3;  break;
                case 3: 
                    adjacentW = 2;  break;
                case 4: 
                    adjacentW = 5;  break;
                case 5: 
                    adjacentW = 4;  break;
                    
            }
            int inFace0 = min( step1CellIdx.w, adjacentW );
            int inFace1 = max( step1CellIdx.w, adjacentW );
            int inEdge  = (inFace0 + 1) * 10 + inFace1;
            step1CellIdx.w = inEdge;
            intersect   = FindNextCell( step1CellIdx, startTexture, rayDirTexture, 
                                        0.0, step2CellIdx, step2T, u, v );
        }
    }
#endif


#if 0
    if( !CellOutsideBound( step2CellIdx.xyz ) )
    {
        int  maxCells = 2 * int(length( volumeDimsf ));
        int  counter;
        for( counter = 0; counter < maxCells; counter++ )
        {
            vec3  step2CellCenterTexture = vec3( step2CellIdx.xyz + 1 ) * volumeStepf;
            float step2Value    = texture( volumeTexture, step2CellCenterTexture ).r;
            vec4  backColor     = texture( colorMapTexture, TranslateValue( step2Value ) );
            color.rgb          += (1.0 - color.a) * backColor.a * backColor.rgb;
            color.a            += (1.0 - color.a) * backColor.a;

            step1CellIdx        = step2CellIdx;
            step1T              = step2T;
            intersect           = FindNextCell( step1CellIdx, origTexture, rayDirTexture, 
                                                step1T, step2CellIdx, step2T           );

            if( CellOutsideBound( step2CellIdx.xyz ) )
            {
                step2CellIdx = step1CellIdx;
                break;
            }
        }
    }
#endif


#if 0
    float nStepsf       = rayDirLength  / stepSize1D;
    vec3  stepSize3D    = rayDirTexture / nStepsf;

    vec3  step1Texture  = startTexture;
    if( ShouldSkip( step1Texture ) )
    {
        color = vec4( 0.0f );
    }
    else
    {
        float step1Value = texture( volumeTexture, step1Texture ).r;
              color      = texture( colorMapTexture, TranslateValue(step1Value) );
              color.rgb *= color.a;
    }

    // let's do a ray casting! 
    for( int i = 0; i < int(nStepsf); i++ )
    {
        if( color.a > 0.999f )  // You can still see through with 0.99...
            break;

        vec3 step2Texture = startTexture + stepSize3D * float(i + 1);
        if( ShouldSkip( step2Texture ) )
            continue;

        float step2Value  = texture( volumeTexture, step2Texture ).r;
        vec4  backColor   = texture( colorMapTexture, TranslateValue(step2Value) );
        
        // Apply lighting if big enough gradient
        if( lighting )
        {
            vec3 gradientModel = CalculateGradient( step2Texture );
            if( length( gradientModel ) > EPSILON )
            {
                vec3 gradientEye = (transposedInverseMV * vec4( gradientModel, 0.0f )).xyz;
                     gradientEye = normalize( gradientEye );
                float diffuse    = abs( dot(lightDirEye, gradientEye) );

                // Calculate eye space coordinates of "step2Texture"
                vec3 step2Model  = boxMin + step2Texture * (boxMax - boxMin);
                vec3 step2Eye    = (ModelView * vec4(step2Model, 1.0f)).xyz;
                vec3 viewDirEye  = normalize( -step2Eye );

                vec3 reflectionEye = reflect( -lightDirEye, gradientEye );
                float specular   = pow( max(0.0f, dot( reflectionEye, viewDirEye )), specularExp ); 
                backColor.rgb    = backColor.rgb * (ambientCoeff + diffuse * diffuseCoeff) + 
                                   specular * specularCoeff;
            }
        }   // End lighting

        // Color compositing
        color.rgb += (1.0f - color.a) * backColor.a * backColor.rgb;
        color.a   += (1.0f - color.a) * backColor.a;
    }   // End ray casting
#endif

}

