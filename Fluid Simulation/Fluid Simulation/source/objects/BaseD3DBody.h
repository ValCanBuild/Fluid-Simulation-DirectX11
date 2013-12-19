/**************************************************************
BaseD3DBody.h: A class to be used a base for any Direct3D body

Author: Valentin Hinov
Date: 05/12/2013
**************************************************************/

#ifndef _BASED3DBODY_H
#define _BASED3DBODY_H

#include "GameObject.h"
#include "RigidBody3D.h"
#include "BoxCollider.h"
#include "GeometricPrimitive.h"

class D3DTexture;
class D3DGraphicsObject;

class BaseD3DBody : public GameObject {
// Public methods
public:
	BaseD3DBody(std::unique_ptr<DirectX::GeometricPrimitive> model, bool hasRigidBody = true, bool hasCollider = true);
	~BaseD3DBody();

	bool Initialize(D3DGraphicsObject * pGraphicsObj, HWND hwnd);
	void Update(float dt);
	void FixedUpdate(float fixedDeltaTime);
	void Render(const Matrix* viewMatrix, const Matrix* projMatrix);

// Public variables
public:
	std::shared_ptr<RigidBody3D> rigidBody3D; 
	std::shared_ptr<BoxCollider> boxCollider; 

// Private Variables
private:
	std::unique_ptr<DirectX::GeometricPrimitive>	mModel;

	Vector3		mPrevPosition;
};

#endif