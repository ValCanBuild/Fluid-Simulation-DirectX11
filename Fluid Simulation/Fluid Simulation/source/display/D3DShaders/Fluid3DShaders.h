/*************************************************************
Fluid3DShaders.h: Contains all the shader classes needed for
3D fluid simulation as well as utilities for resource methods

Author: Valentin Hinov
Date: 09/11/2013
**************************************************************/
#ifndef _FLUID3DSHADERS_H
#define _FLUID3DSHADERS_H

#include "BaseD3DShader.h"
#include "ShaderParams.h"

class D3DGraphicsObject;

namespace Fluid3D {

struct InputBufferGeneral {
	float fTimeStep;
	float fDensityBuoyancy;	
	float fDensityWeight;		
	float fAmbientTemperature; 
};

struct InputBufferDissipation {
	float fDissipation;
	Vector3 padding1;
};

struct InputBufferImpulse {
	Vector3 vPoint;				
	float fRadius;
	Vector4 vFillColor;			
};

class AdvectionShader : public BaseD3DShader {
public:
	enum AdvectionType_t {
		ADVECTION_TYPE_FORWARD,
		ADVECTION_TYPE_BACKWARD,
		ADVECTION_TYPE_MACCORMARCK
	};

public:
	AdvectionShader(AdvectionType_t advectionType);
	~AdvectionShader();

	bool Compute(_In_ D3DGraphicsObject* graphicsObject, _In_ ShaderParams* velocityField, _In_ ShaderParams* advectTarget, _In_ ShaderParams* advectResult);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device) {return true;};

private:
	AdvectionType_t mAdvectionType;
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

}// End namespace Fluid3D

#endif