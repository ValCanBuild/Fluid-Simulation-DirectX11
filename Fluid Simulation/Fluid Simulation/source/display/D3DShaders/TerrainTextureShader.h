/*************************************************************
TerrainTextureShader.h: Describes a shader that textures
a terrain

Author: Valentin Hinov
Date: 03/09/2013
**************************************************************/
#ifndef _TERRAINTEXTURESHADER_H
#define _TERRAINTEXTURESHADER_H

#include "BaseD3DShader.h"

class TerrainTextureShader : public BaseD3DShader {
public:
	TerrainTextureShader();
	~TerrainTextureShader();

	void SetTextures(ID3D11ShaderResourceView *textures[3]);
	void SetPixelBufferValues(ID3D11DeviceContext* context, float heightsArray[3]) const;
	void SetVertexBufferValues(ID3D11DeviceContext* context, const Matrix &wvpMatrix, const Matrix &worldMatrix) const;
	void Render(ID3D11DeviceContext* context, int indexCount);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);
	void BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) override;

	struct InputBufferPixel {
		float heights[3];
		float padding;
	};

	struct InputBufferVertex {
		Matrix worldMatrix;
		Matrix wvpMatrix;
	};

	CComPtr<ID3D11Buffer>		mConstantPixelBuffer;
	CComPtr<ID3D11Buffer>		mConstantVertexBuffer;
	CComPtr<ID3D11SamplerState> mSampleState;
	ID3D11ShaderResourceView *  pTextures[3];

};

#endif