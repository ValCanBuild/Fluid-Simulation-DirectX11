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

class ICamera;

class PrimitiveGameObject : public GameObject {
public:
	PrimitiveGameObject();
	PrimitiveGameObject(std::unique_ptr<DirectX::GeometricPrimitive> primitiveModel);
	PrimitiveGameObject(const PrimitiveGameObject &other);
	virtual ~PrimitiveGameObject();

	void Update();
	// color only used if using PrimitiveGameObject::Render
	void SetColor(Color color);
	virtual void Render(const ICamera &camera);

protected:
	std::shared_ptr<DirectX::GeometricPrimitive> primitive;

private:
	Color mColor;
};

#endif