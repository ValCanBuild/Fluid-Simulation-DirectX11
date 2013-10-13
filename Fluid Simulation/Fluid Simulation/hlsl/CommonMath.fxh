
// Returns a bilinearly interpolated color using the given texture, sampler and texel position
// rdFactors is 1/textureWidth and 1/textureHeight
float4 Bilerp(Texture2D shaderTexture, SamplerState samplerState, float2 pos, float2 rdFactors) {
	float2 textureSize = float2(1.0f/rdFactors.x,1.0f/rdFactors.y);
	float realPosX = pos.x * textureSize.x;
	float realPosY = pos.y * textureSize.y;

	realPosX = max(realPosX,0.5f);
	realPosX = min(realPosX,textureSize.x+0.5f);

	realPosY = max(realPosY,0.5f);
	realPosY = min(realPosY,textureSize.y+0.5f);

	int iX0 = int(realPosX);
	int iY0 = int(realPosY);
	int iX1 = int(realPosX) + 1;
	int iY1 = int(realPosY) + 1;

	float s1 = float(realPosX - iX0);
	float s0 = 1.0f - s1;

	float t1 = float(realPosY - iY0);
	float t0 = 1.0f - t1;

	float texCoordX0 = float(iX0/textureSize.x);
	float texCoordX1 = float(iX1/textureSize.x);

	float texCoordY0 = float(iY0/textureSize.y);
	float texCoordY1 = float(iY1/textureSize.y);

	float4 col00 = shaderTexture.Sample(samplerState, float2(texCoordX0,texCoordY0));
	float4 col01 = shaderTexture.Sample(samplerState, float2(texCoordX0,texCoordY1));
	float4 col10 = shaderTexture.Sample(samplerState, float2(texCoordX1,texCoordY0));
	float4 col11 = shaderTexture.Sample(samplerState, float2(texCoordX1,texCoordY1));

	float4 finalColor = s0*(t0*col00 + t1*col01) +
						s1*(t0*col10 + t1*col11);

	return finalColor;
}