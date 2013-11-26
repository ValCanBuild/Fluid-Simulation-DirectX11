/***************************************************************
cFluid2D.hlsl: Contains the necessary compute shaders to
handle 2D fluid simulation by numerically solving the 
Navier-Stokes equations.

Author: Valentin Hinov
Date: 17/09/2013
***************************************************************/
#pragma warning(disable : 3203)	// disable signed/unsigned mismatch warning

#define NUM_THREADS_X 16
#define NUM_THREADS_Y 8

// Constant buffers
cbuffer InputBufferGeneral : register (b0) {
	float fTimeStep;			// Used for AdvectComputeShader, BuoyancyComputeShader
	
	float fBuoyancy;			// Used for BuoyancyComputeShader
	float fDensityWeight;		// Used for BuoyancyComputeShader
	float fAmbientTemperature;  // Used for BuoyancyComputeShader

	float fAlpha;				// Used for JacobiComputeShader
	float fInverseBeta;			// Used for JacobiComputeShader

	float fHalfInverseCellSize;	// Used for DivergenceComputeShader

	float fGradientScale;		// Used for SubtractGradientComputeShader

	float2 vDimensions;			// Used for all AdvectComputeShader, DivergenceComputeShader, JacobiComputeShader, SubtractGradientComputeShader
	
	float2 padding0;			// pad to 48 bytes
	///48 Bytes///
};

cbuffer InputBufferDissipation : register (b1) {
	float fDissipation;			// Used for AdvectComputeShader
	float3 padding1;			// pad to 16 bytes
}

cbuffer InputBufferImpulse : register (b2) {
	float2 vPoint;				// Used for ImpulseComputeShader
	float2 vFillColor;			// Used for ImpulseComputeShader
	float fRadius;				// Used for ImpulseComputeShader
	float3 padding2;			// pad to 32 bytes
}

// Samplers
SamplerState linearSampler : register (s0);

// Texture Inputs
Texture2D<float2>	velocity : register (t0);	// Used for AdvectComputeShader, DivergenceComputeShader, BuoyancyComputeShader, SubtractGradientComputeShader
Texture2D<float2>	advectionTargetA : register (t1); // Used for AdvectComputeShader, AdvectBackwardComputeShader
Texture2D<float2>	advectionTargetB : register (t2); // User for AdvectMacCormackComputeShader
Texture2D<float2>	advectionTargetC : register (t3); // User for AdvectMacCormackComputeShader
RWTexture2D<float2> advectionResult : register (u0); // Used for AdvectComputeShader, AdvectBackwardComputeShader, AdvectMacCormackComputeShader

Texture2D<float>	temperature : register (t1); // Used for BuoyancyComputeShader
Texture2D<float>	density : register (t2); // Used for BuoyancyComputeShader
RWTexture2D<float2> buoyancyResult : register (u0); // Used for BuoyancyComputeShader

Texture2D<float2>   impulseInitial : register (t0); // Used for ImpulseComputeShader
RWTexture2D<float2> impulseResult : register (u0); // Used for ImpulseComputeShader

Texture2D<float>     divergence   : register (t0);  // Used for JacobiComputeShader
RWTexture2D<float>   divergenceResult : register (u0);  // Used for DivergenceComputeShader

Texture2D<float>   pressure : register (t1);  // Used for JacobiComputeShader, SubtractGradientComputeShader
RWTexture2D<float> pressureResult : register (u0); // Used for JacobiComputeShader

RWTexture2D<float2> velocityResult : register (u0); // Used for SubtractGradientComputeShader

