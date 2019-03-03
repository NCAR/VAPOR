#version 410 core

#include RayMath.frag

uniform mat4 MVP;
// uniform vec2 resolution;
uniform vec3 cameraPos;
uniform vec3 dataBoundsMin;
uniform vec3 dataBoundsMax;
uniform float LUTMin;
uniform float LUTMax;
uniform float isoValue;

uniform sampler3D data;
uniform sampler1D LUT;

in vec2 ST;

out vec4 fragColor;

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

#define AMBIENT 0.2
#define DIFFUSE 0.5
#define SPECULAR 0.25
#define SHININESS 8

float PhongLighting(vec3 normal, vec3 viewDir)
{
    float diffuse = abs(dot(normal, -lightDir)) * DIFFUSE;
    
    float specularStrength = SPECULAR;
    vec3 reflectDir = reflect(lightDir, normal);
    float spec = pow(abs(dot(viewDir, reflectDir)), SHININESS);
    float specular = specularStrength * spec;
    
    return AMBIENT + diffuse + specular;
}

void main(void)
{
    vec2 screen = ST*2-1;
    vec4 world = inverse(MVP) * vec4(screen, 1, 1);
    world /= world.w;
    vec3 dir = normalize(world.xyz - cameraPos);
    
    vec4 accum = vec4(0);
    float t0, t1;
    
    if (IntersectRayBoundingBox(cameraPos, dir, 0, dataBoundsMin, dataBoundsMax, t0, t1)) {

#define STEPS 100
        float step = max(((t1-t0)/float(STEPS))*1.01, (dataBoundsMax[2]-dataBoundsMin[2])/float(STEPS));
        float ld = texture(data, ((cameraPos + dir * t0) - dataBoundsMin) / (dataBoundsMax-dataBoundsMin)).r;
        
        int i = 0;
        for (float t = t0; t < t1; t += step) {
            vec3 hit = cameraPos + dir * t;
            vec3 dataSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
            float dv = texture(data, dataSTR).r;
            
            if ((ld < isoValue && dv >= isoValue) || (ld > isoValue && dv <= isoValue)) {
                float lt = t - step;
                float t = lt + step*(isoValue-ld)/(dv-ld);
                
                vec3 hit = cameraPos + dir * t;
                vec3 dataSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
                float data = texture(data, dataSTR).r;
                float dataNorm = (data - LUTMin) / (LUTMax - LUTMin);
                vec4 color = vec4(1);
                vec3 normal = GetNormal(dataSTR);
                
                color.rgb *= PhongLighting(normal, dir);
                
                accum.rgb += color.rgb * color.a * (1-accum.a);
                accum.a += color.a * (1-accum.a);
            }
            ld = dv;
            
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
