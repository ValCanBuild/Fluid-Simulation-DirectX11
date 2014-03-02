/********************************************************************
PrimitiveGameObject.h: A game object class that is a container of
basic components and a Geometric Primitive

Author:	Valentin Hinov
Date: 28/2/2014
*********************************************************************/

#ifndef _PRIMITIVEGAMEOBJECT_H
#define _PRIMITIVEGAMEOBJECT_H

#include "GameObject.h"
#include <GeometricPrimitive.h>

class PrimitiveGameObject : public GameObject {
public:
	PrimitiveGameObject(std::unique_ptr<DirectX::GeometricPrimitive> primitiveModel);
	PrimitiveGameObject(const PrimitiveGameObject &other);
	virtual ~PrimitiveGameObject();

	void Update();
	virtual void Render(const Matrix &viewMatrix, const Matrix &projectionMatrix);

protected:
	std::shared_ptr<DirectX::GeometricPrimitive> primitive;
};

#endif