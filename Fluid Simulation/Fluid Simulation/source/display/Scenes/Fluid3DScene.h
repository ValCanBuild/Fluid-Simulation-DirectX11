/***************************************************************
Fluid3DScene.h: Describes a scene that displays a 3D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 24/10/2013
***************************************************************/
#ifndef _FLUID3DSCENE_H
#define _FLUID3DSCENE_H

#include <atlbase.h>
#if defined (_DEBUG)
#pragma comment(lib,"atlsd.lib")
#endif

#include <vector>
#include <memory>
#include "IScene.h"
#include "../D3DShaders/Fluid3DShaders.h"

#include "GeometricPrimitive.h"

class D2DTexQuad;
class D3DTexture;
class Camera;
class D3DGraphicsObject;

using namespace std;
using namespace DirectX;
using namespace Fluid3D;

class Fluid3DScene : public IScene {
public:
	Fluid3DScene();
	~Fluid3DScene();

	bool Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd);
	void Update(float delta);
	bool Render();

private:
	void UpdateCamera(float delta);

	bool SetGeneralBuffer();
	bool SetImpulseBuffer(Vector4& point, Vector4& amount, float radius);
	bool SetDissipationBuffer(float dissipation);

	bool PerformComputation();

private:
	unique_ptr<Camera>					mCamera;
	unique_ptr<D2DTexQuad>				mTexQuad;
	
	unique_ptr<GeometricPrimitive>		mContainmentBox;

	unique_ptr<D3DTexture>				mTexture;

	D3DGraphicsObject* pD3dGraphicsObj;

	bool mPaused;
	float mAngle;

private:
	unique_ptr<Fluid3D::AdvectionShader>			mForwardAdvectionShader;
	unique_ptr<Fluid3D::AdvectionShader>			mBackwardAdvectionShader;
	unique_ptr<Fluid3D::AdvectionShader>			mMacCormarckAdvectionShader;
	unique_ptr<Fluid3D::ImpulseShader>			mImpulseShader;
	unique_ptr<Fluid3D::JacobiShader>			mJacobiShader;
	unique_ptr<Fluid3D::DivergenceShader>		mDivergenceShader;
	unique_ptr<Fluid3D::SubtractGradientShader>	mSubtractGradientShader;
	unique_ptr<Fluid3D::BuoyancyShader>			mBuoyancyShader;
	unique_ptr<Fluid3D::Fluid3DRenderShader>		mFluidRenderShader;

	Fluid3D::ShaderParams* mVelocitySP;
	Fluid3D::ShaderParams* mDensitySP;
	Fluid3D::ShaderParams* mTemperatureSP;
	Fluid3D::ShaderParams* mPressureSP;
	unique_ptr<Fluid3D::ShaderParams>	mDivergenceSP;
	CComPtr<ID3D11RenderTargetView>		mPressureRenderTargets[2];

	CComPtr<ID3D11Buffer>				mInputBufferGeneral;
	CComPtr<ID3D11Buffer>				mInputBufferImpulse;
	CComPtr<ID3D11Buffer>				mInputBufferDissipation;
	CComPtr<ID3D11SamplerState>			mSampleState;

	unique_ptr<Fluid3D::ShaderParams>	mFluidRenderResult;

private:
	float mZoom;
};

#endif