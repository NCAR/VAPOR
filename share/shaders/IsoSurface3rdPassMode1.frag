#version 410 core

in vec4 gl_FragCoord;
layout(location = 0) out vec4 color;

uniform sampler2D  backFaceTexture;
uniform sampler2D  frontFaceTexture;
uniform sampler3D  volumeTexture;
uniform usampler3D missingValueMaskTexture; // !!unsigned integer!!
uniform sampler1D  colorMapTexture;
uniform sampler3D  secondVarDataTexture;
uniform usampler3D secondVarMaskTexture; // !!unsigned integer!!

uniform ivec3 volumeDims;        // number of vertices of this volumeTexture
uniform ivec2 viewportDims;      // width and height of this viewport
uniform vec4  clipPlanes[6];     // clipping planes in model coordinates
uniform vec3  boxMin;            // min coordinates of the bounding box of this volume
uniform vec3  boxMax;            // max coordinates of the bounding box of this volume
uniform vec3  colorMapRange;     // min and max and diff values on this color map

uniform float stepSize1D;        // ray casting step size
uniform float lightingCoeffs[4]; // lighting parameters
uniform bool  flags[4];
uniform bool  use2ndVar;

uniform int   numOfIsoValues;    // how many iso values are valid in isoValues array?
uniform float isoValues[4];      // currently set there are at most 4 iso values.

uniform mat4 MV;
uniform mat4 inversedMV;
uniform mat4 Projection;

//
// Derive helper variables
//
const float ULP        = 1.2e-7f;
const float ULP10      = 1.2e-6f;
const float Opaque     = 0.999;
bool  fast             = flags[0];
bool  lighting         = flags[1];
bool  eyeInsideVolume  = flags[2];
bool  hasMissingValue  = flags[3];
float ambientCoeff     = lightingCoeffs[0];
float diffuseCoeff     = lightingCoeffs[1];
float specularCoeff    = lightingCoeffs[2];
float specularExp      = lightingCoeffs[3];
vec3  volumeDims1o     = 1.0 / vec3( volumeDims - 1 );
vec3  boxSpan          = boxMax - boxMin;
mat4  transposedInverseMV = transpose( inversedMV );

