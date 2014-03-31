/********************************************************************
ModelGameObject.cpp: A game object class that is a container of
basic components and a DirectX::Model object

Author:	Valentin Hinov
Date: 22/3/2014
*********************************************************************/

#include "ModelGameObject.h"
#include <CommonStates.h>
#include "../utilities/ICamera.h"
#include "../display/D3DGraphicsObject.h"
#include "../system/ServiceProvider.h"

using namespace DirectX;

ModelGameObject::ModelGameObject() {
	transform = std::shared_ptr<Transform>(new Transform(this));
	bounds = std::shared_ptr<Bounds>(new Bounds(this,BOUNDS_TYPE_BOX));
	pCommonStates = ServiceProvider::Instance().GetGraphicsSystem()->GetCommonD3DStates();
}

ModelGameObject::ModelGameObject(std::shared_ptr<Model> model) :
   mModel(model)
{
	transform = std::shared_ptr<Transform>(new Transform(this));
	bounds = std::shared_ptr<Bounds>(new Bounds(this,BOUNDS_TYPE_BOX));
	pCommonStates = ServiceProvider::Instance().GetGraphicsSystem()->GetCommonD3DStates();
}

ModelGameObject::~ModelGameObject() {

}

void ModelGameObject::Update() {
	bounds->Update();
}

void ModelGameObject::Render(const ICamera &camera, ID3D11DeviceContext * const deviceContext) {
	Matrix worldMatrix;
	transform->GetTransformMatrixQuaternion(worldMatrix);
	mModel->Draw(deviceContext, *pCommonStates, worldMatrix, camera.GetViewMatrix(), camera.GetProjectionMatrix());
}
