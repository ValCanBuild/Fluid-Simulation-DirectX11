/********************************************************************
Fluid3DCalculator.h: Encapsulates a 3D fluid simulation
being calculated on the GPU.

Author:	Valentin Hinov
Date: 18/2/2014
*********************************************************************/

#ifndef _FLUID3DCALCULATOR_H
#define _FLUID3DCALCULATOR_H

#include <map>
#include <array>
#include <memory>
#include "../AtlInclude.h"

#include "../../display/D3DGraphicsObject.h"
#include "FluidSettings.h"
#include "FluidResources.h"

namespace Fluid3D {

class AdvectionShader;
class ImpulseShader;
class ExtinguishmentImpulseShader;
class JacobiShader;
class DivergenceShader;
class SubtractGradientShader;
class BuoyancyShader;
class VorticityShader;
class ConfinementShader;

struct ExtraForce {
	Vector3 position;
	float radius;
	Vector3 amount;
};

class Fluid3DCalculator {
public:
	Fluid3DCalculator(const FluidSettings &fluidSettings);
	~Fluid3DCalculator();

	bool Initialize(_In_ D3DGraphicsObject * d3dGraphicsObj, HWND hwnd);
	void Process();

	void AddForce(const ExtraForce& force);

	// before computing all fluids, attach the resources they all share to the pipeline
	static void AttachCommonResources(ID3D11DeviceContext* context);

	ID3D11ShaderResourceView * GetVolumeTexture() const;
	// If simulating fire - get the reaction values texture
	ID3D11ShaderResourceView * GetReactionTexture() const;

	const FluidSettings &GetFluidSettings() const;
	FluidSettings * const GetFluidSettingsPointer() const;
	void SetFluidSettings(const FluidSettings &fluidSettings);

private:
	bool InitShaders(HWND hwnd);
	bool InitBuffersAndSamplers();

	void Advect(std::array<ShaderParams, 2> &target, SystemAdvectionType_t advectionType, float dissipation, float decay = 0.0f);
	void RefreshConstantImpulse();
	void ApplyExtraForces();
	void ApplyImpulse(std::array<ShaderParams, 2> &target, Vector3 &position, float amount, float radius);
	void ApplyBuoyancy();
	void ComputeVorticityConfinement();
	void CalculatePressureGradient();

	void UpdateAdvectionBuffer(float dissipation, float timeModifier, float decay);
	void UpdateGeneralBuffer();
	void UpdateImpulseBuffer1D(const Vector3& point, float amount, float radius, float extinguishment = 0.0f);
	void UpdateImpulseBuffer3D(const Vector3& point, const Vector3& amount, float radius, float extinguishment = 0.0f);

	int  GetUpdateDirtyFlags(const FluidSettings &newSettings) const;

private:
	D3DGraphicsObject* pD3dGraphicsObj;

	FluidSettings mFluidSettings;
	ExtraForce mExtraVelocityForce;
	bool mExtraVelocityAdded;

	std::unique_ptr<AdvectionShader>				mAdvectionShader;
	std::unique_ptr<AdvectionShader>				mMacCormarckAdvectionShader;
	std::unique_ptr<ImpulseShader>					mImpulseShader;
	std::unique_ptr<ExtinguishmentImpulseShader>	mExtinguishmentImpulseShader;
	std::unique_ptr<VorticityShader>				mVorticityShader;
	std::unique_ptr<ConfinementShader>				mConfinementShader;
	std::unique_ptr<JacobiShader>					mJacobiShader;
	std::unique_ptr<DivergenceShader>				mDivergenceShader;
	std::unique_ptr<SubtractGradientShader>			mSubtractGradientShader;
	std::unique_ptr<BuoyancyShader>					mBuoyancyShader;

	// Resources per object
	FluidResourcesPerObject mFluidResources;

	// Resources that can be shared
	CommonFluidResources mCommonResources;

	// Resources that can be shared between simulations are mapped by a Vector3 holding the size of the domain
	// If fluid calculation domains are of the same size, they can share the same common resources
	static std::map<Vector3, CommonFluidResources> commonResourcesMap;
	static CComPtr<ID3D11SamplerState>		sampleState;


	CComPtr<ID3D11Buffer>					mInputBufferGeneral;
	CComPtr<ID3D11Buffer>					mInputBufferImpulse;
	CComPtr<ID3D11Buffer>					mInputBufferAdvection;
};

}

#endif