/*************************************************************
D3DRenderer.h: Implementation of a Direct3D renderer.
Provides method for initializing and rendering an ID3D11Buffer

Author: Valentin Hinov
Date: 03/09/2013
Version: 1.0
**************************************************************/

#ifndef _D3DRENDERER_H
#define _D3DRENDERER_H

#include <memory>

#include "../utilities/D3dIncludes.h"

class D3DGraphicsObject;

class D3DRenderer {
public:
	// by default, d3d renderer renders a triangle list
	D3DRenderer(D3D_PRIMITIVE_TOPOLOGY topologyChoice = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	~D3DRenderer();

	void RenderBuffers(ID3D11DeviceContext* context) const;

	bool InitializeBuffers(ID3D11Device *device, DWORD* indices, void *vertices, DWORD stride, DWORD vertexCount, DWORD indexCount);

	DWORD GetIndexCount() const;
	DWORD GetVertexCount() const;

private:
	std::unique_ptr<ID3D11Buffer,COMDeleter>	mVB;
	std::unique_ptr<ID3D11Buffer,COMDeleter>	mIB;
	unsigned int	mStride;
	unsigned int	mOffset;
	DWORD			mVertexCount;
	DWORD			mIndexCount;

	D3D_PRIMITIVE_TOPOLOGY	mTopologyChoice;
};

#endif