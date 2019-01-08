#version 410 core

in vec4 gl_FragCoord;
layout(location = 0) out vec4 color;

uniform sampler2D  backFaceTexture;
uniform sampler2D  frontFaceTexture;
uniform sampler3D  volumeTexture;
uniform usampler3D missingValueMaskTexture; // !!unsigned integer!!
uniform sampler1D  colorMapTexture;
uniform sampler2D  depthTexture;

uniform ivec3 volumeDims;        // number of vertices of this volumeTexture
uniform ivec2 viewportDims;      // width and height of this viewport
uniform vec4  clipPlanes[6];     // clipping planes in model coordinates
uniform vec3  boxMin;            // min coordinates of the bounding box of this volume
uniform vec3  boxMax;            // max coordinates of the bounding box of this volume
uniform vec3  colorMapRange;

uniform float stepSize1D;
uniform bool  flags[3];
uniform float lightingCoeffs[4];

uniform mat4 MV;
uniform mat4 inversedMV;
uniform mat4 Projection;

// 
// Derive helper variables
//
const float ULP        = 1.2e-7f;
const float ULP10      = 1.2e-6f;
const float Opaque     = 0.999;  // You can still see something with 0.99...
bool  fast             = flags[0];                  // fast rendering mode
bool  lighting         = fast ? false : flags[1];   // no lighting in fast mode
bool  hasMissingValue  = flags[2];                  // has missing values or not
float ambientCoeff     = lightingCoeffs[0];
float diffuseCoeff     = lightingCoeffs[1];
float specularCoeff    = lightingCoeffs[2];
float specularExp      = lightingCoeffs[3];
vec3  volumeDims1o     = 1.0 / vec3( volumeDims - 1 );
vec3  boxSpan1o        = 1.0 / (boxMax - boxMin);
mat4  transposedInverseMV = transpose( inversedMV );

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

    // Now we know that this position shouldn't be skipped!
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
    return (gl_DepthRange.diff * 0.5 * pNdc.z + (gl_DepthRange.near + gl_DepthRange.far) * 0.5);
}


void main(void)
{
    color               = vec4( 0.0 );
    vec3  lightDirEye   = vec3(0.0, 0.0, 1.0); 

    // Calculate texture coordinates of this fragment
    vec2 fragTex        = gl_FragCoord.xy / vec2( viewportDims );

    vec3 stopEye        = texture( backFaceTexture,  fragTex ).xyz;
    vec3 startEye       = texture( frontFaceTexture, fragTex ).xyz;
    vec3 rayDirEye      = stopEye - startEye ;
    float rayDirLength  = length( rayDirEye );
    if( rayDirLength    < ULP10 )
        discard;

    float nStepsf       = rayDirLength / stepSize1D;
    vec3  stepSize3D    = rayDirEye / nStepsf;
    // nStepsf is the perfect # of steps.
    //   Casting it to integer requires a +1 to cover all volume space.
    int   nSteps        = int(nStepsf) + 1;     

    // Set depth value at the backface minus a tiny value, so it's always inside of the volume.
    gl_FragDepth     =  CalculateDepth( stopEye ) - ULP10;

    // If something else on the scene resulting in a shallower depth, we need to 
    //    compare depth at every step.
    bool  shallow    = false;
    float otherDepth = texture( depthTexture, fragTex ).x;
    if(   otherDepth < gl_FragDepth )
          shallow    = true;

    // Now we need to query the color at the starting point.
    //   However, to prevent unpleasant boundary artifacts, we shift the starting point
    //   into the volume for 1/100 of a step size.
    vec3  step1Eye         = startEye    + 0.01 * stepSize3D;
    vec3  step1Model       = (inversedMV * vec4( step1Eye, 1.0 )).xyz;
    vec3  step1Texture     = (step1Model - boxMin) * boxSpan1o;
    if( !ShouldSkip( step1Texture, step1Model ) )
    {
        float step1Value   = texture( volumeTexture, step1Texture ).r;
        float valTranslate = (step1Value - colorMapRange.x) / colorMapRange.z;
              color        = texture( colorMapTexture, valTranslate );
              color.rgb   *= color.a;
    }

    // let's do a ray casting! 
    vec3 step2Model        = step1Model;
    bool earlyTerm         = false;         // loop early terminated 
    for( int stepi = 1; stepi <= nSteps; stepi++ )  // notice that stepi starts from 1.
    {
        if( color.a > Opaque )
        { 
            earlyTerm      = true;
            break;
        }

        vec3 step2Eye      = startEye  + stepSize3D * float( stepi );

        if( shallow && ( CalculateDepth(step2Eye) > otherDepth ) )
        { 
            earlyTerm      = true;
            break;
        }

        vec3 step2Model    = (inversedMV * vec4(step2Eye, 1.0)).xyz;
        vec3 step2Texture  = (step2Model - boxMin) * boxSpan1o;
        if( ShouldSkip( step2Texture, step2Model ) )
        {
            step1Model     = step2Model;
            continue;
        }

        float step2Value   = texture( volumeTexture, step2Texture ).r;
        float valTranslate = (step2Value - colorMapRange.x) / colorMapRange.z;
        vec4  backColor    = texture( colorMapTexture, valTranslate );
        
        // Apply lighting. 
        //   Note we do the calculation in eye space, because both the light direction
        //   and view direction are defined in the eye space.
        if( lighting && (backColor.a > 1.0 - Opaque) )
        {
            vec3 gradientModel = CalculateGradient( step2Texture );
            if( length( gradientModel ) > ULP10 )
            {
                vec3 gradientEye = (transposedInverseMV * vec4( gradientModel, 0.0 )).xyz;
                     gradientEye = normalize( gradientEye );
                float diffuse    = abs( dot(lightDirEye, gradientEye) );
                vec3 viewDirEye  = normalize( -step2Eye );
                vec3 reflectionEye = reflect( -lightDirEye, gradientEye );
                float specular   = pow( max(0.0, dot( reflectionEye, viewDirEye )), specularExp ); 
                backColor.rgb    = backColor.rgb * (ambientCoeff + diffuse * diffuseCoeff) + 
                                   specular * specularCoeff;
            }
        }

        // Color compositing
        color.rgb += (1.0 - color.a) * backColor.a * backColor.rgb;
        color.a   += (1.0 - color.a) * backColor.a;

        step1Eye   = step2Eye;
        step1Model = step2Model;
    }

    // If loop terminated early, we set depth value at step1 position. Otherwise, this fragment 
    //    will have the depth value at the back of this volume, which is already set.
    if( earlyTerm )
        gl_FragDepth = CalculateDepth( step1Eye );

    // If this pixel is almost transparent, set the depth at the far clipping plane
    if( color.a < (1.0 - Opaque) )
        gl_FragDepth = 1.0;
}

