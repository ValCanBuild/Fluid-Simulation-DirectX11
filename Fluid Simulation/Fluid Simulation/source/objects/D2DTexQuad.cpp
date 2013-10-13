/***************************************************************
D2DTexQuad.cpp: A Direct3D renderable which represents a 2D
texturable ortho quad with
initial size of 1, which when rendered covers the whole screen.

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#include "D2DTexQuad.h"
#include "../display/D3DGraphicsObject.h"
#include "../display/D3DShaders/OrthoTextureShader.h"

struct VertexTypeTex {
	Vector3 position;
	Vector2 texC;
};

D2DTexQuad::D2DTexQuad() {
	pD3dGraphicsObj = nullptr;
	pTextureResource = nullptr;
}

D2DTexQuad::~D2DTexQuad() {
	pD3dGraphicsObj = nullptr;
	pTextureResource = nullptr;
}

bool D2DTexQuad::Initialize(IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	mShader = unique_ptr<OrthoTextureShader>(new OrthoTextureShader());
	bool result = mShader->Initialize(pD3dGraphicsObj->GetDevice(), hwnd);
	if (!result)
		return false;

	VertexTypeTex* vertices = new VertexTypeTex[6];

	float left = -1.0f;
	float right = 1.0f;
	float top = 1.0f;
	float bottom = -1.0f;

	// Load the vertex array with data.
	// First triangle.
	vertices[0].position = Vector3(left, top, 0.0f);  // Top left.
	vertices[0].texC = Vector2(0.0f, 0.0f);

	vertices[1].position = Vector3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].texC = Vector2(1.0f, 1.0f);

	vertices[2].position = Vector3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].texC = Vector2(0.0f, 1.0f);

	// Second triangle.
	vertices[3].position = Vector3(left, top, 0.0f);  // Top left.
	vertices[3].texC = Vector2(0.0f, 0.0f);

	vertices[4].position = Vector3(right, top, 0.0f);  // Top right.
	vertices[4].texC = Vector2(1.0f, 0.0f);

	vertices[5].position = Vector3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].texC = Vector2(1.0f, 1.0f);

	DWORD* indices = new DWORD[6];

	// Load the index array with data.
	for (int i = 0; i < 6; i++)
		indices[i] = i;

	mRenderer = unique_ptr<D3DRenderer>(new D3DRenderer());
	result = mRenderer->InitializeBuffers(pD3dGraphicsObj->GetDevice(),indices,vertices,sizeof(VertexTypeTex),6,6);
	if (!result) {
		return false;
	}

	delete [] vertices;
	delete [] indices;	
	vertices = nullptr;
	indices = nullptr;

	return true;
}

bool D2DTexQuad::Render(const Matrix* viewMatrix, const Matrix* projMatrix) {
	mRenderer->RenderBuffers(pD3dGraphicsObj->GetDeviceContext());
	return mShader->Render(pD3dGraphicsObj->GetDeviceContext(),mRenderer->GetIndexCount(),pTextureResource);
}

D3DRenderer* D2DTexQuad::GetRenderer() {
	return mRenderer.get();
}

void D2DTexQuad::SetScale(float scaleX, float scaleY) {
	mTransformMatrix = Matrix::CreateScale(scaleX,scaleY,1.0f);
}

void D2DTexQuad::SetTexture(ID3D11ShaderResourceView* textureResource) {
	pTextureResource = textureResource;
}