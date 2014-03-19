/********************************************************************
FluidResources.h: Contains fluid calculation resources. 
Both per-object and common resources(ones that can be shared).

Author:	Valentin Hinov
Date: 19/3/2014
*********************************************************************/

#ifndef _FLUIDRESOURCES_H
#define _FLUIDRESOURCES_H

#include <memory>
#include <array>
#include "../../display/D3DShaders/ShaderParams.h"

struct CommonFluidResources {
	ShaderParams divergenceSP;
	std::array<ShaderParams, 2>	pressureSP;
	std::array<ShaderParams, 2>	tempSP;
	CComPtr<ID3D11RenderTargetView>	pressureRT;	

	static CommonFluidResources CreateResources(ID3D11Device * device, Vector3 textureSize, HWND hwnd);
};

struct FluidResourcesPerObject {
	std::array<ShaderParams, 2>	velocitySP;
	std::array<ShaderParams, 2>	densitySP;
	std::array<ShaderParams, 2>	temperatureSP;
	ShaderParams obstacleSP;
	ShaderParams vorticitySP;

	static FluidResourcesPerObject CreateResources(ID3D11Device * device, Vector3 textureSize, HWND hwnd);
};

#endif