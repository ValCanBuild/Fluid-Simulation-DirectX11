/***************************************************************
RigidBodyScene3D.cpp: Describes a scene that simulates rigid body
dynamics

Author: Valentin Hinov
Date: 26/11/2013
***************************************************************/
#include "RigidBodyScene3D.h"

#include "../D3DGraphicsObject.h"
#include "../../utilities/CameraImpl.h"
#include "../../system/ServiceProvider.h"
#include "../../objects/BaseD3DBody.h"
#include "../../utilities/Physics.h"
#include "../../utilities/CollisionManager.h"
#include "../../system/InputSystem.h"

float camMoveFactor = 2.0f;

RigidBodyScene3D::RigidBodyScene3D() :  mTwBar(nullptr), mPickedObject(nullptr), mWireframe(false) {
}

RigidBodyScene3D::~RigidBodyScene3D() {
	pD3dGraphicsObj = nullptr;
	int twResult = TwDeleteBar(mTwBar);
	if (twResult == 0) {
		// deletion failed, Call TwGetLastError to retrieve error
	}
	mTwBar = nullptr;
	mPickedObject = nullptr;
	for (BaseD3DBody *object : mSceneObjects) {
		if (object) {
			delete object;
			object = nullptr;
		}
	}
	mSceneObjects.clear();
}

bool RigidBodyScene3D::Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) {
	pD3dGraphicsObj = dynamic_cast<D3DGraphicsObject*>(graphicsObject);
	float nearVal, farVal;
	graphicsObject->GetScreenDepthInfo(nearVal, farVal);
	int screenWidth, screenHeight;
	graphicsObject->GetScreenDimensions(screenWidth, screenHeight);
	float fieldOfView = (float)PI / 4.0f;
	float screenAspect = (float)screenWidth / (float)screenHeight;

	mCamera = CameraImpl::CreateCameraRH(fieldOfView, screenAspect, nearVal, farVal);
	mCamera->SetPosition(0,5,-12);

	Physics::fGravity = -9.8f;
	Physics::fMaxSimulationTimestep = 0.01f;

	mBody = new BaseD3DBody(GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, true));
	mBody->rigidBody3D->SetMass(5.0f);
	mBody->transform->qRotation = Quaternion::CreateFromAxisAngle(Vector3(0.0f,0.0f,1.0f),0.0f);
	mBody->transform->qRotation *= Quaternion::CreateFromAxisAngle(Vector3(0.0f,1.0f,0.0f),0.0f);
	mBody->transform->qRotation *= Quaternion::CreateFromAxisAngle(Vector3(1.0f,0.0f,0.0f),0.0f);
	mBody->transform->scale = Vector3(3.0f,3.0f,3.0f);
	BaseD3DBody * plane = new BaseD3DBody(GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f));
	plane->transform->position = Vector3(0.0f,-0.5f,0.0f);
	plane->transform->scale = Vector3(70.0f,1.0f,70.0f);
	plane->rigidBody3D->SetImmovable(true);

	mBody->transform->position = Vector3(0.0f,6.0f,0.0f);
	//mBody->transform->scale.y = 0.5f;

	mSceneObjects.push_back(mBody);
	mSceneObjects.push_back(plane);

	bool result;
	for (BaseD3DBody *object : mSceneObjects) {
		result = object->Initialize(pD3dGraphicsObj,hwnd);
		object->FixedUpdate(0.0f);
		if (!result) {
			return false;
		}
	}

	mCollisionManager = unique_ptr<CollisionManager>(new CollisionManager(&mSceneObjects));

	int w,h;
	pD3dGraphicsObj->GetScreenDimensions(w,h);

	// Initialize this scene's tweak bar
	mTwBar = TwNewBar("3D Rigid Bodies");
	// Position bar
	int barPos[2] = {w - 318,2};
	TwSetParam(mTwBar,nullptr,"position", TW_PARAM_INT32, 2, barPos);
	int barSize[2] = {318,230};
	TwSetParam(mTwBar,nullptr,"size", TW_PARAM_INT32, 2, barSize);
	float barRefresh = 0.2f;
	TwSetParam(mTwBar,nullptr,"refresh", TW_PARAM_FLOAT, 1, &barRefresh);

	// Add Variables to tweak bar
	TwAddVarRW(mTwBar,"Timestep", TW_TYPE_FLOAT, &Physics::fMaxSimulationTimestep, "step=0.001");
	TwAddVarRW(mTwBar,"Gravity", TW_TYPE_FLOAT, &Physics::fGravity, "step=0.1");
	TwAddVarRW(mTwBar,"Friction", TW_TYPE_FLOAT, &Physics::fRestitution, "step=0.05");
	TwAddVarRW(mTwBar,"Solver Iterations", TW_TYPE_INT16, &Physics::iSolverIterationCount, "step=1");
	TwAddVarRW(mTwBar,"Allowed Penetration", TW_TYPE_FLOAT, &Physics::fAllowedPenetration, "step=0.001");
	TwAddVarRW(mTwBar,"Contact Bias Factor", TW_TYPE_FLOAT, &Physics::fContactBiasFactor, "step=0.001");
	TwAddVarRW(mTwBar,"Wireframe", TW_TYPE_BOOLCPP, &mWireframe, nullptr);
	TwAddSeparator(mTwBar, " ", nullptr);
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
		object->rigidBody3D->ApplyGravity(fixedDelta);
	}

	// collision detection must happen after rigid body updates
	mCollisionManager->PerformCollisionCheck();

	mCollisionManager->PerformCollisionResponse(fixedDelta);

	for (BaseD3DBody *object : mSceneObjects) {
		object->FixedUpdate(fixedDelta);
	}
}

