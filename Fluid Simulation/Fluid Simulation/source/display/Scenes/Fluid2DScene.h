/***************************************************************
Fluid2DScene.h: Describes a scene that displays a 2D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 10/09/2013
***************************************************************/
#ifndef _FLUID2DSCENE_H
#define _FLUID2DSCENE_H

#include <atlbase.h>
#if defined (_DEBUG)
#pragma comment(lib,"atlsd.lib")
#endif

#include <vector>
#include <memory>
#include "IScene.h"

class D2DTexQuad;
class Camera;
class D3DGraphicsObject;
class AdvectionShader;
class ImpulseShader;
class JacobiShader;
class DivergenceShader;
class SubtractGradientShader;
class BuoyancyShader;
struct ShaderParams;
struct InputBufferGeneral;
struct InputBufferDissipation;
struct InputBufferImpulse;

struct ID3D11RenderTargetView;
struct ID3D11Buffer;
struct ID3D11SamplerState;

using namespace std;

class Fluid2DScene : public IScene {
public:
	Fluid2DScene();
	~Fluid2DScene();

	bool Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd);
	void Update(float delta);
	bool Render();

private:
	bool SetGeneralBuffer();
	bool SetImpulseBuffer(Vector2& point, Vector2& amount, float radius);
	bool SetDissipationBuffer(float dissipation);

	bool PerformComputation();

private:
	TwBar *mTwBar;

private:
	unique_ptr<Camera>					mCamera;
	unique_ptr<D2DTexQuad>				mTexQuad;

	unique_ptr<AdvectionShader>			mForwardAdvectionShader;
	unique_ptr<AdvectionShader>			mBackwardAdvectionShader;
	unique_ptr<AdvectionShader>			mMacCormarckAdvectionShader;
	unique_ptr<ImpulseShader>			mImpulseShader;
	unique_ptr<JacobiShader>			mJacobiShader;
	unique_ptr<DivergenceShader>		mDivergenceShader;
	unique_ptr<SubtractGradientShader>	mSubtractGradientShader;
	unique_ptr<BuoyancyShader>			mBuoyancyShader;

	ShaderParams* mVelocitySP;
	ShaderParams* mDensitySP;
	ShaderParams* mTemperatureSP;
	ShaderParams* mPressureSP;
	ShaderParams* mObstacleSP;
	unique_ptr<ShaderParams>			mDivergenceSP;
	CComPtr<ID3D11RenderTargetView>		mPressureRenderTargets[2];

	CComPtr<ID3D11Buffer>				mInputBufferGeneral;
	CComPtr<ID3D11Buffer>				mInputBufferImpulse;
	CComPtr<ID3D11Buffer>				mInputBufferDissipation;
	CComPtr<ID3D11SamplerState>			mSampleState;

	D3DGraphicsObject* pD3dGraphicsObj;

	int textureShowing;
	bool mPaused;

// Simulation Variables
private:
	int mJacobiIterations;
	float mTimeStep;
	bool mMacCormackEnabled;
};

#endif