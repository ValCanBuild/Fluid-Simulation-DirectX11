/********************************************************************
TerrainObject.cpp: Implementation of TerrainObject

Author:	Valentin Hinov
Date: 24/3/2014
*********************************************************************/

#include "TerrainObject.h"
#include <VertexTypes.h>
#include "../utilities/ICamera.h"
#include "../utilities/HeightMap.h"
#include "../utilities/D3DTexture.h"
#include "../display/D3DRenderer.h"
#include "../display/D3DGraphicsObject.h"
#include "../display/D3DShaders/TerrainTextureShader.h"

using namespace std;
using namespace DirectX;

Vector3 XMFLOAT3ToVec3(XMFLOAT3 floatVec) {
	return Vector3(floatVec.x, floatVec.y, floatVec.z);
}

TerrainObject::TerrainObject(D3DGraphicsObject * const d3dGraphicsObj) : pD3dGraphicsObj(d3dGraphicsObj) {
	transform = std::shared_ptr<Transform>(new Transform(this));
	bounds = std::shared_ptr<Bounds>(new Bounds(this,BOUNDS_TYPE_BOX));
	transform->scale = Vector3(0.2f);
}

TerrainObject::~TerrainObject() {
	pD3dGraphicsObj = nullptr;
}

unique_ptr<TerrainObject> TerrainObject::CreateFromHeightMap(const HeightMap &heightMap, D3DGraphicsObject * const d3dGraphicsObj) {
	unique_ptr<TerrainObject> terrainObject(new TerrainObject(d3dGraphicsObj));

	terrainObject->mRenderer = unique_ptr<D3DRenderer>(new D3DRenderer());
	int gridWidth = heightMap.terrainWidth;
	int gridDepth = heightMap.terrainDepth;
	size_t numVerts = heightMap.heights.size();

	unique_ptr<VertexPositionNormalTexture[]> vertices(new VertexPositionNormalTexture[numVerts]);

	float dx = 1.0f; // cellspacing
	float halfWidth = (gridWidth-1)*dx*0.5f;
	float halfDepth = (gridDepth-1)*dx*0.5f;	

	int index = 0;
	int textureRepeatAmount = 16;
	float widthRepeat = (float)(gridWidth/textureRepeatAmount);
	float depthRepeat = (float)(gridDepth/textureRepeatAmount);
	// Create vertices position
	for(int i = 0; i < gridWidth; ++i){
		float z = halfDepth - i*dx;		
		for(int j = 0; j < gridDepth; ++j){
			float x = -halfWidth + j*dx;
			index = i*gridWidth+j;
			float y = heightMap.heights[index];
			vertices[index].position = Vector3(x, y, z);
			vertices[index].textureCoordinate = Vector2(i/widthRepeat, j/depthRepeat);
		}
	}

	ComputeNormals(vertices, gridWidth, gridDepth);

	// Create indices
	int numIndices = ((gridWidth-1)*(gridDepth-1)*6);
	unique_ptr<DWORD[]> indices(new DWORD[numIndices]);
	int k = 0;
	bool switchPtrn = false;
	for(int i = 0; i < gridWidth-1; ++i) {
		for(int j = 0; j < gridDepth-1; ++j) {
			if (switchPtrn == false) {
				indices[k] = i*gridDepth+j;
				indices[k+1] = i*gridDepth+j+1;
				indices[k+2] = (i+1)*gridDepth+j;

				indices[k+3] = (i+1)*gridDepth+j;
				indices[k+4] = i*gridDepth+j+1;
				indices[k+5] = (i+1)*gridDepth+j+1;
			}
			else {
				indices[k] = i*gridDepth+j;
				indices[k+1] = i*gridDepth+j+1;
				indices[k+2] = (i+1)*gridDepth+j+1;

				indices[k+3] = (i+1)*gridDepth+j+1;
				indices[k+4] = (i+1)*gridDepth+j;
				indices[k+5] = i*gridDepth+j;
			}
			k += 6; // next quad
			switchPtrn = !switchPtrn;
		}
		switchPtrn = (gridWidth % 2 == 1) ?  !switchPtrn : switchPtrn;
	}

	auto d3dDevice = d3dGraphicsObj->GetDevice();
	auto deviceContext = d3dGraphicsObj->GetDeviceContext();
	bool result = terrainObject->mRenderer->InitializeBuffers(d3dDevice, indices.get(), vertices.get(), sizeof(VertexPositionNormalTexture), numVerts, numIndices);
	if (!result) {
		return nullptr;
	}

	int numTextures = 3;
	terrainObject->mTextures = unique_ptr<D3DTexture[]>(new D3DTexture[numTextures]);
	for (int i = 0; i < numTextures; ++i) {
		WCHAR* name;
		if (i == 0) {
			name = L"data/textures/terrain/grass0.dds";
		} else if (i == 1) {
			name = L"data/textures/terrain/ground0.dds";
		} else if (i == 2) {
			name = L"data/textures/terrain/stone0.dds";
		}
		if (!terrainObject->mTextures[i].Initialize(d3dDevice, deviceContext, name)) {
			return nullptr;
		}
	}

	terrainObject->mShader = unique_ptr<TerrainTextureShader>(new TerrainTextureShader);
	if (!terrainObject->mShader->Initialize(d3dDevice)) {
		return nullptr;
	}

	ID3D11ShaderResourceView *pSRVs[3];
	for (int i = 0; i < numTextures; ++i) {
		pSRVs[i] = terrainObject->mTextures[i].GetTexture();
	}
	terrainObject->mShader->SetTextures(pSRVs);

	float heights[3] = {0, 5, 14};
	terrainObject->mShader->SetPixelBufferValues(deviceContext, heights);

	return terrainObject;
}

