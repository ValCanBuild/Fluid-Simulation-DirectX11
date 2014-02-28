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
#include "D3DGraphicsObject.h"

struct ShaderParams;
class Camera;

namespace DirectX 
{
	class CommonStates;
}

class VolumeRenderer : public PrimitiveGameObject {
public:
	~VolumeRenderer();
	VolumeRenderer(ID3D11DeviceContext *pContext, Vector3 &volumeSize);

	bool Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd);
	void Render(const Matrix &viewMatrix, const Matrix &projectionMatrix);

	void SetSourceTexture(ID3D11ShaderResourceView *sourceTexSRV);
	void SetCamera(Camera *camera);

private:	
	Vector3 mVolumeSize;

	D3DGraphicsObject* pD3dGraphicsObj;
	ID3D11ShaderResourceView *pSourceTexSRV;
	Camera *pCamera;

	std::shared_ptr<DirectX::CommonStates>	pCommonStates;	
};

#endif