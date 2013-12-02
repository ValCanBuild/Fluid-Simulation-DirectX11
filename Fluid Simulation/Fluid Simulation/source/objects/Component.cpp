/***************************************************************
Component.cpp: A base component class that each component extends.
A GameObject is a collection of components

Author: Valentin Hinov
Date: 02/12/2013
**************************************************************/

#include "Component.h"

#include "GameObject.h"

Component::Component(const GameObject * const gameObject) : gameObject(gameObject) {

}

Component::~Component() {
}

const GameObject * const Component::GetGameObject() const {
	return gameObject;
}