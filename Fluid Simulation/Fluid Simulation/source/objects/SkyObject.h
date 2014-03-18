/********************************************************************
SkyObject: Represents a primitive object to be used a skysphere

Author:	Valentin Hinov
Date: 18/3/2014
*********************************************************************/

#ifndef _SKYOBJECT_H
#define _SKYOBJECT_H

#include "PrimitiveGameObject.h"

#include <string>
#include <memory>
#include "../utilities/AtlInclude.h"
#include "../display/D3DGraphicsObject.h"

class ICamera;
class D3DTexture;
class SkyShader;

namespace DirectX 
{
	class CommonStates;
}

class SkyObject : public PrimitiveGameObject {
public:
	SkyObject();
	~SkyObject();

	bool Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, _In_ WCHAR *texturePath, HWND hwnd);
	void Render(const ICamera &camera) override;

private:	
	D3DGraphicsObject* pD3dGraphicsObj;
	std::unique_ptr<D3DTexture>	mSkyTexture;
	std::unique_ptr<SkyShader>	mSkyShader;

	std::shared_ptr<DirectX::CommonStates>	pCommonStates;	
};

#endif