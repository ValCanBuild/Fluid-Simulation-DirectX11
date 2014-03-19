/********************************************************************
PrimitiveGameObject.cpp: A game object class that is a container of
basic components and a Geometric Primitive

Author:	Valentin Hinov
Date: 28/2/2014
*********************************************************************/

#include "PrimitiveGameObject.h"
#include "../utilities/ICamera.h"

using namespace DirectX;

PrimitiveGameObject::PrimitiveGameObject() {
	transform = std::shared_ptr<Transform>(new Transform(this));
	bounds = std::shared_ptr<Bounds>(new Bounds(this,BOUNDS_TYPE_BOX));
	mColor = Color(Colors::White.f);
}

PrimitiveGameObject::PrimitiveGameObject(std::unique_ptr<GeometricPrimitive> primitiveModel) :
   primitive(std::shared_ptr<GeometricPrimitive>(std::move(primitiveModel)))
{
	transform = std::shared_ptr<Transform>(new Transform(this));
	bounds = std::shared_ptr<Bounds>(new Bounds(this,BOUNDS_TYPE_BOX));
	mColor = Color(Colors::White.f);
}

PrimitiveGameObject::PrimitiveGameObject(const PrimitiveGameObject &other) {
	transform = std::shared_ptr<Transform>(new Transform(this));

	this->transform->position = other.transform->position;
	this->transform->qRotation = other.transform->qRotation;
	this->transform->scale = other.transform->scale;
	this->mColor = other.mColor;
	primitive = other.primitive;

	bounds = std::shared_ptr<Bounds>(new Bounds(this,BOUNDS_TYPE_BOX));
}

PrimitiveGameObject::~PrimitiveGameObject() {

}

void PrimitiveGameObject::Update() {
	bounds->Update();
}

void PrimitiveGameObject::Render(const ICamera &camera) {
	Matrix worldMatrix;
	transform->GetTransformMatrixQuaternion(worldMatrix);
	primitive->Draw(worldMatrix, camera.GetViewMatrix(), camera.GetProjectionMatrix(), mColor);
}

void PrimitiveGameObject::SetColor(Color color) {
	mColor = color;
}
