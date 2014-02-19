/********************************************************************
VolumeRenderer.h: Implementation of a Volume renderer object which
is used to render a 3D texture of values

Author:	Valentin Hinov
Date: 19/2/2014
*********************************************************************/

#ifndef _VOLUMERENDERER_H
#define _VOLUMERENDERER_H

#include <memory>
#include <atlbase.h>
#if defined (_DEBUG)
#pragma comment(lib,"atlsd.lib")
#endif

#include "D3DGraphicsObject.h"

class VolumeRenderShader;
struct ShaderParams;
class D2DTexQuad;
class Camera;

class VolumeRenderer {
public:
	VolumeRenderer(Vector3 &volumeSize, Vector3 &position);
	~VolumeRenderer();

	bool Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd);
	bool Render(ID3D11ShaderResourceView * sourceTexSRV, Camera *camera, float zoom, const Matrix* viewMatrix, const Matrix* projMatrix);

	void SetPosition(Vector3 &position);

private:
	bool InitRenderResult(HWND hwnd);
	bool InitializeRenderQuad(HWND hwnd);

private:
	Vector3 mPosition;
	Vector3 mVolumeSize;

	D3DGraphicsObject* pD3dGraphicsObj;
	std::unique_ptr<D2DTexQuad>	mTexQuad;

	unique_ptr<ShaderParams> mRenderResult;
	std::unique_ptr<VolumeRenderShader>	mVolumeRenderShader;
};

#endif