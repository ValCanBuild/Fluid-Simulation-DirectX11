/***************************************************************
Fluid3DScene.h: Describes a scene that displays a 3D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 24/10/2013
***************************************************************/
#ifndef _FLUID3DSCENE_H
#define _FLUID3DSCENE_H

#include "../../utilities/AtlInclude.h"

#include <vector>
#include <memory>
#include "IScene.h"

#include "GeometricPrimitive.h"

class Camera;
class D3DGraphicsObject;
class VolumeRenderer;
class InputSystem;

namespace Fluid3D {
	class Fluid3DSimulator;
}

using namespace std;
using namespace DirectX;

class Fluid3DScene : public IScene {
public:
	Fluid3DScene();
	~Fluid3DScene();

	bool Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd);
	void Update(float delta);

	bool Render();

private:
	void UpdateCamera(float delta);
	void HandleInput();

private:
	unique_ptr<Fluid3D::Fluid3DSimulator>	mFluid3DEffect;
	unique_ptr<VolumeRenderer>			mVolumeRenderer;
	unique_ptr<Camera>					mCamera;
	
	unique_ptr<GeometricPrimitive>		mPlane;

	D3DGraphicsObject* pD3dGraphicsObj;

	TwBar *mTwBar;
	InputSystem *pInputSystem;

	bool mPaused;
};

#endif