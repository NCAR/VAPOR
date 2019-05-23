uniform float isoValue[4];
uniform bool  isoEnabled[4];
uniform vec4 constantColor;

uniform bool useColormapData;
uniform bool hasMissingData2;
uniform sampler3D data2;
uniform sampler3D missingMask2;

bool DoesSampleHaveMissingData2(vec3 dataSTR)
{
    return texture(missingMask2, dataSTR).r > 0;
}

vec4 GetIsoSurfaceColor(vec3 sampleSTR)
{
	if (useColormapData) {
		if (hasMissingData2)
			if (DoesSampleHaveMissingData2(sampleSTR))
					return vec4(0);

		float data = texture(data2, sampleSTR).r;
		float dataNorm = (data - LUTMin) / (LUTMax - LUTMin);
		return texture(LUT, dataNorm);
	} else {
		return constantColor;
	}
}
