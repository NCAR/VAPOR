uniform float isoValue[4];
uniform bool  isoEnabled[4];
uniform vec4 constantColor;

vec4 GetIsoSurfaceColor(vec3 sampleSTR)
{
#ifdef USE_SECOND_DATA
	if (useColormapData) {
		if (hasMissingData2)
			if (DoesSampleHaveMissingData2(sampleSTR))
					return vec4(0);

		float value2 = texture(data2, sampleSTR).r;
		float valueNorm2 = (value2 - LUTMin2) / (LUTMax2 - LUTMin2);
		return texture(LUT2, valueNorm2);
	}
#endif
    return constantColor;
}
