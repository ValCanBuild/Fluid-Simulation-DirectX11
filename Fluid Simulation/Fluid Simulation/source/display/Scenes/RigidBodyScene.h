/***************************************************************
RigidBodyScene.h: Describes a scene that simulates rigid body
dynamics

Author: Valentin Hinov
Date: 26/11/2013
***************************************************************/
#ifndef _RIGIDBODYSCENE_H
#define _RIGIDBODYSCENE_H

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

class RigidBodyScene : public IScene {
public:
	RigidBodyScene();
	~RigidBodyScene();

	bool Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd);
	void Update(float delta);
	bool Render();

private:
	void UpdateCamera(float delta);

private:
	unique_ptr<Camera>					mCamera;
	
	unique_ptr<GeometricPrimitive>		mBox;
	unique_ptr<GeometricPrimitive>		mPlane;

	D3DGraphicsObject* pD3dGraphicsObj;

	bool mPaused;
	float mAngle;
};

#endif