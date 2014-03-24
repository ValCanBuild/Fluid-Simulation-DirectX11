/********************************************************************
ModelGameObject.h: A game object class that is a container of
basic components and a DirectX::Model object

Author:	Valentin Hinov
Date: 22/3/2014
*********************************************************************/

#ifndef _MODELGAMEOBJECT_H
#define _MODELGAMEOBJECT_H

#include "GameObject.h"
#include <Model.h>

class ICamera;
class D3DGraphicsObject;
namespace DirectX
{
	class CommonStates;
}

class ModelGameObject : public GameObject {
public:
	ModelGameObject();
	ModelGameObject(std::unique_ptr<DirectX::Model> model);
	~ModelGameObject();

	void Update();
	void Render(const ICamera &camera, ID3D11DeviceContext * const deviceContext);

private:
	std::unique_ptr<DirectX::Model> mModel;
	std::shared_ptr<DirectX::CommonStates> pCommonStates;
};

#endif