//
// Input:  Location to be evaluated in texture coordinates.
// Output: If this location should be skipped.
// Note:   It is skipped in two cases: 1) it represents a missing value
//                                     2) it is outside of clipping planes
//
bool ShouldSkip( const in vec3 tc, const in vec3 mc )
{
    if( hasMissingValue && (texture(missingValueMaskTexture, tc).r != 0u) )
        return true;

    if( use2ndVar && (texture(secondVarMaskTexture, tc).r != 0u) )
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
vec3 CalculateGradient( const in vec3 tc )
{
    vec3 h0 = vec3(-0.5 ) * volumeDims1o;
    vec3 h1 = vec3( 0.5 ) * volumeDims1o;
    vec3 h  = vec3( 1.0 );

    if ((tc.x + h0.x) < 0.0) {
        h0.x = 0.0;
        h.x  = 0.5;
    }
    if ((tc.x + h1.x) > 1.0) {
        h1.x = 0.0;
        h.x  = 0.5;
    }
    if ((tc.y + h0.y) < 0.0) {
        h0.y = 0.0;
        h.y  = 0.5;
    }
    if ((tc.y + h1.y) > 1.0) {
        h1.y = 0.0;
        h.y  = 0.5;
    }
    if ((tc.z + h0.z) < 0.0) {
        h0.z = 0.0;
        h.z  = 0.5;
    }
    if ((tc.z + h1.z) > 1.0) {
        h1.z = 0.0;
        h.z  = 0.5;
    }

    vec3 a0, a1;
    a0.x = texture( volumeTexture, tc + vec3(h0.x, 0.0f, 0.0f) ).r;
    a1.x = texture( volumeTexture, tc + vec3(h1.x, 0.0f, 0.0f) ).r;
    a0.y = texture( volumeTexture, tc + vec3(0.0f, h0.y, 0.0f) ).r;
    a1.y = texture( volumeTexture, tc + vec3(0.0f, h1.y, 0.0f) ).r;
    a0.z = texture( volumeTexture, tc + vec3(0.0f, 0.0f, h0.z) ).r;
    a1.z = texture( volumeTexture, tc + vec3(0.0f, 0.0f, h1.z) ).r;

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


void main(void)
{
    color               = vec4( 0.0 );
    vec3  lightDirEye   = vec3(0.0, 0.0, 1.0); 

    // Get texture coordinates of this fragment
    vec2 fragTexture    = gl_FragCoord.xy / vec2( viewportDims );

    vec3 stopEye        = texture( backFaceTexture,  fragTexture ).xyz;
    vec3 startEye       = vec3( 0.0 );
    if( !eyeInsideVolume )
         startEye       = texture( frontFaceTexture, fragTexture ).xyz;
    vec3 rayDirEye      = stopEye - startEye;
    float rayDirLength  = length( rayDirEye );
    if( rayDirLength    < ULP10 )
        discard;

    float nStepsf       = rayDirLength  / stepSize1D;
    vec3  stepSize3D    = rayDirEye   / nStepsf;
    int   nSteps        = int(nStepsf) + 1;

    // Set depth value at the backface minus 1/100 of a step size,
    //   so it's always inside of the volume.
    gl_FragDepth        =  CalculateDepth( stopEye ) - ULP10;

    // Shift the starting point (step1) inside of the volume for 1/100 of a step size
    //   to prevent potential boundary artifacts.
    vec3  step1Eye      = startEye    + 0.01 * stepSize3D;
    vec3  step1Model    = (inversedMV * vec4(step1Eye, 1.0)).xyz;
    vec3  step1Texture  = (step1Model - boxMin) / boxSpan;
    float step1Value    = texture( volumeTexture, step1Texture ).r;

    // let's do a ray casting! 
    for( int stepi = 1; stepi <= nSteps; stepi++ )
    {
        if( color.a > Opaque )
            break;

        vec3 step2Eye     = startEye    + stepSize3D * float( stepi );
        vec3 step2Model   = (inversedMV * vec4(step2Eye, 1.0)).xyz;
        vec3 step2Texture = (step2Model - boxMin) / boxSpan;
        float step2Value  = texture( volumeTexture, step2Texture ).r;
        if( ShouldSkip( step2Texture, step2Model ) )
        {
            step1Eye      = step2Eye;
            step1Model    = step2Model;
            step1Texture  = step2Texture;
            step1Value    = step2Value;
            continue;
        }

        for( int j = 0; j < numOfIsoValues; j++ )
        {
            float isoValJ = isoValues[j];
            if( ( isoValJ - step1Value) * (isoValJ - step2Value) < 0.0 )
            {
                float weight    = (isoValJ - step1Value) / (step2Value - step1Value);
                vec3  isoEye    = mix( step1Eye, step2Eye, weight );
                vec3  isoTex    = mix( step1Texture, step2Texture, weight );

                // Retrieve data value of the secondary variable at the same location
                //   if color mapping variable is enabled.
                if( use2ndVar )
                {
                    isoValJ     = texture( secondVarDataTexture,   isoTex ).x;
                }

                float valTrans  = (isoValJ - colorMapRange.x) / colorMapRange.z;
                vec4  backColor = texture( colorMapTexture, valTrans );

                // Apply lighting (in eye space)
                if( lighting && backColor.a > (1.0 - Opaque) )
                {
                    vec3 gradientModel   = CalculateGradient( isoTex );
                    if( length( gradientModel ) > ULP10 ) // Only apply lighting if big enough gradient
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

            } // Finish rendering one iso-surface.
        }     // Finish testing all iso-values.

        step1Eye     = step2Eye;
        step1Model   = step2Model;
        step1Texture = step2Texture;
        step1Value   = step2Value;
    }   // Finish ray casting

    // If this pixel is almost transparent, set the depth at the far clipping plane
    if( color.a < (1.0 - Opaque) )
        gl_FragDepth = 1.0;
}

