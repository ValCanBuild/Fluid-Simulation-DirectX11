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
class VolumeRenderShader;
struct CTwBar;
struct SmokeProperties;

namespace DirectX 
{
	class CommonStates;
}

class VolumeRenderer : public PrimitiveGameObject {
public:
	~VolumeRenderer();
	VolumeRenderer(Vector3 &volumeSize);

	bool Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd);
	void Render(const ICamera &camera) override;

	void SetSourceTexture(ID3D11ShaderResourceView *sourceTexSRV);

	void DisplayRenderInfoOnBar(CTwBar * const pBar);

private:
	static void __stdcall SetSmokePropertiesCallback(void *clientData);
	void RefreshSmokeProperties();

private:	
	Vector3 mVolumeSize;
	Vector3 mPrevCameraPos;

	D3DGraphicsObject* pD3dGraphicsObj;

	std::unique_ptr<SmokeProperties>		mSmokeProperties;
	std::unique_ptr<VolumeRenderShader>		mVolumeRenderShader;
	std::shared_ptr<DirectX::CommonStates>	pCommonStates;	
};

#endif