/***************************************************************
IScene.h: The interface implementation for a project scene.
Provides methods for updating and rendering the current scene

Author: Valentin Hinov
Date: 10/09/2013
**************************************************************/
#ifndef _ISCENE_H
#define _ISCENE_H

#include "../IGraphicsObject.h"

class IScene {
public:
	IScene(void) {}
	virtual ~IScene(void) {}

	virtual bool Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) = 0;
	virtual void Update(float delta) = 0;
	virtual bool Render() = 0;
};

#endif