bool RigidBodyScene3D::Render() {
	Matrix viewMatrix, projectionMatrix;
	mCamera->GetProjectionMatrix(projectionMatrix);
	mCamera->GetViewMatrix(viewMatrix);

	for (BaseD3DBody *object : mSceneObjects) {
		object->Render(&viewMatrix,&projectionMatrix, mWireframe);
	}

	mCollisionManager->DebugRender(&viewMatrix,&projectionMatrix);

	return true;
}

void RigidBodyScene3D::HandleInput() {
	auto inputSystem = ServiceProvider::Instance().GetService<InputSystem>();
	if (inputSystem->IsKeyDown(VK_UP)) {
		mBody->rigidBody3D->AddTorque(Vector3(2.0f,2.0f,0.0f));
	}
	else if (inputSystem->IsKeyDown(VK_DOWN)) {
		mBody->rigidBody3D->AddTorque(Vector3(-2.0f,-2.0f,0.0f));
	}
	int scrollDelta = 0;
	inputSystem->GetMouseScrollDelta(scrollDelta);
	if (scrollDelta > 0) {
		camMoveFactor += scrollDelta*0.005f;
	}

	// Mouse picking
	if (inputSystem->IsMouseLeftClicked()) {
		// ray cast and pick object
		int posX,posY;
		inputSystem->GetMousePos(posX,posY);
		Ray ray = mCamera->ScreenPointToRay(Vector2((float)posX,(float)posY));
		BaseD3DBody *objectToPick = nullptr;
		float prevMinDistance = 1000000.0f;
		for (BaseD3DBody *object : mSceneObjects) {
			// check if object has a bounds object
			if (object->bounds != nullptr && object->rigidBody3D != nullptr) {
				const BoundingBox *boundingBox = object->bounds->GetBoundingBox();
				float distanceToObject = 0.0f;
				if (ray.Intersects(*boundingBox,distanceToObject)) {
					// choose picked object and display info
					if (distanceToObject < prevMinDistance) {
						prevMinDistance = distanceToObject;
						objectToPick = object;
					}
				}
			}
		}
		ObjectPicked(objectToPick);
	}

	// Spawn a cube that travels forward
	else if ( inputSystem->IsMouseMidClicked() || (inputSystem->IsMouseLeftDown() && inputSystem->IsKeyClicked(VK_CONTROL)) ) {
		int posX,posY;
		inputSystem->GetMousePos(posX,posY);
		Ray ray = mCamera->ScreenPointToRay(Vector2((float)posX,(float)posY));
		// create object and launch it
		BaseD3DBody* body = new BaseD3DBody(GeometricPrimitive::CreateCube(pD3dGraphicsObj->GetDeviceContext(), 1.0f, true));
		mCamera->GetPosition(body->transform->position);
		body->transform->scale = Vector3(2.0f);
		body->transform->qRotation = Quaternion(Vector3(RandF(),RandF(),RandF()),RandF());
		body->rigidBody3D->SetMass(1.0f);
		body->FixedUpdate(0.0f);
		body->Initialize(pD3dGraphicsObj,nullptr);
		body->rigidBody3D->AddForce(ray.direction*20, true);
		mSceneObjects.push_back(body);
	}
}

