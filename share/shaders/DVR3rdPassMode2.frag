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
uniform vec3  boxMin;
uniform vec3  boxMax;
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
vec3  volumeDimsf      = vec3( volumeDims );
vec3  boxSpan          = boxMax - boxMin;

//
// Code for faces and edges
//
const int FaceFront    = 0;
const int FaceBack     = 1;
const int FaceTop      = 2;
const int FaceBottom   = 3;
const int FaceLeft     = 4;
const int FaceRight    = 5;
const int Edge01       = (1 + 1) * 10 + 3;
const int Edge45       = (1 + 1) * 10 + 2;
const int Edge04       = (1 + 1) * 10 + 5;
const int Edge15       = (1 + 1) * 10 + 4;
const int Edge23       = (0 + 1) * 10 + 3;
const int Edge67       = (0 + 1) * 10 + 2;
const int Edge37       = (0 + 1) * 10 + 5;
const int Edge26       = (0 + 1) * 10 + 4;
const int Edge12       = (3 + 1) * 10 + 4;
const int Edge03       = (3 + 1) * 10 + 5;
const int Edge56       = (2 + 1) * 10 + 4;
const int Edge47       = (2 + 1) * 10 + 5;

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
// Input:  logical index of a vertex
// Output: user coordinates in the eye space
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

