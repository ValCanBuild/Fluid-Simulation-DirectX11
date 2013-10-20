/*************************************************************
ComputeFluid2DShaders.h: Contains all the shader classes needed for
2D fluid simulation as well as utilities for resource methods

Author: Valentin Hinov
Date: 18/09/2013
**************************************************************/
#ifndef _COMPUTEFLUID2DSHADERS_H
#define _COMPUTEFLUID2DSHADERS_H

#include "BaseD3DShader.h"

class D3DGraphicsObject;

// Struct to encapsulate the common resources required for the shaders
struct ShaderParams {
	CComPtr<ID3D11ShaderResourceView>	mSRV;
	CComPtr<ID3D11UnorderedAccessView>  mUAV;
};

struct InputBufferGeneral {
	float fTimeStep;

	float fBuoyancy;	
	float fDensityWeight;		
	float fAmbientTemperature;  				

	float fAlpha;				
	float fInverseBeta;			

	float fHalfInverseCellSize;	

	float fGradientScale;		

	Vector2 vDimensions;

	Vector2 padding0;	// pad to 48 bytes
};

struct InputBufferDissipation {
	float fDissipation;
	Vector3 padding1;
};

struct InputBufferImpulse {
	Vector2 vPoint;				
	Vector2 vFillColor;			
	float fRadius;

	Vector3 padding2; // pad to 32 bytes
};

class AdvectionShader : public BaseD3DShader {
public:
	AdvectionShader();
	~AdvectionShader();

	bool Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* advectTarget, _In_ ShaderParams* advectResult);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device) {return true;};
};


class ImpulseShader : public BaseD3DShader {
public:
	ImpulseShader();
	~ImpulseShader();

	bool Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* impulseInitial, _In_ ShaderParams* impulseResult);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device) {return true;};
};


class JacobiShader : public BaseD3DShader {
public:
	JacobiShader();
	~JacobiShader();

	bool Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* pressureField, _In_ ShaderParams* divergence, _In_ ShaderParams* pressureResult);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device) {return true;};
};


class DivergenceShader : public BaseD3DShader {
public:
	DivergenceShader();
	~DivergenceShader();

	bool Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* divergenceResult);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device) {return true;};
};


class SubtractGradientShader : public BaseD3DShader {
public:
	SubtractGradientShader();
	~SubtractGradientShader();

	bool Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* pressureField, _In_ ShaderParams* velocityResult);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device) {return true;};
};


class BuoyancyShader : public BaseD3DShader {
public:
	BuoyancyShader();
	~BuoyancyShader();

	bool Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* temperatureField, _In_ ShaderParams* density, _In_ ShaderParams* velocityResult);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device) {return true;};
};

#endif