void RigidBodyScene3D::UpdateCamera(float delta) {
	auto inputSystem = ServiceProvider::Instance().GetService<InputSystem>();

	// Move camera with WASD 
	float forwardAmount = 0.0f;
	float rightAmount = 0.0f;

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
		mCamera->MoveFacing(forwardAmount*camMoveFactor,rightAmount*camMoveFactor);
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
		TwRemoveVar(mTwBar, "Linear Damping");
		TwRemoveVar(mTwBar, "Angular Damping");
		TwRemoveVar(mTwBar, "Linear Velocity");
		TwRemoveVar(mTwBar, "Angular Velocity");
		TwRemoveVar(mTwBar, "Orientation");
		TwRemoveVar(mTwBar, "Scale");
		TwRemoveVar(mTwBar, "Inertia Tensor");
		TwRemoveVar(mTwBar, "Immovable");
	}
	else if (mPickedObject == object) {
		return;
	}

	mPickedObject = object;

	if (mPickedObject != nullptr) {
		TwAddVarCB(mTwBar, "Mass", TW_TYPE_FLOAT, SetMassCallback, GetMassCallback, mPickedObject->rigidBody3D.get(), "step=0.1");
		TwAddVarCB(mTwBar, "Linear Damping", TW_TYPE_FLOAT, SetLinearDragCallback, GetLinearDragCallback, mPickedObject->rigidBody3D.get(), "step=0.01 max=1.0 min=0.0");
		TwAddVarCB(mTwBar, "Angular Damping", TW_TYPE_FLOAT, SetAngularDragCallback, GetAngularDragCallback, mPickedObject->rigidBody3D.get(), "step=0.01 max=1.0 min=0.0");
		TwAddVarCB(mTwBar, "Orientation", TW_TYPE_QUAT4F, SetOrientationCallback, GetOrientationCallback, mPickedObject->transform.get(), "");
		TwAddVarCB(mTwBar, "Scale", TW_TYPE_DIR3F, SetScaleCallback, GetScaleCallback, mPickedObject->transform.get(), "");
		TwAddVarCB(mTwBar, "Linear Velocity", TW_TYPE_DIR3F, SetLinearVelocityCallback, GetLinearVelocityCallback, mPickedObject->rigidBody3D.get(), "");
		TwAddVarCB(mTwBar, "Angular Velocity", TW_TYPE_DIR3F, SetAngularVelocityCallback, GetAngularVelocityCallback, mPickedObject->rigidBody3D.get(), "");
		TwAddVarCB(mTwBar, "Inertia Tensor", TW_TYPE_DIR3F, nullptr, GetInertiaTensorCallback, mPickedObject->rigidBody3D.get(), "");
		TwAddVarCB(mTwBar, "Immovable", TW_TYPE_BOOLCPP, SetImmovableCallback, GetImmovableCallback, mPickedObject->rigidBody3D.get(), "");
	}
}

///TWEAK BAR METHODS
// SETTERS
void TW_CALL RigidBodyScene3D::SetLinearVelocityCallback(const void* value, void *clientData) {
	Vector3 linVel = *static_cast<const Vector3 *>(value);
	static_cast<RigidBody3D *>(clientData)->SetLinearVelocity(linVel);
}

