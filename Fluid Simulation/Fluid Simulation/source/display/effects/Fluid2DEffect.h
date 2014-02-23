/********************************************************************
Fluid2DEffect.h: Encapsulates a 2D fluid effect being calculated on the
GPU. Allows for interaction and choosing what fluid property to render.

Author:	Valentin Hinov
Date: 18/2/2014
*********************************************************************/

#ifndef _FLUID2DEFFECT_H
#define _FLUID2DEFFECT_H

#include <vector>
#include <memory>
#include "../../utilities/AtlInclude.h"


#include "../D3DGraphicsObject.h"
#include "../../objects/D2DTexQuad.h"

struct ShaderParams;
struct InputBufferGeneral;
struct InputBufferDissipation;
struct InputBufferImpulse;

enum FluidPropertyType_t {
	DENSITY = 0, TEMPERATURE = 1, VELOCITY = 2
};

namespace Fluid2D {

class AdvectionShader;
class ImpulseShader;
class JacobiShader ;
class DivergenceShader;
class SubtractGradientShader;
class BuoyancyShader;
class Fluid2DRenderShader; 

class Fluid2DEffect {
public:
	Fluid2DEffect();
	~Fluid2DEffect();

	bool Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd);
	void ProcessEffect();

	bool Render(FluidPropertyType_t fluidPropertyType);

	void AddObstacle(Vector2 &pos, float radius);
	void AddVelocity(Vector2 &pos, Vector2& amount, float radius);
	void AddDensity(Vector2 &pos, Vector2& amount, float radius);

public:
	int jacobiIterations;
	float timeStep;
	bool macCormackEnabled;

private:
	void Advect(ShaderParams *target);
	void RefreshConstantImpulse();
	void ApplyBuoyancy();
	void CalculatePressureGradient();
	void SetGeneralBuffer();
	void SetImpulseBuffer(Vector2& point, Vector2& amount, float radius);
	void SetDissipationBuffer(float dissipation);

private:
	D3DGraphicsObject* pD3dGraphicsObj;

	std::unique_ptr<D2DTexQuad>				mTexQuad;

	std::unique_ptr<AdvectionShader>			mForwardAdvectionShader;
	std::unique_ptr<AdvectionShader>			mBackwardAdvectionShader;
	std::unique_ptr<AdvectionShader>			mMacCormarckAdvectionShader;
	std::unique_ptr<ImpulseShader>			mImpulseShader;
	std::unique_ptr<JacobiShader>			mJacobiShader;
	std::unique_ptr<DivergenceShader>		mDivergenceShader;
	std::unique_ptr<SubtractGradientShader>	mSubtractGradientShader;
	std::unique_ptr<BuoyancyShader>			mBuoyancyShader;
	std::unique_ptr<Fluid2DRenderShader>		mFluidRenderShader;

	ShaderParams* mVelocitySP;
	ShaderParams* mDensitySP;
	ShaderParams* mTemperatureSP;
	ShaderParams* mPressureSP;
	ShaderParams* mObstacleSP;
	std::unique_ptr<ShaderParams>			mDivergenceSP;
	CComPtr<ID3D11RenderTargetView>		mPressureRenderTargets[2];

	CComPtr<ID3D11Buffer>				mInputBufferGeneral;
	CComPtr<ID3D11Buffer>				mInputBufferImpulse;
	CComPtr<ID3D11Buffer>				mInputBufferDissipation;
	CComPtr<ID3D11SamplerState>			mSampleState;
};

}

#endif