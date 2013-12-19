/***************************************************************
RigidBodyScene3D.cpp: Describes a scene that simulates rigid body
dynamics

Author: Valentin Hinov
Date: 26/11/2013
***************************************************************/
#include "RigidBodyScene3D.h"

#include "../D3DGraphicsObject.h"
#include "../../utilities/Camera.h"
#include "../../system/ServiceProvider.h"
#include "../../objects/BaseD3DBody.h"
#include "../../utilities/Physics.h"
#include "../../utilities/CollisionManager.h"

RigidBodyScene3D::RigidBodyScene3D() : mAngle(0.0f), mTwBar(nullptr), mPickedObject(nullptr) {
}

RigidBodyScene3D::~RigidBodyScene3D() {
	pD3dGraphicsObj = nullptr;
	int twResult = TwDeleteBar(mTwBar);
	if (twResult == 0) {
		// deletion failed, Call TwGetLastError to retrieve error
	}
	mTwBar = nullptr;
	mPickedObject = nullptr;
	mSceneObjects.clear();
}

bool RigidBodyScene3D::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	mCamera = unique_ptr<Camera>(new Camera());	
	mCamera->SetPosition(0,2,-10);

	mBody = unique_ptr<BaseD3DBody>(new BaseD3DBody(GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, true)));
	mPlane = unique_ptr<BaseD3DBody>(new BaseD3DBody(GeometricPrimitive::CreatePlane(pD3dGraphicsObj->GetDeviceContext(), 100.0f, 1, true),false));
	mPlane->bounds->UpdateExtents(Vector3(50.0f,0.0f,50.0f));
	mPlane->boxCollider->UpdateExtents(Vector3(50.0f,0.0f,50.0f));
	
	mBody->transform->position = Vector3(0.0f,8.0f,0.0f);
	//mBody->transform->scale.y = 0.5f;

	mSceneObjects.push_back(mBody.get());
	mSceneObjects.push_back(mPlane.get());

	bool result;
	for (BaseD3DBody *object : mSceneObjects) {
		result = object->Initialize(pD3dGraphicsObj,hwnd);
		if (!result) {
			return false;
		}
	}

	mCollisionManager = unique_ptr<CollisionManager>(new CollisionManager(&mSceneObjects));

	// Initialize this scene's tweak bar
	mTwBar = TwNewBar("3D Rigid Bodies");
	// Position bar
	int barPos[2] = {580,2};
	TwSetParam(mTwBar,nullptr,"position", TW_PARAM_INT32, 2, barPos);
	int barSize[2] = {218,150};
	TwSetParam(mTwBar,nullptr,"size", TW_PARAM_INT32, 2, barSize);

	// Add Variables to tweak bar
	TwAddVarRW(mTwBar,"Timestep", TW_TYPE_FLOAT, &Physics::fMaxSimulationTimestep, "step=0.001");
	TwAddVarRW(mTwBar,"Gravity", TW_TYPE_FLOAT, &Physics::fGravity, "step=0.1");

	return true;
}

void RigidBodyScene3D::Update(float delta) {
	UpdateCamera(delta);
	HandleInput();
	for (BaseD3DBody *object : mSceneObjects) {
		object->Update(delta);
	}
}

void RigidBodyScene3D::FixedUpdate(float fixedDelta) {
	for (BaseD3DBody *object : mSceneObjects) {
		object->FixedUpdate(fixedDelta);
	}
	mCollisionManager->FixedUpdate(fixedDelta);
}

bool RigidBodyScene3D::Render() {
	Matrix viewMatrix, projectionMatrix, worldMatrix;
	pD3dGraphicsObj->GetProjectionMatrix(projectionMatrix);
	mCamera->GetViewMatrix(viewMatrix);
	pD3dGraphicsObj->GetWorldMatrix(worldMatrix);

	mPlane->Render(&viewMatrix,&projectionMatrix);

	mBody->Render(&viewMatrix,&projectionMatrix);

	return true;
}

void RigidBodyScene3D::HandleInput() {
	I_InputSystem *inputSystem = ServiceProvider::Instance().GetInputSystem();
	if (inputSystem->IsKeyDown(VK_UP)) {
		mBody->rigidBody3D->AddTorque(Vector3(2.0f,2.0f,0.0f));
	}
	else if (inputSystem->IsKeyDown(VK_DOWN)) {
		mBody->rigidBody3D->AddTorque(Vector3(-2.0f,-2.0f,0.0f));
	}

	if (inputSystem->IsMouseLeftClicked()) {
		// ray cast and pick object
		int posX,posY;
		inputSystem->GetMousePos(posX,posY);
		Ray ray = mCamera->ScreenPointToRay(Vector2((float)posX,(float)posY));
		BaseD3DBody *objectToPick = nullptr;
		for (BaseD3DBody *object : mSceneObjects) {
			// check if object has a bounds object
			if (object->bounds != nullptr && object->rigidBody3D != nullptr) {
				const BoundingBox *boundingBox = object->bounds->GetBoundingBox();
				float distanceToObject = 0.0f;
				if (ray.Intersects(*boundingBox,distanceToObject)) {
					// choose picked object and display info
					objectToPick = object;
					break;
				}
			}
		}
		ObjectPicked(objectToPick);
	}
}

