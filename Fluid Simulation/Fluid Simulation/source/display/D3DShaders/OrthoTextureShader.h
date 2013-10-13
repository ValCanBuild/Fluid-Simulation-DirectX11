/*************************************************************
OrthoTextureShader.h: Describes a simple shader that
orthographically renders a texture

Author: Valentin Hinov
Date: 03/09/2013
Version: 1.0
**************************************************************/
#ifndef _ORTHOTEXTURESHADER_H
#define _ORTHOTEXTURESHADER_H

#include "BaseD3DShader.h"

class OrthoTextureShader : public BaseD3DShader {
public:
	OrthoTextureShader();
	~OrthoTextureShader();

	bool Render(ID3D11DeviceContext* context, int indexCount, ID3D11ShaderResourceView* texture);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

	CComPtr<ID3D11SamplerState> mSampleState;
};

#endif