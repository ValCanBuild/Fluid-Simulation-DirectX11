/*************************************************************
D3DRenderer.cpp: Implementation of a Direct3D renderer.
Provides method for initializing and rendering an ID3D11Buffer

Author: Valentin Hinov
Date: 03/09/2013
Version: 1.0
**************************************************************/

#include "D3DRenderer.h"
#include "D3DGraphicsObject.h"

D3DRenderer::D3DRenderer(D3D_PRIMITIVE_TOPOLOGY topologyChoice) :
mTopologyChoice(topologyChoice) {

}

D3DRenderer::~D3DRenderer() {

}

void D3DRenderer::RenderBuffers(ID3D11DeviceContext* context) const {
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	context->IASetVertexBuffers(0, 1, &(mVB._Myptr), &mStride, &mOffset);
	
	// Set the index buffer to active in the input assembler so it can be rendered.
	context->IASetIndexBuffer(mIB.get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	context->IASetPrimitiveTopology(mTopologyChoice);
}

bool D3DRenderer::InitializeBuffers(ID3D11Device *device, DWORD* indices, void *vertices, DWORD stride, DWORD vertexCount, DWORD indexCount) {
	mVertexCount = vertexCount;
	mIndexCount = indexCount;

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.ByteWidth = stride * mVertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	HRESULT result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &(mVB._Myptr));
	if (FAILED(result)) {
		return false;
	}

	// Set up the description of the index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(DWORD) * mIndexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &(mIB._Myptr));

	if(FAILED(result)){
		return false;
	}

	mStride = stride; //set the stride of the buffers to be the size of the vertex struct
	mOffset = 0;
	return true;
}

DWORD D3DRenderer::GetIndexCount() const {
	return mIndexCount;
}

DWORD D3DRenderer::GetVertexCount() const {
	return mVertexCount;
}