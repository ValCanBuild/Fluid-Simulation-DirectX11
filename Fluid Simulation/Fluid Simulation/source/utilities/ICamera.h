/***************************************************************
ICamera.h: Camera interface class

Author: Valentin Hinov
Date: 11/03/2014
**************************************************************/

#ifndef _ICAMERA_H
#define _ICAMERA_H

#include "../utilities/D3dIncludes.h"

class ICamera {
public:
	virtual void GetPosition(Vector3& pos) const = 0;
	virtual void GetTarget(Vector3& target) const = 0;
	
	virtual const Matrix &GetProjectionMatrix() const = 0;
	virtual const Matrix &GetViewMatrix() const = 0;
	virtual const Matrix &GetViewProjectionMatrix() const = 0;
	virtual float GetFieldOfView() const = 0;
	virtual void GetProjectionMatrix(Matrix& projMatrix) const = 0;
	virtual void GetViewMatrix(Matrix& viewMatrix) const = 0;
	virtual void GetViewProjectionMatrix(Matrix& viewProjMatrix) const = 0;
	virtual void GetRotationMatrix(Matrix& rotationMatrix) const = 0;

	virtual Ray ScreenPointToRay(Vector2 position) const = 0;

	// works only for Left handed coordinate systems
	virtual const DirectX::BoundingFrustum &GetBoundingFrustum() const = 0;	
};

#endif