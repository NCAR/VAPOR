#version 410 core

#include RayMath.frag

uniform mat4 MVP;
// uniform vec2 resolution;
uniform vec3 cameraPos;
uniform vec3 dataBoundsMin;
uniform vec3 dataBoundsMax;
uniform float LUTMin;
uniform float LUTMax;
bool readDepthBuffer = true;

uniform sampler3D data;
uniform sampler1D LUT;
uniform sampler2D sceneDepth;

in vec2 ST;

out vec4 fragColor;

vec4 ROYGBV(float v, float minV, float maxV)
{
    vec3 colors[6];
    colors[0] = vec3(1.00, 0.00, 0.00);
    colors[1] = vec3(1.00, 0.60, 0.00);
    colors[2] = vec3(1.00, 1.00, 0.00);
    colors[3] = vec3(0.00, 1.00, 0.00);
    colors[4] = vec3(0.00, 0.00, 1.00);
    colors[5] = vec3(0.32, 0.00, 0.32);
    float ratio = 5.0 * clamp((v-minV)/(maxV-minV), 0, 1);
    int indexMin=int(floor(ratio));
    int indexMax=min(int(indexMin)+1,5);
    vec3 c = mix(colors[indexMin], colors[indexMax], ratio-indexMin);
    return vec4(c, 1);
}

vec3 GetNormal(vec3 p)
{
    vec3 dims = vec3(textureSize(data, 0));
    vec3 d = 1/dims * 0.5;
    vec3 s0, s1;
    s1.x = texture(data, p + d*vec3(1,0,0)).r;
    s1.y = texture(data, p + d*vec3(0,1,0)).r;
    s1.z = texture(data, p + d*vec3(0,0,1)).r;
    s0.x = texture(data, p - d*vec3(1,0,0)).r;
    s0.y = texture(data, p - d*vec3(0,1,0)).r;
    s0.z = texture(data, p - d*vec3(0,0,1)).r;
    return normalize(s1-s0);
    //return s1-s0;
}

vec3 lightDir = vec3(0,0,-1);

float Shadow(vec3 to) {
    float t = 0, t0, t1;
    IntersectRayBoundingBox(to, -lightDir, 0, vec3(0), vec3(1), t0, t1);
    //return t1;
    
    float acc = 0;
    for (; t < t1; t+= 0.05) {
        float dataNorm = (texture(data, to-t*lightDir).r - LUTMin) / (LUTMax - LUTMin);
        float opacity = texture(LUT, dataNorm).a;
        acc += opacity * (1-acc);
    }
    return 1-acc;
}

float GetDepthBuffer()
{
    if (readDepthBuffer) {
        float depth = texture(sceneDepth, ST).r;
        return depth * 2 - 1; // back to NDC
    } else {
        return 1;
    }
}

void main(void)
{
    vec2 screen = ST*2-1;
    vec4 world = inverse(MVP) * vec4(screen, GetDepthBuffer(), 1);
    world /= world.w;
    vec3 dir = normalize(world.xyz - cameraPos);
    float sceneDepthT = length(world.xyz - cameraPos);
    
    vec4 accum = vec4(0);
    float t0, t1;
    
    if (IntersectRayBoundingBox(cameraPos, dir, 0, dataBoundsMin, dataBoundsMax, t0, t1)) {
        
#define STEPS 100
        float step = max(((t1-t0)/float(STEPS))*1.01, (dataBoundsMax[2]-dataBoundsMin[2])/float(STEPS));
        
        int i = 0;
        for (float t = t0; t < t1 && t < sceneDepthT; t += step) {
            
            vec3 hit = cameraPos + dir * t;
            vec3 dataSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
            float dataNorm = (texture(data, dataSTR).r - LUTMin) / (LUTMax - LUTMin);
            vec4 color = texture(LUT, dataNorm);
            vec3 normal = GetNormal(dataSTR);
            
            accum.rgb += color.rgb * color.a * (1-accum.a);
            accum.a += color.a * (1-accum.a);
            
            if (accum.a > 0.999)
                break;
                
            // Failsafe
            if (i++ > STEPS)
                break;
        }
        
        fragColor = accum;
    }
        
    if (accum.a < 0.1)
        discard;
}
