/***************************************************************
IRenderable.h: An interface for an object that knows how to render
itself

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#ifndef _IRENDERABLE_H
#define _IRENDERABLE_H

#include "../display/IGraphicsObject.h"

class IRenderable {
public:
	IRenderable(){}
	virtual ~IRenderable(){}

	virtual bool Initialize(IGraphicsObject* graphicsObject, HWND hwnd) = 0;
	virtual bool Render(const D3DXMATRIX* viewMatrix, const D3DXMATRIX* projMatrix) = 0;
};

#endif