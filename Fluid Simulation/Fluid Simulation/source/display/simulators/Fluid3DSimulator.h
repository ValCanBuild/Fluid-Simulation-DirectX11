/********************************************************************
Fluid3DSimulator.h: Encapsulates a 3D fluid simulation
being calculated on the GPU.

Author:	Valentin Hinov
Date: 18/2/2014
*********************************************************************/

#ifndef _FLUID3DSIMULATOR_H
#define _FLUID3DSIMULATOR_H

#include <vector>
#include <memory>
#include "../../utilities/AtlInclude.h"


#include "../D3DGraphicsObject.h"

struct ShaderParams;
struct InputBufferGeneral;
struct InputBufferDissipation;
struct InputBufferImpulse;

namespace Fluid3D {

class AdvectionShader;
class ImpulseShader;
class JacobiShader ;
class DivergenceShader;
class SubtractGradientShader;
class BuoyancyShader;

class Fluid3DSimulator {
public:
	Fluid3DSimulator(Vector3 dimensions);
	~Fluid3DSimulator();

	bool Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd);
	void ProcessEffect();

	ID3D11ShaderResourceView * GetVolumeTexture() const;

public:
	int jacobiIterations;
	float timeStep;
	bool macCormackEnabled;

private:
	Vector3 mDimensions;

private:
	void Advect(ShaderParams *target);
	void RefreshConstantImpulse();
	void ApplyBuoyancy();
	void CalculatePressureGradient();

	void SetGeneralBuffer();
	void SetImpulseBuffer(Vector4& point, Vector4& amount, float radius);
	void SetDissipationBuffer(float dissipation);

private:
	D3DGraphicsObject* pD3dGraphicsObj;

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
	CComPtr<ID3D11Buffer>					mInputBufferDissipation;
	CComPtr<ID3D11SamplerState>				mSampleState;
};

}

#endif