Texture2D<float>	obstacles : register (t4);	// Used for Advection, Jacobi, Divergence and Subtract Gradient, Obstacles

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, 1)]
// Advect the speed by sampling at pos - deltaTime*velocity
void AdvectComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	// advect by trace back
	uint2 i = DTid.xy;

	// Sample obstacles texture and make result 0 if an obstacle exists
	if (obstacles[i] > 0.0f) {
		advectionResult[i] = float2(0.0f,0.0f);
		return;
	}

	float2 prevPos = i - fTimeStep * velocity[i];
	prevPos = (prevPos+0.5f)/vDimensions;

	float2 result = advectionTargetA.SampleLevel(linearSampler, prevPos, 0);

	advectionResult[i] = result;//*fDissipation;
}

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, 1)]
// Advect the speed by sampling at pos + deltaTime*velocity
void AdvectBackwardComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	// advect by trace back
	uint2 i = DTid.xy;

	// Sample obstacles texture and make result 0 if an obstacle exists
	if (obstacles[i] > 0.0f) {
		advectionResult[i] = float2(0.0f,0.0f);
		return;
	}

	float2 prevPos = i + fTimeStep * velocity[i];
	prevPos = (prevPos+0.5f)/vDimensions;

	float2 result = advectionTargetA.SampleLevel(linearSampler, prevPos, 0);

	advectionResult[i] = result;
}

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, 1)]
// Advect the speed by using the two intermediate semi-Lagrangian steps to achieve higher-order accuracy
void AdvectMacCormackComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	uint2 i = DTid.xy;

	// Sample obstacles texture and make result 0 if an obstacle exists
	if (obstacles[i] > 0.0f) {
		advectionResult[i] = float2(0.0f,0.0f);
		return;
	}

	// advect by trace back
	float2 prevPos = i - fTimeStep * velocity[i];
	uint2 j = (uint2) prevPos;

	prevPos = (prevPos+0.5f)/vDimensions;

	// Get the values of nodes that contribute to the interpolated value.  
	float2 r0 = advectionTargetA[j + uint2(0,0)];
	float2 r1 = advectionTargetA[j + uint2(1,0)];
	float2 r2 = advectionTargetA[j + uint2(0,1)];
	float2 r3 = advectionTargetA[j + uint2(1,1)];

	// Determine a valid range for the result.
	float2 lmin = min(r0,min(r1,min(r2, r3)));
	float2 lmax = max(r0,max(r1,max(r2, r3)));

	// Perform final advection, combining values from intermediate advection steps.
	// based on http://http.developer.nvidia.com/GPUGems3/elementLinks/0640equ01.jpg
	float2 phi_n_1_hat = advectionTargetA.SampleLevel(linearSampler,prevPos, 0);
	float2 phi_n_hat = advectionTargetB.SampleLevel(linearSampler,prevPos, 0);
	float2 phi_n = advectionTargetC.SampleLevel(linearSampler,prevPos, 0);
	
	float2 s = phi_n_1_hat + 0.5f*(phi_n - phi_n_hat);

	// clamp results to desired range
	s = clamp(s,lmin,lmax);

	advectionResult[i] = s*fDissipation;
}

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, 1)]
// Create upward force by using the temperature difference
void BuoyancyComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	uint2 i = DTid.xy;

	float temperatureVal = temperature[i];
	float densityVal = density[i];

	float2 result = velocity[i];

	if (temperatureVal > fAmbientTemperature) {
		result += (fTimeStep * (temperatureVal - fAmbientTemperature) * fBuoyancy - (densityVal * fDensityWeight) ) * float2(0.0f,-1.0f);
	}
	buoyancyResult[i] = result;
}

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, 1)]
// Adds impulse depending on point of interaction, also used for rendering obstacles
void ImpulseComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	uint2 i = DTid.xy;

	float d = distance(vPoint,i);

	if (d < fRadius) {
		//float a = (fRadius - d) * 0.5f;
		//a = min(a,1.0f);
		impulseResult[i] = vFillColor;
	}
	else
		impulseResult[i] = impulseInitial[i];
}

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, 1)]
// calculate the velocity divergence
void DivergenceComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	uint2 i = DTid.xy;

	uint2 coordT = i + uint2(0, 1);
	uint2 coordB = i - uint2(0, 1);
	uint2 coordR = i + uint2(1, 0);
	uint2 coordL = i - uint2(1, 0);

	// Find neighbouring velocities
	float2 vT = velocity[coordT];
	float2 vB = velocity[coordB];
	float2 vR = velocity[coordR];
	float2 vL = velocity[coordL];
	
	bool oT = obstacles[coordT] > 0.0f;
	bool oB = obstacles[coordB] > 0.0f;
	bool oR = obstacles[coordR] > 0.0f;
	bool oL = obstacles[coordL] > 0.0f;

	// Enforce boundaries
	if (oT || coordT.y > vDimensions.y - 1) {
		vT.y = 0.0f;
	}
	if (oB || coordB.y < 1) {
		vB.y = 0.0f;
	}
	if (oR || coordR.x > vDimensions.x - 1) {
		vR.x = 0.0f;
	}
	if (oL || coordL.x < 1) {
		vL.x = 0.0f;
	}

	float result = fHalfInverseCellSize * (vR.x - vL.x + vT.y - vB.y);

	divergenceResult[i] = result;
}

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, 1)]
// jacobi shader to compute the gradient pressure field
void JacobiComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	uint2 i = DTid.xy;

	uint2 coordT = uint2(i.x, min(i.y+1,vDimensions.y-1));
	uint2 coordB = uint2(i.x, max(i.y-1,1));
	uint2 coordR = uint2(min(i.x+1,vDimensions.x-1), i.y);
	uint2 coordL = uint2(max(i.x-1,1), i.y);

	float xC = pressure[i];

	float xT = pressure[coordT];
	float xB = pressure[coordB];
	float xR = pressure[coordR];
	float xL = pressure[coordL];

	if (obstacles[coordT] > 0.0f) {
		xT = xC;
	}
	if (obstacles[coordB] > 0.0f) {
		xB = xC;
	}
	if (obstacles[coordR] > 0.0f) {
		xR = xC;
	}
	if (obstacles[coordL] > 0.0f) {
		xL = xC;
	}

	// Sample divergence
	float bC = divergence[i];

	float final = (xL + xR + xB + xT + (fAlpha * bC) ) * fInverseBeta;

	pressureResult[i] = final;
}

