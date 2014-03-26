/********************************************************************
VolumeRenderer.h: Implementation of a Volume renderer object which
is used to render a 3D texture of values

Author:	Valentin Hinov
Date: 19/2/2014
*********************************************************************/

#ifndef _VOLUMERENDERER_H
#define _VOLUMERENDERER_H

#include "../objects/PrimitiveGameObject.h"

#include <string>
#include <memory>
#include "../utilities/AtlInclude.h"
#include "../display/D3DGraphicsObject.h"

class ICamera;
class SmokeRenderShader;
struct CTwBar;
struct SmokeProperties;

enum  FluidType_t;

namespace DirectX 
{
	class CommonStates;
}

class VolumeRenderer : public PrimitiveGameObject {
public:
	~VolumeRenderer();
	VolumeRenderer(const Vector3 &volumeSize);

	bool Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd, const FluidType_t &fluidType);
	void Render(const ICamera &camera) override;

	void SetSourceTexture(ID3D11ShaderResourceView *sourceTexSRV);
	void SetReactionTexture(ID3D11ShaderResourceView *reactionTexSRV);
	void SetFireGradientTexture(ID3D11ShaderResourceView *gradientTexSRV);

	void DisplayRenderInfoOnBar(CTwBar * const pBar);

private:
	static void __stdcall SetSmokePropertiesCallback(void *clientData);
	void RefreshSmokeProperties();

private:	
	Vector3 mVolumeSize;
	Vector3 mPrevCameraPos;
	FluidType_t mFluidType;

	D3DGraphicsObject* pD3dGraphicsObj;

	std::unique_ptr<SmokeProperties>		mSmokeProperties;
	std::unique_ptr<SmokeRenderShader>		mVolumeRenderShader;
	std::shared_ptr<DirectX::CommonStates>	pCommonStates;	
};

#endif