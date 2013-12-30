/***************************************************************
RigidBodyScene3D.h: Describes a scene that simulates 3D rigid body
dynamics

Author: Valentin Hinov
Date: 26/11/2013
***************************************************************/
#ifndef _RIGIDBODYSCENE3D_H
#define _RIGIDBODYSCENE3D_H

#include <AntTweakBar.h>

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
class BaseD3DBody;
class CollisionManager;

using namespace std;
using namespace DirectX;

class RigidBodyScene3D : public IScene {
public:
	RigidBodyScene3D();
	~RigidBodyScene3D();

	bool Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd);
	void Update(float delta);
	void FixedUpdate(float fixedDelta);
	bool Render();
	void ObjectPicked(BaseD3DBody *object = nullptr);

private:
	void UpdateCamera(float delta);
	void HandleInput();

// Tweak Bar Methods
private:
	// SETTERS
	static void TW_CALL SetMassCallback(const void* value, void *clientData);
	static void TW_CALL SetLinearDragCallback(const void* value, void *clientData);
	static void TW_CALL SetAngularDragCallback(const void* value, void *clientData);
	
	// GETTERS
	static void TW_CALL GetLinearVelocityCallback(void* value, void *clientData);
	static void TW_CALL GetMassCallback(void* value, void *clientData);
	static void TW_CALL GetLinearSpeedCallback(void* value, void *clientData);
	static void TW_CALL GetLinearDragCallback(void* value, void *clientData);
	static void TW_CALL GetAngularDragCallback(void* value, void *clientData);

private:
	unique_ptr<Camera>					mCamera;
	
	BaseD3DBody*						mBody;
	BaseD3DBody*						mPlane;

	vector<BaseD3DBody*>				mSceneObjects;

	BaseD3DBody*						mPickedObject;

	D3DGraphicsObject* pD3dGraphicsObj;

	unique_ptr<CollisionManager>		mCollisionManager;

	bool mPaused;
	float mAngle;

private:
	TwBar *mTwBar;
};

#endif