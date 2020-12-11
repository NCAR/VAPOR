uniform float isoValue[4];
uniform bool  isoEnabled[4];
uniform vec4 constantColor;

vec4 GetIsoSurfaceColor(vec3 sampleSTR)
{
    float value = texture(data, sampleSTR).r;
    float valueNorm = (value - LUTMin) / (LUTMax - LUTMin);
    float opacity = texture(LUT, valueNorm).a;
    
#ifdef USE_SECOND_DATA
	if (useColormapData) {
		if (hasMissingData2)
			if (DoesSampleHaveMissingData2(sampleSTR))
					return vec4(0);

		float value2 = texture(data2, sampleSTR).r;
		float valueNorm2 = (value2 - LUTMin2) / (LUTMax2 - LUTMin2);
        return vec4(texture(LUT2, valueNorm2).rgb, opacity);
	}
#endif
    
    return vec4(constantColor.rgb, opacity);
}
