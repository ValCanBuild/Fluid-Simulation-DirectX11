/*************************************************************
SkyShader.h: Implementation of a shader that renders
a sky cube texture

Author: Valentin Hinov
Date: 18/03/2014
**************************************************************/
#ifndef _SKYSHADER_H
#define _SKYSHADER_H

#include "BaseD3DShader.h"

class D3DGraphicsObject;

class SkyShader : public BaseD3DShader {
public:
	SkyShader(const D3DGraphicsObject * const d3dGraphicsObject);
	~SkyShader();

	void SetVertexBufferValues(Matrix &wvpMatrix, Matrix &worldMatrix) const;
	void SetSkyCubeTexture(ID3D11ShaderResourceView *skyTexture);
private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);
	void BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) override;
private:
	const D3DGraphicsObject * pD3dGraphicsObject;

	struct VertexInputBuffer {
		Matrix wvpMatrix;
		Matrix worldMatrix;
	};

	CComPtr<ID3D11Buffer>		mVertexInputBuffer;

	ID3D11ShaderResourceView *  pSkyTexture;
};

#endif