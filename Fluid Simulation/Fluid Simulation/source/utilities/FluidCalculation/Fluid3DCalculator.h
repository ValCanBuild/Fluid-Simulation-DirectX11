/********************************************************************
Fluid3DCalculator.h: Encapsulates a 3D fluid simulation
being calculated on the GPU.

Author:	Valentin Hinov
Date: 18/2/2014
*********************************************************************/

#ifndef _FLUID3DCALCULATOR_H
#define _FLUID3DCALCULATOR_H

#include <vector>
#include <memory>
#include "../AtlInclude.h"

#include "../../display/D3DGraphicsObject.h"
#include "FluidSettings.h"

struct ShaderParams;

namespace Fluid3D {

class AdvectionShader;
class ImpulseShader;
class JacobiShader ;
class DivergenceShader;
class SubtractGradientShader;
class BuoyancyShader;

class Fluid3DCalculator {
public:
	Fluid3DCalculator(FluidSettings fluidSettings);
	~Fluid3DCalculator();

	bool Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd);
	void Process();

	ID3D11ShaderResourceView * GetVolumeTexture() const;

	const FluidSettings &GetFluidSettings() const;
	void SetFluidSettings(const FluidSettings &fluidSettings);

private:
	bool InitShaders(HWND hwnd);
	bool InitShaderParams(HWND hwnd);
	bool InitBuffersAndSamplers();

	void Advect(ShaderParams *target);
	void RefreshConstantImpulse();
	void ApplyBuoyancy();
	void CalculatePressureGradient();

	enum DissipationBufferType_t {
		DENSITY, VELOCITY, TEMPERATURE
	};
	void UpdateDissipationBuffer(DissipationBufferType_t bufferType);
	void UpdateGeneralBuffer();
	void UpdateImpulseBuffer(Vector3& point, Vector4& amount, float radius);

	int  GetUpdateDirtyFlags(const FluidSettings &newSettings) const;
private:
	D3DGraphicsObject* pD3dGraphicsObj;

	FluidSettings mFluidSettings;

	std::unique_ptr<AdvectionShader>			mForwardAdvectionShader;
	std::unique_ptr<AdvectionShader>			mBackwardAdvectionShader;
	std::unique_ptr<AdvectionShader>			mMacCormarckAdvectionShader;
	std::unique_ptr<ImpulseShader>				mImpulseShader;
	std::unique_ptr<JacobiShader>				mJacobiShader;
	std::unique_ptr<DivergenceShader>			mDivergenceShader;
	std::unique_ptr<SubtractGradientShader>		mSubtractGradientShader;
	std::unique_ptr<BuoyancyShader>				mBuoyancyShader;

	ShaderParams* mVelocitySP;
	ShaderParams* mDensitySP;
	ShaderParams* mTemperatureSP;
	ShaderParams* mPressureSP;
	ShaderParams* mObstacleSP;
	std::unique_ptr<ShaderParams>			mDivergenceSP;
	CComPtr<ID3D11RenderTargetView>			mPressureRenderTargets[2];

	CComPtr<ID3D11Buffer>					mInputBufferGeneral;
	CComPtr<ID3D11Buffer>					mInputBufferImpulse;
	CComPtr<ID3D11Buffer>					mInputBufferDensityDissipation;
	CComPtr<ID3D11Buffer>					mInputBufferVelocityDissipation;
	CComPtr<ID3D11Buffer>					mInputBufferTemperatureDissipation;
	CComPtr<ID3D11SamplerState>				mSampleState;
};

}

#endif