void TW_CALL RigidBodyScene3D::SetAngularVelocityCallback(const void* value, void *clientData) {
	Vector3 angVel = *static_cast<const Vector3 *>(value);
	static_cast<RigidBody3D *>(clientData)->SetAngularVelocity(angVel);
}

void TW_CALL RigidBodyScene3D::SetMassCallback(const void* value, void *clientData) {
	static_cast<RigidBody3D *>(clientData)->SetMass(*static_cast<const float *>(value));
}

void TW_CALL RigidBodyScene3D::SetLinearDragCallback(const void* value, void *clientData) {
	static_cast<RigidBody3D *>(clientData)->SetLinearDamping(*static_cast<const float *>(value));
}

void TW_CALL RigidBodyScene3D::SetAngularDragCallback(const void* value, void *clientData) {
	static_cast<RigidBody3D *>(clientData)->SetAngularDamping(*static_cast<const float *>(value));
}

void TW_CALL RigidBodyScene3D::SetOrientationCallback(const void* value, void *clientData) {
	Transform * bodyTransform = static_cast<Transform *>(clientData);
	const Quaternion orientation = *static_cast<const Quaternion *>(value);
	bodyTransform->qRotation = orientation;
}

void TW_CALL RigidBodyScene3D::SetScaleCallback(const void* value, void *clientData) {
	Transform * bodyTransform = static_cast<Transform *>(clientData);
	const Vector3 scale = *static_cast<const Vector3 *>(value);
	bodyTransform->scale = scale;
}

void TW_CALL RigidBodyScene3D::SetImmovableCallback(const void* value, void *clientData) {
	static_cast<RigidBody3D *>(clientData)->SetImmovable(*static_cast<const bool *>(value));
}

// GETTERS	 
void TW_CALL RigidBodyScene3D::GetLinearVelocityCallback(void* value, void *clientData) {
	static_cast<const RigidBody3D *>(clientData)->GetVelocity(*static_cast<Vector3 *>(value));
}

void TW_CALL RigidBodyScene3D::GetAngularVelocityCallback(void* value, void *clientData) {
	static_cast<const RigidBody3D *>(clientData)->GetAngularVelocity(*static_cast<Vector3 *>(value));
}

void TW_CALL RigidBodyScene3D::GetMassCallback(void* value, void *clientData) {
	*static_cast<float *>(value) = static_cast<const RigidBody3D *>(clientData)->GetMass();
}

void TW_CALL RigidBodyScene3D::GetLinearSpeedCallback(void* value, void *clientData) {
	*static_cast<float *>(value) = static_cast<const RigidBody3D *>(clientData)->GetSpeed();
}

void TW_CALL RigidBodyScene3D::GetLinearDragCallback(void* value, void *clientData) {
	*static_cast<float *>(value) = static_cast<const RigidBody3D *>(clientData)->GetLinearDamping();
}

void TW_CALL RigidBodyScene3D::GetAngularDragCallback(void* value, void *clientData) {
	*static_cast<float *>(value) = static_cast<const RigidBody3D *>(clientData)->GetAngularDamping();
}

void TW_CALL RigidBodyScene3D::GetOrientationCallback(void* value, void *clientData) {
	Transform * bodyTransform = static_cast<Transform *>(clientData);
	*static_cast<Quaternion *>(value) = bodyTransform->qRotation;
}

void TW_CALL RigidBodyScene3D::GetScaleCallback(void* value, void *clientData) {
	Transform * bodyTransform = static_cast<Transform *>(clientData);
	*static_cast<Vector3 *>(value) = bodyTransform->scale;
}

void TW_CALL RigidBodyScene3D::GetInertiaTensorCallback(void* value, void *clientData) {
	*static_cast<Vector3 *>(value) = static_cast<const RigidBody3D *>(clientData)->GetInertiaTensor();
}

void TW_CALL RigidBodyScene3D::GetImmovableCallback(void* value, void *clientData) {
	*static_cast<bool *>(value) = static_cast<const RigidBody3D *>(clientData)->GetIsImmovable();
}