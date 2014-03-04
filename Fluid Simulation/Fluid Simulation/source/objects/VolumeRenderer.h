/********************************************************************
VolumeRenderer.h: Implementation of a Volume renderer object which
is used to render a 3D texture of values

Author:	Valentin Hinov
Date: 19/2/2014
*********************************************************************/

#ifndef _VOLUMERENDERER_H
#define _VOLUMERENDERER_H

#include "../objects/PrimitiveGameObject.h"

#include <memory>
#include "../utilities/AtlInclude.h"
#include "../display/D3DGraphicsObject.h"

struct ShaderParams;
class Camera;
class VolumeRenderShader;

namespace DirectX 
{
	class CommonStates;
}

class VolumeRenderer : public PrimitiveGameObject {
public:
	~VolumeRenderer();
	VolumeRenderer(Vector3 &volumeSize);

	bool Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd);
	virtual void Render(const Matrix &viewMatrix, const Matrix &projectionMatrix) override;

	void SetSourceTexture(ID3D11ShaderResourceView *sourceTexSRV);
	void SetCamera(Camera *camera);

private:	
	Vector3 mVolumeSize;
	Vector3 mPrevCameraPos;

	D3DGraphicsObject* pD3dGraphicsObj;
	ID3D11ShaderResourceView *pSourceTexSRV;
	Camera *pCamera;

	std::unique_ptr<VolumeRenderShader>		mVolumeRenderShader;
	std::shared_ptr<DirectX::CommonStates>	pCommonStates;	
};

#endif