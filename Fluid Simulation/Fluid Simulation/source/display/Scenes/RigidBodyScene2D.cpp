/***************************************************************
RigidBodyScene2D.cpp: Describes a scene that simulates 2D rigid body
dynamics

Author: Valentin Hinov
Date: 26/11/2013
***************************************************************/

#define MAX_NUM_PARTICLES 30
#define MAX_NUM_OBSTACLES 7

#include "RigidBodyScene2D.h"

#include <AntTweakBar.h>

#include "SpriteBatch.h"
#include "CommonStates.h"

#include "../D3DGraphicsObject.h"
#include "../../utilities/Camera.h"
#include "../../system/ServiceProvider.h"
#include "../../utilities/D3DTexture.h"
#include "../../objects/D2DParticle.h"
#include "../../utilities/math/MathUtils.h"
#include "../../utilities/Physics.h"

using namespace DirectX;

RigidBodyScene2D::RigidBodyScene2D() : mTwBar(nullptr) {
}

RigidBodyScene2D::~RigidBodyScene2D() {
	pD3dGraphicsObj = nullptr;
	while (!mParticleUnits.empty()) {
		D2DParticle *particle = mParticleUnits.back();
		if (particle) {
			delete particle;
			particle = nullptr;
		}
		mParticleUnits.pop_back();
	}
	mParticleUnits.clear();

	while (!mObstacles.empty()) {
		D2DParticle *particle = mObstacles.back();
		if (particle) {
			delete particle;
			particle = nullptr;
		}
		mObstacles.pop_back();
	}
	mObstacles.clear();

	int twResult = TwDeleteBar(mTwBar);
	if (twResult == 0) {
		// deletion failed, Call TwGetLastError to retrieve error
	}
	mTwBar = nullptr;
}

bool RigidBodyScene2D::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);

	mSpriteBatch = unique_ptr<SpriteBatch>(new SpriteBatch(pD3dGraphicsObj->GetDeviceContext()));

	bool result;
	int screenWidth,screenHeight;
	graphicsObject->GetScreenDimensions(screenWidth,screenHeight);
	for (int i = 0; i < MAX_NUM_PARTICLES; ++i) {
		float randMass = RandF(1.0f,10.0f);
		float randRadius = RandF(0.1f,0.4f);
		Vector3 randPos;
		randPos.x = RandF(0,screenWidth);
		randPos.y = RandF(0,screenHeight*0.25f);
		
		D2DParticle *particle = new D2DParticle(randMass,randPos,randRadius);
		result = particle->Initialize(pD3dGraphicsObj,hwnd,L"data/circle_sprite.png");
		if (!result) {
			return false;
		}
		mParticleUnits.push_back(particle);
	}

	for (int i = 0; i < MAX_NUM_OBSTACLES; ++i) {
		float mass = 100.0f;
		float radius = 1.2f;
		Vector3 randPos;
		randPos.x = RandF(0,screenWidth - 40);
		randPos.y = RandF(screenHeight*0.25f,screenHeight - 40);
		
		D2DParticle *particle = new D2DParticle(mass,randPos,radius);
		result = particle->Initialize(pD3dGraphicsObj,hwnd,L"data/circle_sprite.png");
		if (!result) {
			return false;
		}
		mObstacles.push_back(particle);
	}

	// Initialize this scene's tweak bar
	mTwBar = TwNewBar("2D Rigid Bodies");
	// Position bar
	int barPos[2] = {580,2};
	TwSetParam(mTwBar,nullptr,"position", TW_PARAM_INT32, 2, barPos);
	int barSize[2] = {218,150};
	TwSetParam(mTwBar,nullptr,"size", TW_PARAM_INT32, 2, barSize);

	// Add Variables to tweak bar
	TwAddVarRW(mTwBar,"Gravity", TW_TYPE_FLOAT, &Physics::fGravity, "step=0.1");
	TwAddVarRW(mTwBar,"Air Density", TW_TYPE_FLOAT, &Physics::fAirDensity, "step=0.01");
	TwAddVarRW(mTwBar,"Drag Coefficient", TW_TYPE_FLOAT, &Physics::fDragCoefficient, "step=0.05");
	TwAddVarRW(mTwBar,"Restitution", TW_TYPE_FLOAT, &Physics::fRestitution, "step=0.05");
	//TwAddVarRW(mTwBar,"MacCormarck Advection", TW_TYPE_BOOLCPP, &mMacCormackEnabled, nullptr);
	//TwAddVarRW(mTwBar,"Simulation Paused", TW_TYPE_BOOLCPP, &mPaused, nullptr);

	return true;
}

void RigidBodyScene2D::Update(float delta) {

}

void RigidBodyScene2D::FixedUpdate(float fixedDelta) {
	for (D2DParticle *particle : mParticleUnits) {
		particle->FixedUpdate(fixedDelta,mObstacles);
	}
}

bool RigidBodyScene2D::Render() {
	IGraphicsSystem *pGraphicsSystem = ServiceProvider::Instance().GetGraphicsSystem();
	mSpriteBatch->Begin(SpriteSortMode_Deferred,pGraphicsSystem->GetCommonD3DStates()->NonPremultiplied());
	{
		// Draw particles
		for (D2DParticle *particle : mParticleUnits) {
			particle->Render(mSpriteBatch.get());
		}

		// Draw obstacles
		for (D2DParticle *obstacle : mObstacles) {
			obstacle->Render(mSpriteBatch.get());
		}
	}
	mSpriteBatch->End();

	return true;
}