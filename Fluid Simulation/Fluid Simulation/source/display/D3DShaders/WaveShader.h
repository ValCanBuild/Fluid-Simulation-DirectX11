/*************************************************************
WaveShader.h: Describes a shader that solves the 2D wave
equation

Author: Valentin Hinov
Date: 06/09/2013
Version: 1.0
**************************************************************/
#ifndef _WAVESHADER_H
#define _WAVESHADER_H

#include "BaseD3DShader.h"

class D3DGraphicsObject;

class WaveShader : public BaseD3DShader {
public:
	WaveShader();
	~WaveShader();

	bool Render(D3DGraphicsObject* graphicsObject, int indexCount, ID3D11ShaderResourceView** texArr);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

private:
	struct ScreenSizeBuffer {
		float screenWidth;
		float screenHeight;
		Vector2f padding;
	};

	std::unique_ptr<ID3D11Buffer,COMDeleter> mScreenSizeBuffer;
	std::unique_ptr<ID3D11SamplerState,COMDeleter> mSampleState;
};

#endif