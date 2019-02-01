#version 330 core

#include RayMath.frag

uniform mat4 MVP;
// uniform vec2 resolution;
uniform vec3 cameraPos;
uniform vec3 dataBoundsMin;
uniform vec3 dataBoundsMax;
uniform float LUTMin;
uniform float LUTMax;

uniform ivec3 coordDims;
vec3 coordDimsF = vec3(coordDims);

uniform sampler3D data;
uniform sampler1D LUT;
uniform sampler3D coords;

in vec2 ST;

out vec4 fragColor;

#define F_LEFT ivec3(-1, 0, 0)
#define F_RIGHT ivec3( 1, 0, 0)
#define F_UP ivec3( 0, 1, 0)
#define F_DOWN ivec3( 0,-1, 0)
#define F_FRONT ivec3( 0, 0,-1)
#define F_BACK ivec3( 0, 0, 1)

void GetFaceCoordinateIndices(ivec3 cell, ivec3 face, out ivec3 i0, out ivec3 i1, out ivec3 i2, out ivec3 i3)
{
    // CCW
    if (face == F_DOWN) {
        i0 = cell + ivec3(0, 0, 0);
        i1 = cell + ivec3(0, 1, 0);
        i2 = cell + ivec3(1, 1, 0);
        i3 = cell + ivec3(1, 0, 0);
    }
    else if (face == F_UP) {
        i0 = cell + ivec3(0, 0, 1);
        i1 = cell + ivec3(1, 0, 1);
        i2 = cell + ivec3(1, 1, 1);
        i3 = cell + ivec3(0, 1, 1);
    }
    else if (face == F_LEFT) {
        i0 = cell + ivec3(0, 0, 0);
        i1 = cell + ivec3(0, 0, 1);
        i2 = cell + ivec3(0, 1, 1);
        i3 = cell + ivec3(0, 1, 0);
    }
    else if (face == F_RIGHT) {
        i0 = cell + ivec3(1, 0, 0);
        i1 = cell + ivec3(1, 1, 0);
        i2 = cell + ivec3(1, 1, 1);
        i3 = cell + ivec3(1, 0, 1);
    }
    else if (face == F_FRONT) {
        i0 = cell + ivec3(0, 0, 0);
        i1 = cell + ivec3(1, 0, 0);
        i2 = cell + ivec3(1, 0, 1);
        i3 = cell + ivec3(0, 0, 1);
    }
    else if (face == F_BACK) {
        i0 = cell + ivec3(0, 1, 0);
        i1 = cell + ivec3(0, 1, 1);
        i2 = cell + ivec3(1, 1, 1);
        i3 = cell + ivec3(1, 1, 0);
    }
}

bool IntersectRayCellFace(vec3 o, vec3 d, ivec3 cellIndex, ivec3 face, out float t)
{
    ivec3 i0, i1, i2, i3;
    GetFaceCoordinateIndices(cellIndex, face, i0, i1, i2, i3);
    return IntersectRayQuad(o, d,
        texture(coords, i0/coordDimsF).xyz,
        texture(coords, i1/coordDimsF).xyz,
        texture(coords, i2/coordDimsF).xyz,
        texture(coords, i3/coordDimsF).xyz,
        t);
}

void FindNextCell(vec3 origin, vec3 dir, float t, ivec3 currentCell)
{
    
}

void main(void)
{
    vec2 screen = ST*2-1;
    vec4 world = inverse(MVP) * vec4(screen, 1, 1);
    world /= world.w;
    vec3 dir = normalize(world.xyz - cameraPos);
    
    vec4 accum = vec4(0);
    float t0, t1, tp;
    
    bool intersectBox = IntersectRayBoundingBox(cameraPos, dir, dataBoundsMin, dataBoundsMax, t0, t1);
    
    if (intersectBox) {
        
        for (int y = 0; y < coordDims[1]-1; y++) {
            for (int x = 0; x < coordDims[0]-1; x++) {
                float t;
                if (IntersectRayCellFace(cameraPos, dir, ivec3(x, y, 0), F_DOWN, t)) {
                        float dataNorm = (texture(data, (vec3(x,y,0)+vec3(0.5))/(coordDims-1)).r - LUTMin) / (LUTMax - LUTMin);
                        fragColor = texture(LUT, dataNorm);
                        
                        ivec3 cellIndex = ivec3(x, y, 0);
                        
                        return;
                }
            }
        }
        
        float step = max(((t1-t0)/100.f)*1.01, (dataBoundsMax[2]-dataBoundsMin[2])/100.f);
        
		int stepi = 0;
        for (float t = t0; t < t1 && stepi < 100; t += step, stepi++) {
            vec3 hit = cameraPos + dir * t;
            vec3 coordSTR = (hit - dataBoundsMin) / (dataBoundsMax-dataBoundsMin);
			vec3 dataSTR = texture(coords, coordSTR).rgb;

			vec4 color = vec4(dataSTR/3, 1);
            
            accum.rgb += color.rgb * color.a * (1-accum.a);
            accum.a += color.a * (1-accum.a);
            
            if (accum.a > 0.999)
                break;
        }
        
        fragColor = accum;
    }
        
    if (accum.a < 0.1)
        discard;
}
