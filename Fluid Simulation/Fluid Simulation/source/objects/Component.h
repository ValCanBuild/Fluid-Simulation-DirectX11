/***************************************************************
Component.h: A base component class that each component extends.
A GameObject is a collection of components

Author: Valentin Hinov
Date: 02/12/2013
**************************************************************/

#ifndef _COMPONENT_H
#define _COMPONENT_H

#include <memory>

class GameObject;

class Component {
public:
	Component(const GameObject * const gameObject);
	~Component();

	const GameObject * const GetGameObject() const;

private:
	const GameObject * const gameObject;
};

#endif