[numthreads(NUM_THREADS_X, NUM_THREADS_Y, 1)]
// enforce incompressibility condition by making the velocity divergence 0 by subtracting the pressure gradient
void SubtractGradientComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	uint2 i = DTid.xy;

	uint2 coordT = i + uint2(0, 1);
	uint2 coordB = i - uint2(0, 1);
	uint2 coordR = i + uint2(1, 0);
	uint2 coordL = i - uint2(1, 0);

	// Find neighbouring pressure
	float pT = pressure[coordT];
	float pB = pressure[coordB];
	float pR = pressure[coordR];
	float pL = pressure[coordL];
	float pC = pressure[i];

	bool oT = obstacles[coordT] > 0.0f;
	bool oB = obstacles[coordB] > 0.0f;
	bool oR = obstacles[coordR] > 0.0f;
	bool oL = obstacles[coordL] > 0.0f;

	//float2 obstV = float2(0,0);
	//float2 vMask = float2(1,1);
	// If an adjacent cell is solid or boundary, ignore its pressure and use its velocity. 
	if (oT || coordT.y > vDimensions.y - 1) {
		pT = pC;
	}

	if (oB || coordB.y < 1) {
		pB = pC;
	}

	if (oR || coordR.x > vDimensions.x - 1) {
		pR = pC;
	}

	if (oL || coordL.x < 1) {
		pL = pC;
	}

	// Compute the gradient of pressure at the current cell by taking central differences of neighboring pressure values. 
	float2 grad = float2(pR - pL, pT - pB) * fGradientScale;
	// Project the velocity onto its divergence-free component by subtracting the gradient of pressure.  
	float2 oldV = velocity[i];
	float2 newV = oldV - grad;
	// Explicitly enforce the free-slip boundary condition by  
	// replacing the appropriate components of the new velocity with  
	// obstacle velocities. 

	//newV = (vMask * newV) + obstV;

	velocityResult[i] = newV;
}