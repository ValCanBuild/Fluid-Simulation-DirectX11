/***************************************************************
Fluid2DScene.cpp: Describes a scene that displays a 2D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 10/09/2013
***************************************************************/
#include <AntTweakBar.h>
#include "Fluid2DScene.h"

#include "../D3DGraphicsObject.h"
#include "../../objects/D2DTexQuad.h"
#include "../../system/ServiceProvider.h"
#include "../../system/InputSystem.h"
#include "../../utilities/FluidCalculation/Fluid2DCalculator.h"


#define INTERACTION_IMPULSE_RADIUS 7.0f
#define OBSTACLES_IMPULSE_RADIUS 5.0f

using namespace Fluid2D;

Fluid2DScene::Fluid2DScene() : 
	fluidProperty(DENSITY), 
	mPaused(false) {
	
}

Fluid2DScene::~Fluid2DScene() {
	pD3dGraphicsObj = nullptr;

	int twResult = TwDeleteBar(mTwBar);
	if (twResult == 0) {
		// deletion failed, Call TwGetLastError to retrieve error
	}
	mTwBar = nullptr;
}

bool Fluid2DScene::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);

	mFluid2DEffect = unique_ptr<Fluid2DCalculator>(new Fluid2DCalculator());
	bool result = mFluid2DEffect->Initialize(pD3dGraphicsObj, hwnd);
	if (!result) {
		return false;
	}
	
	// Initialize this scene's tweak bar
	mTwBar = TwNewBar("2D Fluid Simulation");
	// Position bar
	int barPos[2] = {580,2};
	TwSetParam(mTwBar,nullptr,"position", TW_PARAM_INT32, 2, barPos);
	int barSize[2] = {218,150};
	TwSetParam(mTwBar,nullptr,"size", TW_PARAM_INT32, 2, barSize);

	// Add Variables to tweak bar
	TwAddVarRW(mTwBar,"Jacobi Iterations", TW_TYPE_INT32, &mFluid2DEffect->jacobiIterations, "min=1 max=100 step=1");
	TwAddVarRW(mTwBar,"Time Step", TW_TYPE_FLOAT, &mFluid2DEffect->timeStep, "min=0.0 max=1.0 step=0.001");
	TwAddVarRW(mTwBar,"MacCormarck Advection", TW_TYPE_BOOLCPP, &mFluid2DEffect->macCormackEnabled, nullptr);
	TwAddVarRW(mTwBar,"Simulation Paused", TW_TYPE_BOOLCPP, &mPaused, nullptr);

	return true;
}

void Fluid2DScene::Update(float delta) {
	HandleInput();

	if (!mPaused) {
		mFluid2DEffect->ProcessEffect();
	}
}

bool Fluid2DScene::Render() {
	Matrix orthoMatrix;
	pD3dGraphicsObj->GetOrthoMatrix(orthoMatrix);
	
	bool result = mFluid2DEffect->Render(fluidProperty);

	return true;
}

void Fluid2DScene::ChangeFluidPropertyView() {
	int i = static_cast<int>(fluidProperty);
	++i;
	if (i > 2) {
		i = 0;
	}
	fluidProperty = static_cast<FluidPropertyType_t>(i);
}

void Fluid2DScene::HandleInput() {
	// mouse mid button adds obstacles
	auto inputSystem = ServiceProvider::Instance().GetService<InputSystem>();
	if (inputSystem->IsKeyClicked(VK_SHIFT)) {
		ChangeFluidPropertyView();
	}

	if (inputSystem->IsMouseMidDown()) {
		int x,y;
		inputSystem->GetMousePos(x,y);
		mFluid2DEffect->AddObstacle(Vector2((float)x,(float)y), OBSTACLES_IMPULSE_RADIUS);
	}

	else if (inputSystem->IsMouseLeftDown()) {
		int x,y;
		inputSystem->GetMousePos(x,y);
		int xDelta,yDelta;
		inputSystem->GetMouseDelta(xDelta,yDelta);
		mFluid2DEffect->AddDensity(Vector2((float)x,(float)y),Vector2(abs(xDelta*1.5f),abs(yDelta*1.5f)), INTERACTION_IMPULSE_RADIUS);
	}

	else if (inputSystem->IsMouseRightDown()) {
		int x,y;
		inputSystem->GetMousePos(x,y);
		int xDelta,yDelta;
		inputSystem->GetMouseDelta(xDelta,yDelta);
		mFluid2DEffect->AddVelocity(Vector2((float)x,(float)y),Vector2(abs(xDelta*1.5f),abs(yDelta*1.5f)), INTERACTION_IMPULSE_RADIUS);
	}
}
