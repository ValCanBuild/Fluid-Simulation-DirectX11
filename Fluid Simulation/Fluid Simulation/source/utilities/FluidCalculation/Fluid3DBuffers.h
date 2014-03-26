/********************************************************************
Fluid3DBuffers: Contains all the necessary buffer structs for the
cFluid3D.hlsl shaders

Author:	Valentin Hinov
Date: 5/3/2014
*********************************************************************/

#ifndef _FLUID3DBUFFERS_H
#define _FLUID3DBUFFERS_H

#include "../D3dIncludes.h"

namespace Fluid3D {

	struct InputBufferGeneral {
		float fTimeStep;
		float fDensityBuoyancy;	
		float fDensityWeight;		
		float fVorticityStrength; 
	};

	struct InputBufferAdvection {
		float fDissipation;
		float fTimeStepModifier;
		float fDecay;
		float padding1;
	};

	struct InputBufferImpulse {
		Vector3 vPoint;				
		float fRadius;
		float fAmount;
		float fExtinguishment;
		Vector2 padding2;	
	};
}

#endif