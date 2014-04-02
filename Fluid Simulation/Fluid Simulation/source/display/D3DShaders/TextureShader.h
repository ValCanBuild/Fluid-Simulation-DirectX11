/*************************************************************
TextureShader.h: Describes a simple shader that textures a 
3D object

Author: Valentin Hinov
Date: 03/09/2013
**************************************************************/
#ifndef _TEXTURESHADER_H
#define _TEXTURESHADER_H

#include "BaseD3DShader.h"

class TextureShader : public BaseD3DShader {
public:
	TextureShader();
	~TextureShader();

	void SetTexture(ID3D11ShaderResourceView *texture);
	void SetVertexBufferValues(ID3D11DeviceContext* context, const Matrix &wvpMatrix) const;
	void Render(ID3D11DeviceContext* context, int indexCount);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);
	void BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) override;

	struct InputBuffer {
		Matrix wvpMatrix;
	};

	CComPtr<ID3D11Buffer>		mInputBuffer;
	CComPtr<ID3D11SamplerState> mSampleState;
	ID3D11ShaderResourceView *  pTexture;

};

#endif