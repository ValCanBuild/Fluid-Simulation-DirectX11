/*************************************************************
D3DFrameBuffer.h: Specific implementation of a Frame Buffer
to be used with the Direct3D API

Author: Valentin Hinov
Date: 03/09/2013
Version: 1.0
**************************************************************/

#ifndef _D3DFRAMEBUFFER_H
#define _D3DFRAMEBUFFER_H

#include <atlbase.h>

#include "IFrameBuffer.h"
#include "../utilities/D3dIncludes.h"

class D3DGraphicsObject;

class D3DFrameBuffer : public IFrameBuffer {
public:
	D3DFrameBuffer(DXGI_FORMAT bufferFormat = DXGI_FORMAT_R32G32B32A32_FLOAT);
	~D3DFrameBuffer();

	// Initialize a D3DFrameBuffer with the given width, height and buffer format. Default buffer format is R32G32B32A32_FLOAT
	bool Initialize(IGraphicsObject* graphicsObject, int width, int height);

	void BeginRender(float clearRed, float clearGreen, float clearBlue, float clearAlpha) const;
	void EndRender() const;
	
	// Return a void pointer to the underlying texture resource. This will be of type ID3D11ShaderResourceView
	const void* GetTextureResource() const;

private:
	CComPtr<ID3D11Texture2D>			mRenderTargetTexture;
	CComPtr<ID3D11RenderTargetView>		mRenderTargetView;
	CComPtr<ID3D11ShaderResourceView>	mShaderResourceView;

	D3DGraphicsObject *pD3dGraphicsObject;

	DXGI_FORMAT	mBufferFormat;
};

#endif