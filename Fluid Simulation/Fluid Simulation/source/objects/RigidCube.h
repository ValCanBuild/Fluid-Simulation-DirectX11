/***************************************************************
RigidCube.h: Defines a cube shaped rigid body

Author: Valentin Hinov
Date: 26/11/2013
****************************************************************/
#ifndef _RIGIDCUBE_H
#define _RIGIDCUBE_H

#include "Transform.h"


class RigidCube
{
public:
	RigidCube();
	RigidCube(Transform &transform);
	RigidCube(Vector3 &position);

	~RigidCube();

private:
	Vector3 mCenter;

	Transform mTransform;
};

#endif