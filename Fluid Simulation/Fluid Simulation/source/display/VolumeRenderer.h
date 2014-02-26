/********************************************************************
VolumeRenderer.h: Implementation of a Volume renderer object which
is used to render a 3D texture of values

Author:	Valentin Hinov
Date: 19/2/2014
*********************************************************************/

#ifndef _VOLUMERENDERER_H
#define _VOLUMERENDERER_H

#include <memory>
#include <GeometricPrimitive.h>
#include "../utilities/AtlInclude.h"
#include "../objects/Transform.h"

#include "D3DGraphicsObject.h"

//class VolumeRenderShader;
class VolumeRenderShader;
struct ShaderParams;
class Camera;

class VolumeRenderer {
public:
	VolumeRenderer(Vector3 &volumeSize, Vector3 &position);
	~VolumeRenderer();

	bool Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd);
	void Render(ID3D11ShaderResourceView *sourceTexSRV, Camera *camera, const Matrix* viewMatrix, const Matrix* projMatrix);

	void SetPosition(Vector3 &position);

private:
	Transform mTransform;
	Vector3 mVolumeSize;

	D3DGraphicsObject* pD3dGraphicsObj;
	std::unique_ptr<DirectX::GeometricPrimitive> mVolumeBox;

	std::unique_ptr<VolumeRenderShader>	mVolumeRenderShader;
};

#endif