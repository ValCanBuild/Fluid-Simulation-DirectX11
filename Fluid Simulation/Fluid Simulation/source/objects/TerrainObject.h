/********************************************************************
TerrainObject.h: Represents a terrain game object.

Author:	Valentin Hinov
Date: 24/3/2014
*********************************************************************/

#ifndef _TERRAINOBECT_H
#define	_TERRAINOBECT_H

#include "GameObject.h"
#include "../utilities/D3dIncludes.h"

struct HeightMap;
class ICamera;
class D3DRenderer;
class D3DGraphicsObject;
class TerrainTextureShader;
class D3DTexture;

namespace DirectX
{
	struct VertexPositionNormalTexture;
}

class TerrainObject : public GameObject {
public:
	~TerrainObject();
	static std::unique_ptr<TerrainObject> CreateFromHeightMap(const HeightMap &heightMap, D3DGraphicsObject * const d3dGraphicsObj);

	void Render(const ICamera &camera);

private:
	TerrainObject(D3DGraphicsObject * const d3dGraphicsObj);
	static void ComputeNormals(std::unique_ptr<DirectX::VertexPositionNormalTexture[]> &vertices, int gridWidth, int gridDepth);
private:
	std::unique_ptr<D3DRenderer> mRenderer;
	std::unique_ptr<TerrainTextureShader> mShader;
	std::unique_ptr<D3DTexture[]> mTextures;
	D3DGraphicsObject * pD3dGraphicsObj;

};

#endif