// 
// Woop ray-triangle intersection algorithm.
// Note: woopPre must be initialized by InitializeWoopPre() before invoking this function.
//
int   WoopIntersection( const in vec3 orig, const in vec3 dir,
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

    /* Fall back to double precision.
       This is probably not necessary when the subsequent edge test compares with ULP.
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
        return 1;

    // Parallel test
    float det = U + V + W;
    if( abs(det) < ULP )
        return 10;

    float det1o = 1.0 / det;
    float Az = woopPre.Sz * A[ woopPre.kz ];
    float Bz = woopPre.Sz * B[ woopPre.kz ];
    float Cz = woopPre.Sz * C[ woopPre.kz ];
    float T  = fma( U, Az, fma( V, Bz, W * Cz ) );
    outT     = T * det1o;
    outU     = U * det1o;
    outV     = V * det1o;
    outW     = W * det1o;

    return 0;
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

bool CellOutsideBound( const in ivec3 cellIdx )
{
    if( cellIdx.x < 0 || cellIdx.x > volumeDims.x - 2 || 
        cellIdx.y < 0 || cellIdx.y > volumeDims.y - 2 ||
        cellIdx.z < 0 || cellIdx.z > volumeDims.z - 2   )
        return true;
    else
        return false;
}

int InputTest( const in ivec4 cellIdx, const in vec3 rayOrig, const in vec3 rayDir )
{
    if( CellOutsideBound( cellIdx.xyz ) )
        return -2;

    ivec3 cubeVertIdx[8], triangles[12];
    vec3  cubeVertCoord[8];
    FindCellIndices( cellIdx, cubeVertIdx, cubeVertCoord, triangles );
    int tri1 = cellIdx.w * 2;
    int tri2 = cellIdx.w * 2 + 1;
    int intersect[12];
    float t, u, v, w;

    // First test the two indicated incoming triangles.
    // If either reports a hit, this test succeeds.
    intersect[tri1] = WoopIntersection( rayOrig, rayDir,
                                        cubeVertCoord[ triangles[tri1][0] ],
                                        cubeVertCoord[ triangles[tri1][1] ],
                                        cubeVertCoord[ triangles[tri1][2] ],
                                        t, u, v, w                        );
    intersect[tri2] = WoopIntersection( rayOrig, rayDir,
                                        cubeVertCoord[ triangles[tri2][0] ],
                                        cubeVertCoord[ triangles[tri2][1] ],
                                        cubeVertCoord[ triangles[tri2][2] ],
                                        t, u, v, w                        );
    if( intersect[tri1] == 0 || intersect[tri2] == 0 )
        return 0;

    // Second we test exit triangles. 
    // If the ray exits this cell, the test also succeeds
    for( int i = 0; i < 12; i++ )
        if( i != tri1 && i != tri2 )
        {
            intersect[i] = WoopIntersection( rayOrig, rayDir,
                                             cubeVertCoord[ triangles[i][0] ],
                                             cubeVertCoord[ triangles[i][1] ],
                                             cubeVertCoord[ triangles[i][2] ],
                                             t, u, v, w                        );
            if( intersect[i] == 0 )
                return 0;
        }

    // Now we know this ray carries a wrong index!
    return -1;
}

int  FindNextCell( const in ivec4 step1CellIdx, const in vec3 rayOrig,  const in vec3 rayDir,
                   out ivec4 step2CellIdx,      out float step2T,       in bool checkInFace )
{
    ivec3 cubeVertIdx[8], triangles[12];
    vec3  cubeVertCoord[8];
    FindCellIndices( step1CellIdx, cubeVertIdx, cubeVertCoord, triangles );

    // Test intersection with 12 triangles
    int   intersect[12];
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

    // Check if the ray hits the supposed incoming faces
    if( checkInFace )
    {
        if( intersect[inFace[0]] != 0 && intersect[inFace[1]] != 0 )
            return -1;
    }
    
    int numInterceptFace = 0;
    for( int i = 0; i < 6; i++ )
        if( i != inFace[0] && i != inFace[1] && (intersect[2*i] == 0 || intersect[2*i+1] == 0) )     
            numInterceptFace++;
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



void main(void)
{
    color               = vec4( 0.0 );
    vec3  lightDirEye   = vec3(0.0, 0.0, 1.0); 

    // Get texture coordinates of this frament
    vec2 fragTexture    = gl_FragCoord.xy / vec2( viewportDims );

    vec3 stopModel        = texture( backFaceTexture,  fragTexture ).xyz;
    vec3 startModel       = texture( frontFaceTexture, fragTexture ).xyz;
    vec3 rayDirModel      = stopModel - startModel;
    float rayDirLength  = length( rayDirModel );
    if(   rayDirLength  < ULP )
        discard;
    // Initialize Woop constants right after defining the ray direction
    InitializeWoopPre( rayDirModel );

    ivec4 step1CellIdx  = provokingVertexIdx;
    ivec4 step2CellIdx  = ivec4( 0 );
    float step1T, step2T;
    vec3  step1Model    = startModel - 0.01 * rayDirModel;

    // Correct starting cell index when necessary
    if( InputTest( step1CellIdx, step1Model, rayDirModel ) != 0 )
    {
        ivec4 badCell  = step1CellIdx;
        ivec4 goodCell = step1CellIdx;
        ivec4 left, right, top, bottom;
        switch( badCell.w / 2 )
        {
            case 0:     // front or back face
                left    = ivec4( badCell.x - 1, badCell.yzw );
                if( InputTest(   left, step1Model, rayDirModel ) == 0 )
                    { goodCell = left;  break; }

                right   = ivec4( badCell.x + 1, badCell.yzw );
                if( InputTest(   right, step1Model, rayDirModel ) == 0 )
                    { goodCell = right; break; }

                top     = ivec4( badCell.x, badCell.y + 1, badCell.zw );
                if( InputTest(   top, step1Model, rayDirModel ) == 0 )
                    { goodCell = top; break; }

                bottom  = ivec4( badCell.x, badCell.y - 1, badCell.zw );
                if( InputTest(   bottom, step1Model, rayDirModel ) == 0 )
                    { goodCell = bottom; break; }

                break;

            case 1:     // top or bottom face
                left    = ivec4( badCell.x - 1, badCell.yzw );
                if( InputTest(   left, step1Model, rayDirModel ) == 0 )
                    { goodCell = left; break; }

                right   = ivec4( badCell.x + 1, badCell.yzw );
                if( InputTest(   right, step1Model, rayDirModel ) == 0 )
                    { goodCell = right; break; }

                top     = ivec4( badCell.xy, badCell.z - 1, badCell.w );
                if( InputTest(   top, step1Model, rayDirModel ) == 0 ) 
                    { goodCell = top; break; }

                bottom  = ivec4( badCell.xy, badCell.z + 1, badCell.w );
                if( InputTest(   bottom, step1Model, rayDirModel ) == 0 )
                    { goodCell = bottom; break; }

                break;

            case 2:     // left or right face
                left    = ivec4( badCell.xy, badCell.z - 1, badCell.w );
                if( InputTest(   left, step1Model, rayDirModel ) == 0 )
                    { goodCell = left; break; }

                right   = ivec4( badCell.xy, badCell.z + 1, badCell.w );
                if( InputTest(   right, step1Model, rayDirModel ) == 0 )
                    { goodCell = right; break; }

                top     = ivec4( badCell.x, badCell.y + 1, badCell.zw );
                if( InputTest(   top, step1Model, rayDirModel ) == 0 )
                    { goodCell = top; break; }

                bottom  = ivec4( badCell.x, badCell.y - 1, badCell.zw );
                if( InputTest(   bottom, step1Model, rayDirModel ) == 0 )
                    { goodCell = bottom; break; }

                break;
        }
        step1CellIdx = goodCell;
    }

    // Cell traverse -- 1st cell
    int numOfIntersect = FindNextCell( step1CellIdx, step1Model, rayDirModel, step2CellIdx, step2T, false );
    switch( numOfIntersect )
    {
        case 0:  color = vec4( 0.9, 0.2, 0.2, 1.0 ); break;  // red
        case 1:  color = vec4( 0.2, 0.2, 0.2, 0.2 ); break;  // almost black
        case 2:  color = vec4( 0.2, 0.9, 0.2, 1.0 ); break;  // green
        case 3:  color = vec4( 0.2, 0.2, 0.9, 1.0 ); break;  // blue
        default: color = vec4( 1.0 ); break;    // white
    }
    


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