void TerrainObject::Render(const ICamera &camera) {
	auto deviceContext = pD3dGraphicsObj->GetDeviceContext();
	Matrix worldMat;
	transform->GetTransformMatrixQuaternion(worldMat);
	Matrix wvpMatrix = worldMat*camera.GetViewMatrix()*camera.GetProjectionMatrix();
	mShader->SetVertexBufferValues(deviceContext, wvpMatrix, worldMat);
	mRenderer->RenderBuffers(deviceContext);
	mShader->Render(deviceContext, mRenderer->GetIndexCount());
}

void TerrainObject::ComputeNormals(unique_ptr<VertexPositionNormalTexture[]> &vertices, int gridWidth, int gridDepth) {
	for (int i = 0; i < gridWidth; i++) {
		for (int j = 0; j < gridDepth; j++) {
			int index = i*gridDepth+j;
			Vector3 normal;
			Vector3 v1,v2,v3,v4,v12,v23,v34,v41;

			if (j != gridWidth - 1){
				v1 = XMFLOAT3ToVec3(vertices[i*gridDepth+j+1].position) - XMFLOAT3ToVec3(vertices[index].position);
			}
			if (i != gridWidth - 1){
				v2 = XMFLOAT3ToVec3(vertices[(i+1)*gridDepth+j].position) - XMFLOAT3ToVec3(vertices[index].position);
			}
			if (j > 0){
				v3 = XMFLOAT3ToVec3(vertices[i*gridDepth+j-1].position) - XMFLOAT3ToVec3(vertices[index].position);
			}
			if (i > 0){
				v4 = XMFLOAT3ToVec3(vertices[(i-1)*gridDepth+j].position) - XMFLOAT3ToVec3(vertices[index].position);
			}

			v1.Cross(v2).Normalize(v12);
			v2.Cross(v3).Normalize(v23);
			v3.Cross(v4).Normalize(v34);
			v4.Cross(v1).Normalize(v41);

			if (v12.Length() > 0.0f) {
				normal = normal + v12;		 
			}						 
			if (v23.Length() > 0.0f) {
				normal = normal + v23;		 
			}						 
			if (v34.Length() > 0.0f) {
				normal = normal + v34;		 
			}						 
			if (v41.Length() > 0.0f) {
				normal = normal + v41;
			}

			normal.Normalize();

			vertices[index].normal = normal;
		}
	}
}