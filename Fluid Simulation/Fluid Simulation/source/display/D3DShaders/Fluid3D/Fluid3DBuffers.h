/********************************************************************
Fluid3DBuffers: Contains all the necessary buffer structs for the
cFluid3D.hlsl shaders

Author:	Valentin Hinov
Date: 5/3/2014
*********************************************************************/

#ifndef _FLUID3DBUFFERS_H
#define _FLUID3DBUFFERS_H

#include "../../../utilities/D3dIncludes.h"

namespace Fluid3D {

	struct InputBufferGeneral {
		float fTimeStep;
		float fDensityBuoyancy;	
		float fDensityWeight;		
		float fAmbientTemperature; 
	};

	struct InputBufferDissipation {
		float fDissipation;
		Vector3 padding1;
	};

	struct InputBufferImpulse {
		Vector3 vPoint;				
		float fRadius;
		Vector4 vFillColor;			
	};

	template<typename T>
	bool BuildBuffer(_In_ ID3D11Device *device, _Out_ ID3D11Buffer **pOutBuffer) {
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = sizeof(T);
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		// General buffer
		HRESULT hresult = device->CreateBuffer(&bufferDesc, NULL, pOutBuffer);

		return !FAILED(hresult);
	}
}

#endif