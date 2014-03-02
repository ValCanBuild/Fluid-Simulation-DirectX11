/***************************************************************
IScene.h: The interface for a project scene.
Provides methods for updating and rendering the current scene

Author: Valentin Hinov
Date: 10/09/2013
**************************************************************/
#ifndef _ISCENE_H
#define _ISCENE_H

#include "../IGraphicsObject.h"
#include "SpriteFont.h"

class IScene {
public:
	IScene(void) {}
	virtual ~IScene(void) {}

	virtual bool Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd) = 0;
	virtual void Update(float delta) = 0;
	virtual void FixedUpdate(float fixedDelta) {}; // update that happens at fixed time step - use for physics simulation
	virtual bool Render() = 0;
	virtual void RenderOverlay(std::shared_ptr<DirectX::SpriteBatch> spriteBatch, std::shared_ptr<DirectX::SpriteFont> spriteFont) {};
};

#endif