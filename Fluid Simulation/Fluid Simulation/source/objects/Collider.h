/*************************************************************
Collider Class: Provides a collider component for objects to use.

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#ifndef _COLLIDER_H
#define _COLLIDER_H

#include "Component.h"
#include "Bounds.h"
#include "RigidBody3D.h"

class BaseD3DBody;

class Collider : public Component{
public:
	Collider(const BaseD3DBody * const d3dBody);
	~Collider();

	std::shared_ptr<RigidBody3D> GetAttachedRigidBody() const;

	void SetEnabled(bool enabled);

	bool IsEnabled();

private:
	bool mEnabled;
};

#endif