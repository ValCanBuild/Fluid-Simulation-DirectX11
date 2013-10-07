/*************************************************************
D3DFrameBuffer.h: Specific implementation of a Frame Buffer
to be used with the Direct3D API

Author: Valentin Hinov
Date: 03/09/2013
Version: 1.0
**************************************************************/

#ifndef _D3DFRAMEBUFFER_H
#define _D3DFRAMEBUFFER_H

#include <memory>

#include "IFrameBuffer.h"
#include "../utilities/D3dIncludes.h"

class D3DGraphicsObject;

class D3DFrameBuffer : public IFrameBuffer {
public:
	D3DFrameBuffer();
	~D3DFrameBuffer();

	bool Initialize(IGraphicsObject* graphicsObject, int width, int height);

	void BeginRender(float clearRed, float clearGreen, float clearBlue, float clearAlpha) const;
	void EndRender() const;
	
	// Return a void pointer to the underlying texture resource. This will be of type ID3D11ShaderResourceView
	const void* GetTextureResource() const;

private:
	std::unique_ptr<ID3D11Texture2D,COMDeleter>				mRenderTargetTexture;
	std::unique_ptr<ID3D11RenderTargetView,COMDeleter>		mRenderTargetView;
	std::unique_ptr<ID3D11ShaderResourceView,COMDeleter>	mShaderResourceView;

	D3DGraphicsObject *pD3dGraphicsObject;
};

#endif