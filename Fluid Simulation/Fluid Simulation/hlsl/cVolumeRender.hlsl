/***************************************************************
cVolumeRender.hlsl: Renders a 3D texture using ray-marching

Author: Valentin Hinov
Date: 19/02/2014
***************************************************************/
#pragma warning(disable : 3203)	// disable signed/unsigned mismatch warning

#define NUM_THREADS_X 16
#define NUM_THREADS_Y 8

// Constant buffers
cbuffer InputBuffer : register (b0) {
	float3 vDimensions;	
	float  fZoom;				// 16 bytes

	float3 vWorldPos;			
	float  padding0;			// 32 bytes

	float3 vEyePos;	
	float  padding1;			// 48 bytes

	uint2  vViewportDimensions;	// 56 bytes
	
	float2 padding2;			// pad do 64 bytes
};


// Samplers
SamplerState linearSampler : register (s0);

Texture3D<float>	renderInputTexture : register (t0);
RWTexture2D<float4> renderResult : register (u0);


//static const float stepSize = 1.0f;
static const float densityFactor = 10;
static const float absorption = 10.0f;
static const float3 lightPosition = float3(1.0f, 1.0f, 2.0f);
static const float3 lightIntensity = float3(10.0f, 10.0f, 10.0f);
static const float maxDist = sqrt(2.0);
static const int numSamples = 128;
static const float stepSize = maxDist/float(numSamples);
static const int numLightSamples = 32;
static const float lscale = maxDist / float(numLightSamples);

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, 1)]
// Output the result of a 3D texture to a render target by using ray-marching
void RenderComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	uint2 j = DTid.xy;

	float windowSize = min(vViewportDimensions.x, vViewportDimensions.y);
	float zoom = distance(vEyePos, vWorldPos);
	float3 raydir = float3( (2*j - float2(vViewportDimensions)) / windowSize * zoom , 0) - vEyePos;
	// rotate view
	//float3 vEyeProper = mul(float4(vEyePos,1.0f), mRotationMatrix).xyz;

	//raydir = mul(float4(raydir,1.0f), mRotationMatrix).xyz;
	
	float3 t1 = max((-1 - vEyePos) / raydir, 0);
	float3 t2 = max(( 1 - vEyePos) / raydir, 0);

	// Determine the closest and furthest points
	float3 front = min(t1, t2);
	float3 back  = max(t1, t2);

	// AABB intersection
	float tfront = max(front.x, max(front.y, front.z));
	float tback  = min( back.x, min( back.y,  back.z));

	//if eye is in cube then start ray at eye
	if (tfront < 0.0f) {
		tfront = 0.0f;
	}

	// Calculate texture coordinates of front and back intersection
	float3 rayStart  =  (vEyePos + tfront*raydir + 1.0f) * 0.5f;
	float3 rayEnd  =  (vEyePos +  tback*raydir + 1.0f) * 0.5f;

	float3 pos = rayStart;
	float3 step = normalize(rayEnd - rayStart) * stepSize;
	float travel = distance(rayEnd, rayStart);

	float T = 1.0f;

	//float3 Lo = float3(0.0f,0.0f,0.0f);

	// simple MIP render
	for (int i = 0; i < numSamples && travel > 0.0; ++i, pos += step, travel -= stepSize) {
		//float3 samplingPoint = rayStart + i*rayDir;
		float density = renderInputTexture.SampleLevel(linearSampler, pos, 0) * densityFactor;      
		if (density <= 0.0f) {
			continue;
		}

		T *= 1.0f - saturate(density*stepSize*absorption);
		if (T <= 0.01f) {
			break;
		}

		/*float3 lightDir = normalize(lightPosition - pos) * lscale;
		float Tl = 1.0f;
		float3 lpos = pos + lightDir;

		for (int s = 0; s < numLightSamples; ++s) {
			float ld = renderInputTexture.SampleLevel(linearSampler, lpos, 0);
			Tl *= 1.0f - absorption*stepSize*ld;
			if (Tl <= 0.01f) {
				lpos += lightDir;
			}
		}

		float3 Li = lightIntensity*Tl;
		Lo += Li*T*density*stepSize;*/
	}
	float4 col = float4(0.95f, 0.95f, 0.95f, 1.0f) * (1.0f - T);
	//float4 col = float4(Lo,1.0f-T);
	//float whiteSmoke = (1.0f-T)*245.0f/250.0f;
	//float4 col = saturate(float4(whiteSmoke,whiteSmoke,whiteSmoke,T));

	renderResult[j] = col;
}