/*************************************************************
OrthoTextureShader.h: Describes a simple shader that
orthographically renders a color

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/
#ifndef _ORTHOCOLORSHADER_H
#define _ORTHOCOLORSHADER_H

#include "BaseD3DShader.h"

class OrthoColorShader : public BaseD3DShader {
public:
	OrthoColorShader();
	~OrthoColorShader();

	bool Render(ID3D11DeviceContext* context, int indexCount, Matrix *worldMatrix);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

private:
	CComPtr<ID3D11Buffer> mMatrixBuffer;

private:
	struct MatrixBufferType {
		Matrix world;
	};
};

#endif