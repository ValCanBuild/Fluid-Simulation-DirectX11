/***************************************************************
D2DColorQuad.cpp: A Direct3D renderable which represents a 2D
colored ortho quad with
initial size of 1, which when rendered covers the whole screen.

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#include "D2DColorQuad.h"
#include "../display/D3DGraphicsObject.h"
#include "../display/D3DShaders/OrthoColorShader.h"

struct VertexType {
	Vector3 position;
	Color color;
};

D2DColorQuad::D2DColorQuad() {
	pD3dGraphicsObj = nullptr;
}

D2DColorQuad::~D2DColorQuad() {
	pD3dGraphicsObj = nullptr;
}

bool D2DColorQuad::Initialize(IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	mShader = unique_ptr<OrthoColorShader>(new OrthoColorShader());
	bool result = mShader->Initialize(pD3dGraphicsObj->GetDevice(), hwnd);
	if (!result)
		return false;

	VertexType* vertices = new VertexType[6];

	float left = -1.0f;
	float right = 1.0f;
	float top = 1.0f;
	float bottom = -1.0f;

	// Load the vertex array with data.
	// First triangle.
	vertices[0].position = Vector3(left, top, 0.0f);  // Top left.
	vertices[0].color = Color(1.0f, 1.0f, 1.0f, 1.0f);

	vertices[1].position = Vector3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].color = Color(1.0f, 1.0f, 1.0f, 1.0f);

	vertices[2].position = Vector3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].color = Color(1.0f, 1.0f, 1.0f, 1.0f);

	// Second triangle.
	vertices[3].position = Vector3(left, top, 0.0f);  // Top left.
	vertices[3].color = Color(1.0f, 1.0f, 1.0f, 1.0f);

	vertices[4].position = Vector3(right, top, 0.0f);  // Top right.
	vertices[4].color = Color(1.0f, 1.0f, 1.0f, 1.0f);

	vertices[5].position = Vector3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].color = Color(1.0f, 1.0f, 1.0f, 1.0f);

	DWORD* indices = new DWORD[6];

	// Load the index array with data.
	for (int i = 0; i < 6; i++)
		indices[i] = i;

	mRenderer = unique_ptr<D3DRenderer>(new D3DRenderer());
	result = mRenderer->InitializeBuffers(pD3dGraphicsObj->GetDevice(),indices,vertices,sizeof(VertexType),6,6);
	if (!result) {
		return false;
	}

	delete [] vertices;
	delete [] indices;	
	vertices = nullptr;
	indices = nullptr;

	return true;
}

bool D2DColorQuad::Render(const Matrix* viewMatrix, const Matrix* projMatrix) {
	mRenderer->RenderBuffers(pD3dGraphicsObj->GetDeviceContext());
	Matrix transformMatrix;
	mTransform.GetTransformMatrixQuaternion(transformMatrix);
	return mShader->Render(pD3dGraphicsObj->GetDeviceContext(),mRenderer->GetIndexCount(), &transformMatrix);
}

void D2DColorQuad::SetColor(Color colorV) {
	
}