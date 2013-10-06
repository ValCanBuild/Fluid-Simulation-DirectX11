/*************************************************************
IFrameBuffer.h: Abstract class that encapsulates a technology
agnostic frame buffer object that can be set as the
current render target. Provides methods for retrieving the
current render texture.

Author: Valentin Hinov
Date: 03/09/2013
Version: 1.0
**************************************************************/

#ifndef _IFRAMEBUFFER_H
#define _IFRAMEBUFFER_H

#include "IGraphicsObject.h"

class IFrameBuffer {
public:
	IFrameBuffer(){}
	virtual ~IFrameBuffer(){}

	virtual bool Initialize(IGraphicsObject* graphicsObject, int width, int height) = 0;

	virtual void BeginRender(float clearRed, float clearGreen, float clearBlue, float clearAlpha) const = 0;
	virtual void EndRender() const = 0;
	
	// Return a void pointer to the underlying texture resource - child class
	virtual const void* GetTextureResource() const = 0;
};

#endif