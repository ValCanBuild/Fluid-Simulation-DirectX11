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

	void PrepareForRender(ID3D11DeviceContext* context, ID3D11ShaderResourceView* texture, const Matrix *wvpMatrix);
	bool Render(ID3D11DeviceContext* context, int indexCount);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

	struct InputBuffer {
		Matrix wvpMatrix;
	};

	CComPtr<ID3D11Buffer>		mInputBuffer;
	CComPtr<ID3D11SamplerState> mSampleState;
};

#endif