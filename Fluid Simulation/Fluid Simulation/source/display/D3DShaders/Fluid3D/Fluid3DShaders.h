/*************************************************************
Fluid3DShaders.h: Contains all the shader classes needed for
3D fluid simulation as well as utilities for resource methods

Author: Valentin Hinov
Date: 09/11/2013
**************************************************************/
#ifndef _FLUID3DSHADERS_H
#define _FLUID3DSHADERS_H

#include "../BaseD3DShader.h"
#include "../ShaderParams.h"

namespace Fluid3D {

class BaseFluid3DShader : public BaseD3DShader {
public:
	~BaseFluid3DShader();

protected:
	BaseFluid3DShader(Vector3 dimensions);	// base class cannot be created
	void Dispatch(_In_ ID3D11DeviceContext* context) const;

private:
	UINT mNumThreadGroupX, mNumThreadGroupY, mNumThreadGroupZ;
	void SetDimensions(const Vector3 &dimensions);

	ShaderDescription GetShaderDescription();
};

class AdvectionShader : public BaseFluid3DShader {
public:
	enum AdvectionShaderType_t {
		ADVECTION_TYPE_FORWARD,
		ADVECTION_TYPE_BACKWARD,
		ADVECTION_TYPE_MACCORMARCK
	};

public:
	AdvectionShader(AdvectionShaderType_t advectionType, Vector3 dimensions);
	~AdvectionShader();

	void Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* advectTarget, _In_ ShaderParams* advectResult);

private:
	ShaderDescription GetShaderDescription();

private:
	AdvectionShaderType_t mAdvectionType;
};


class ImpulseShader : public BaseFluid3DShader {
public:
	ImpulseShader(Vector3 dimensions);
	~ImpulseShader();

	void Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* impulseInitial, _In_ ShaderParams* impulseResult);

private:
	ShaderDescription GetShaderDescription();
};


class JacobiShader : public BaseFluid3DShader {
public:
	JacobiShader(Vector3 dimensions);
	~JacobiShader();

	void Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* pressureField, _In_ ShaderParams* divergence, _In_ ShaderParams* pressureResult);

private:
	ShaderDescription GetShaderDescription();
};


class DivergenceShader : public BaseFluid3DShader {
public:
	DivergenceShader(Vector3 dimensions);
	~DivergenceShader();

	void Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* divergenceResult);

private:
	ShaderDescription GetShaderDescription();
};


class SubtractGradientShader : public BaseFluid3DShader {
public:
	SubtractGradientShader(Vector3 dimensions);
	~SubtractGradientShader();

	void Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* pressureField, _In_ ShaderParams* velocityResult);

private:
	ShaderDescription GetShaderDescription();
};


class BuoyancyShader : public BaseFluid3DShader {
public:
	BuoyancyShader(Vector3 dimensions);
	~BuoyancyShader();

	void Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* temperatureField, _In_ ShaderParams* density, _In_ ShaderParams* velocityResult);

private:
	ShaderDescription GetShaderDescription();
};

class VorticityShader : public BaseFluid3DShader {
public:
	VorticityShader(Vector3 dimensions);
	~VorticityShader();

	void Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* vorticityResult);

private:
	ShaderDescription GetShaderDescription();
};

class ConfinementShader : public BaseFluid3DShader {
public:
	ConfinementShader(Vector3 dimensions);
	~ConfinementShader();

	void Compute(_In_ ID3D11DeviceContext* context, _In_ ShaderParams* velocityField, _In_ ShaderParams* vorticityField, _In_ ShaderParams* velocityResult);

private:
	ShaderDescription GetShaderDescription();
};

}// End namespace Fluid3D

#endif