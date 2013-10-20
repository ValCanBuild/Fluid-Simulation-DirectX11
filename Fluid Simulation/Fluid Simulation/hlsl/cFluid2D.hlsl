/***************************************************************
cFluid2D.hlsl: Contains the necessary compute shaders to
handle 2D fluid simulation by numerically solving the 
Navier-Stokes equations.

Author: Valentin Hinov
Date: 17/09/2013
***************************************************************/
#pragma warning(disable : 3203)	// disable signed/unsigned mismatch warning

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
Texture2D<float2>	advectionTarget : register (t1); // Used for AdvectComputeShader
RWTexture2D<float2> advectionResult : register (u0); // Used for AdvectComputeShader

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

[numthreads(32, 32, 1)]
// Advect the speed by sampling at pos - deltaTime*velocity
void AdvectComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	// advect by trace back
	uint2 i = DTid.xy;

	if (i.y > vDimensions.y - 1) {
		advectionResult[i] = float2(0,0);
		return;
	}
	if (i.y < 1) {
		advectionResult[i] = float2(0,0);
		return;
	}
	if (i.x > vDimensions.x - 1) {
		advectionResult[i] = float2(0,0);
		return;
	}
	if (i.x < 1) {
		advectionResult[i] = float2(0,0);
		return;
	}

	float2 prevPos = i - velocity[i];
	prevPos = (prevPos+0.5f)/vDimensions;

	float2 result = advectionTarget.SampleLevel(linearSampler, prevPos, 0);

	advectionResult[i] = result;
}

[numthreads(32, 32, 1)]
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

[numthreads(32, 32, 1)]
// Adds impulse depending on point of interaction
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

[numthreads(32, 32, 1)]
// calculate the velocity divergence
void DivergenceComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	uint2 i = DTid.xy;

	uint2 coordT = i + uint2(0, 1);
	uint2 coordB = i - uint2(0, 1);
	uint2 coordR = i + uint2(1, 0);
	uint2 coordL = i - uint2(1, 0);
	/*uint2 coordT = uint2(i.x, min(i.y+1,vDimensions.y-1));
	uint2 coordB = uint2(i.x, max(i.y-1,0));
	uint2 coordR = uint2(min(i.x+1,vDimensions.x-1), i.y);
	uint2 coordL = uint2(max(i.x-1,0), i.y);*/

	// Find neighbouring velocities
	float2 vT = velocity[coordT];
	float2 vB = velocity[coordB];
	float2 vR = velocity[coordR];
	float2 vL = velocity[coordL];
	//float2 vC =	velocity[i];

	// Enforce boundaries
	if (coordT.y > vDimensions.y - 1) {
		//vT = vC;
		vT = 0;
	}
	if (coordB.y < 1) {
		//vB = vC;
		vB = 0;
	}
	if (coordR.x > vDimensions.x - 1) {
		//vR = vC;
		vR = 0;
	}
	if (coordL.x < 1) {
		//vL = vC;
		vL = 0;
	}

	float result = fHalfInverseCellSize * (vR.x - vL.x + vT.y - vB.y);

	divergenceResult[i] = result;
}

[numthreads(32, 32, 1)]
// jacobi shader to compute the gradient pressure field
void JacobiComputeShader( uint3 DTid : SV_DispatchThreadID ) {
	uint2 i = DTid.xy;

	/*uint2 coordT = i.xy + uint2(0, 1);
	uint2 coordB = i.xy - uint2(0, 1);
	uint2 coordR = i.xy + uint2(1, 0);
	uint2 coordL = i.xy - uint2(1, 0);*/

	uint2 coordT = uint2(i.x, min(i.y+1,vDimensions.y-1));
	uint2 coordB = uint2(i.x, max(i.y-1,1));
	uint2 coordR = uint2(min(i.x+1,vDimensions.x-1), i.y);
	uint2 coordL = uint2(max(i.x-1,1), i.y);

	float xT = pressure[coordT];
	float xB = pressure[coordB];
	float xR = pressure[coordR];
	float xL = pressure[coordL];
	
	/*float xC = pressure[i];

	// Enforce boundary conditions
	if (coordT.y > vDimensions.y - 1)
		xT = xC;
	if (coordB.y < 1)
		xB = xC;
	if (coordR.x > vDimensions.x - 1)
		xR = xC;
	if (coordL.x < 1)
		xL = xC;*/

	// Sample divergence
	float bC = divergence[i];

	float final = (xL + xR + xB + xT + (fAlpha * bC) ) * fInverseBeta;

	pressureResult[i] = final;
}

[numthreads(32, 32, 1)]
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

	float2 obstV = float2(0,0);
	float2 vMask = float2(1,1);
	// If an adjacent cell is solid or boundary, ignore its pressure and use its velocity. 
	if (coordT.y > vDimensions.y - 1) {
		pT = pC;
		//obstV.y = velocity[coordT].y;
		//vMask.y = 0;
	}

	if (coordB.y < 1) {
		pB = pC;
		//obstV.y = velocity[coordB].y;
		//vMask.y = 0;
	}

	if (coordR.x > vDimensions.x - 1) {
		pR = pC;
		//obstV.x = 0;
		//vMask.x = 0;
	}

	if (coordL.x < 1) {
		pL = pC;
		//obstV.x = 0;
		//vMask.x = 0;
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