void RigidBodyScene3D::UpdateCamera(float delta) {
	I_InputSystem *inputSystem = ServiceProvider::Instance().GetInputSystem();

	// Move camera with WASD 
	float forwardAmount = 0.0f;
	float rightAmount = 0.0f;
	const float moveFactor = 2.0f;

	if (inputSystem->IsKeyDown('W')) {
		forwardAmount += delta;
	}
	else if (inputSystem->IsKeyDown('S')) {
		forwardAmount -= delta;
	}
	if (inputSystem->IsKeyDown('A')) {
		rightAmount += delta;
	}
	else if (inputSystem->IsKeyDown('D')) {
		rightAmount -= delta;
	}

	if (forwardAmount != 0.0f || rightAmount != 0.0f) {
		mCamera->MoveFacing(forwardAmount*moveFactor,rightAmount*moveFactor);
	}

	// Rotate camera with mouse button
	if (inputSystem->IsMouseRightDown()) {
		int xDelta,yDelta;
		float mouseSensitivity = 0.003f;
		inputSystem->GetMouseDelta(xDelta,yDelta);
		mCamera->AddYawPitchRoll(-xDelta*mouseSensitivity,yDelta*mouseSensitivity,0.0f);
	}

	mCamera->Update();
}

// Pick object - if nullptr passed, any active objects will be unpicked
void RigidBodyScene3D::ObjectPicked(BaseD3DBody *object) {
	// if unpicking object or picking a new one - remove var's
	if (mPickedObject != object) {
		TwRemoveVar(mTwBar, "Mass");
		TwRemoveVar(mTwBar, "Linear Drag");
		TwRemoveVar(mTwBar, "Angular Drag");
		TwRemoveVar(mTwBar, "Linear Speed");
		TwRemoveVar(mTwBar, "Linear Velocity");
	}
	else if (mPickedObject == object) {
		return;
	}

	mPickedObject = object;

	if (mPickedObject != nullptr) {
		TwAddVarCB(mTwBar, "Mass", TW_TYPE_FLOAT, SetMassCallback, GetMassCallback, mPickedObject->rigidBody3D.get(), "step=0.1");
		TwAddVarCB(mTwBar, "Linear Drag", TW_TYPE_FLOAT, SetLinearDragCallback, GetLinearDragCallback, mPickedObject->rigidBody3D.get(), "step=0.01");
		TwAddVarCB(mTwBar, "Angular Drag", TW_TYPE_FLOAT, SetAngularDragCallback, GetAngularDragCallback, mPickedObject->rigidBody3D.get(), "step=0.01");
		TwAddVarCB(mTwBar, "Linear Speed", TW_TYPE_FLOAT, nullptr, GetLinearSpeedCallback, mPickedObject->rigidBody3D.get(), "");
		TwAddVarCB(mTwBar, "Linear Velocity", TW_TYPE_DIR3F, nullptr, GetLinearVelocityCallback, mPickedObject->rigidBody3D.get(), "");
	}
}

///TWEAK BAR METHODS
// SETTERS
void TW_CALL RigidBodyScene3D::SetMassCallback(const void* value, void *clientData) {
	static_cast<RigidBody3D *>(clientData)->SetMass(*static_cast<const float *>(value));
}

void TW_CALL RigidBodyScene3D::SetLinearDragCallback(const void* value, void *clientData) {
	static_cast<RigidBody3D *>(clientData)->SetLinearDrag(*static_cast<const float *>(value));
}

void TW_CALL RigidBodyScene3D::SetAngularDragCallback(const void* value, void *clientData) {
	static_cast<RigidBody3D *>(clientData)->SetAngularDrag(*static_cast<const float *>(value));
}
			 
// GETTERS	 
void TW_CALL RigidBodyScene3D::GetLinearVelocityCallback(void* value, void *clientData) {
	static_cast<const RigidBody3D *>(clientData)->GetVelocity(*static_cast<Vector3 *>(value));
}

void TW_CALL RigidBodyScene3D::GetMassCallback(void* value, void *clientData) {
	*static_cast<float *>(value) = static_cast<const RigidBody3D *>(clientData)->GetMass();
}

void TW_CALL RigidBodyScene3D::GetLinearSpeedCallback(void* value, void *clientData) {
	*static_cast<float *>(value) = static_cast<const RigidBody3D *>(clientData)->GetSpeed();
}

void TW_CALL RigidBodyScene3D::GetLinearDragCallback(void* value, void *clientData) {
	*static_cast<float *>(value) = static_cast<const RigidBody3D *>(clientData)->GetLinearDrag();
}

void TW_CALL RigidBodyScene3D::GetAngularDragCallback(void* value, void *clientData) {
	*static_cast<float *>(value) = static_cast<const RigidBody3D *>(clientData)->GetAngularDrag();
}