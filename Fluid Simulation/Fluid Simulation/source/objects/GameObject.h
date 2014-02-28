/***************************************************************
GameObject.h: A base game object class that is a container of
basic components

Author: Valentin Hinov
Date: 02/12/2013
**************************************************************/

#ifndef _GAMEOBJECT_H
#define _GAMEOBJECT_H

#include <memory>

#include "Transform.h"
#include "Bounds.h"

class GameObject {
public:
	std::shared_ptr<Transform>		transform;
	std::shared_ptr<Bounds>			bounds;

	GameObject() {
	}

	virtual ~GameObject() {
	}
};

#endif