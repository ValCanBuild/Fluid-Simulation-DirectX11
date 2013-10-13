/*************************************************************
Fluid2DShaders.h: Contains all the shader classes needed for
2D fluid simulation

Author: Valentin Hinov
Date: 11/09/2013
**************************************************************/
#ifndef _FLUID2DSHADERS_H
#define _FLUID2DSHADERS_H

#include "BaseD3DShader.h"

class D3DGraphicsObject;

class AdvectionShader : public BaseD3DShader {
public:
	AdvectionShader();
	~AdvectionShader();

	bool Render(D3DGraphicsObject* graphicsObject, int indexCount, float timeStep, float dissipation, ID3D11ShaderResourceView* velocityField, ID3D11ShaderResourceView* advectTarget);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

private:
	struct InputBuffer {
		float fTextureWidth;
		float fTextureHeight;
		float fTimeStep;
		float fDissipation;
	};

	CComPtr<ID3D11Buffer>		mInputBuffer;
	CComPtr<ID3D11SamplerState> mSampleState;
};


class ImpulseShader : public BaseD3DShader {
public:
	ImpulseShader();
	~ImpulseShader();

	bool Render(D3DGraphicsObject* graphicsObject, int indexCount, Vector2 point, Vector2 fill, float radius, ID3D11ShaderResourceView* originalState);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

private:
	struct InputBuffer {
		Vector2 vPoint;	// the point of user interaction, must be in normalized window coordinates [0,1]
		Vector2 vFillColor;	// the color which will represent the impulse magnitudes in the X and Y directions
		float  fRadius;	// the extent to which the point of interaction affects neighbouring pixels
		Vector3 padding;	//padding to align to 16 bytes
	};

	CComPtr<ID3D11Buffer>		mInputBuffer;
	CComPtr<ID3D11SamplerState> mSampleState;
};


class JacobiShader : public BaseD3DShader {
public:
	JacobiShader();
	~JacobiShader();

	bool Render(D3DGraphicsObject* graphicsObject, int indexCount, float alpha, float inverseBeta, ID3D11ShaderResourceView* pressureField, ID3D11ShaderResourceView* divergence);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

private:
	struct InputBuffer {
		float fTextureWidth;
		float fTextureHeight;
		float fAlpha;
		float fInverseBeta;
	};

	CComPtr<ID3D11Buffer>		mInputBuffer;
	CComPtr<ID3D11SamplerState> mSampleState;
};


class DivergenceShader : public BaseD3DShader {
public:
	DivergenceShader();
	~DivergenceShader();

	bool Render(D3DGraphicsObject* graphicsObject, int indexCount, float halfInverseCellSize, ID3D11ShaderResourceView* targetField);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

private:
	struct InputBuffer {
		float fTextureWidth;
		float fTextureHeight;
		float fHalfInverseCellSize;
		float padding;	//padding to align to 16 bytes
	};

	CComPtr<ID3D11Buffer>		mInputBuffer;
	CComPtr<ID3D11SamplerState> mSampleState;
};


class SubtractGradientShader : public BaseD3DShader {
public:
	SubtractGradientShader();
	~SubtractGradientShader();

	bool Render(D3DGraphicsObject* graphicsObject, int indexCount, float gradientScale, ID3D11ShaderResourceView* velocityField, ID3D11ShaderResourceView* pressureField);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

private:
	struct InputBuffer {
		float fTextureWidth;
		float fTextureHeight;
		float fGradientScale;	//usually is 0.5f/gridScale
		float padding0;	//padding to align to 16 bytes
	};

	CComPtr<ID3D11Buffer>		mInputBuffer;
	CComPtr<ID3D11SamplerState> mSampleStateVelocity;
	CComPtr<ID3D11SamplerState> mSampleStatePressure;
};


class BuoyancyShader : public BaseD3DShader {
public:
	BuoyancyShader();
	~BuoyancyShader();

	bool Render(D3DGraphicsObject* graphicsObject, int indexCount, float timeStep, float buoyancy, float weight, float ambTemp, ID3D11ShaderResourceView* velocityField, ID3D11ShaderResourceView* temperatureField, ID3D11ShaderResourceView* density);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

private:
	struct InputBuffer {
		float fAmbientTemperature;
		float fTimeStep;
		float fSigma;
		float fKappa;	
		//Vector4 padding1; // padding to align to 32 bytes
	};

	CComPtr<ID3D11Buffer>		mInputBuffer;
	CComPtr<ID3D11SamplerState> mSampleState;
};

#endif