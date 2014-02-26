/***************************************************************
Fluid2DScene.h: Describes a scene that displays a 2D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 10/09/2013
***************************************************************/
#ifndef _FLUID2DSCENE_H
#define _FLUID2DSCENE_H

#include "../../utilities/AtlInclude.h"

#include <vector>
#include <memory>
#include "IScene.h"

using namespace std;

namespace Fluid2D
{
	class Fluid2DSimulator;
}

enum FluidPropertyType_t;
class D2DTexQuad;
class Camera;
class D3DGraphicsObject;

class Fluid2DScene : public IScene {
public:
	Fluid2DScene();
	~Fluid2DScene();

	bool Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd);
	void Update(float delta);
	bool Render();

private:
	void ChangeFluidPropertyView();
	void HandleInput();

private:
	TwBar *mTwBar;

	unique_ptr<Fluid2D::Fluid2DSimulator>			mFluid2DEffect;
	unique_ptr<Camera>					mCamera;

	D3DGraphicsObject* pD3dGraphicsObj;

	FluidPropertyType_t fluidProperty;
	bool mPaused;
};

#endif