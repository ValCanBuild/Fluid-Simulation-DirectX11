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

#include "GeometricPrimitive.h"

class Camera;
class D3DGraphicsObject;

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

private:
	unique_ptr<Camera>					mCamera;
	
	unique_ptr<GeometricPrimitive>		mContainmentBox;

	D3DGraphicsObject* pD3dGraphicsObj;

	bool mPaused;
	float mAngle;
};

#endif