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

	float4x4 mRotationMatrix;	// 80 bytes

	float3 vEyePos;	
	float  padding0;			// 96 bytes

	uint2  vViewportDimensions;	// 104 bytes
	
	float2 padding1;			// pad do 112 bytes
};


// Samplers
SamplerState linearSampler : register (s0);

Texture3D<float>	renderInputTexture : register (t0);
RWTexture2D<float4> renderResult : register (u0);

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, 1)]
// Output the result of a 3D texture to a render target by using ray-marching
void RenderComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	uint2 j = DTid.xy;

	float3 raydir = float3( (2*j - float2(vViewportDimensions)) / min(vViewportDimensions.x, vViewportDimensions.y) * fZoom , 0) - vEyePos;

	// rotate vuew
	float3 vEyeProper = mul(float4(vEyePos,1.0f), mRotationMatrix).xyz;
	raydir	   = mul(float4(raydir,1.0f), mRotationMatrix).xyz;

	float3 t1 = max((-1 - vEyeProper) / raydir, 0);
	float3 t2 = max(( 1 - vEyeProper) / raydir, 0);

	// Determine the closest and furthest points
	float3 front = min(t1, t2);
	float3 back  = max(t1, t2);

	float tfront = max(front.x, max(front.y, front.z));
	float tback  = min( back.x, min( back.y,  back.z));

	// Calculate texture coordinates of front and back intersection
	float3 texf  =  (vEyeProper + tfront*raydir + 1) * 0.5f;
	float3 texb  =  (vEyeProper + tback *raydir + 1) * 0.5f;

	// determine the number of steps necessary to traverse the simulation volume
	float steps = floor(length(texf - texb)*vDimensions.x + 0.5f);
	float3 texdir = (texb-texf)/steps;

	steps = (tfront >= tback) ? 0 : steps; // no intersection ?

	 // simple MIP render
	 float m = 0.0f;
	 for (float i = 0.5f; i < steps; ++i) {
		float3 samplingPoint = texf + i*texdir;
		float s = renderInputTexture.SampleLevel(linearSampler, samplingPoint, 0);      
		m += s;
		if (m > 1)
			break;
	 }

	 // hot metal color
	 float whiteSmoke = m*245.0f/250.0f;
	 float4 col = saturate(float4(whiteSmoke,whiteSmoke,whiteSmoke,m));

	 renderResult[j] = col;
}