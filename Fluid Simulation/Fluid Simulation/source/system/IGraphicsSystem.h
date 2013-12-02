/******************************************************************
IGraphicsSystem.h: Interface to the graphics system that is to be
globally available. Provides access to global states and functions

Author: Valentin Hinov
Date: 01/12/2013
******************************************************************/
#ifndef _IGRAPHICSSYSTEM_H
#define _IGRAPHICSSYSTEM_H

#include <windows.h>
#include <memory>

#if defined (D3D)
namespace DirectX 
{
	class CommonStates;
	class SpriteFont;
}
#endif

class IGraphicsSystem {
public:
	virtual bool TakeScreenshot(LPCWSTR name) const = 0;

	#if defined (D3D)
	virtual std::shared_ptr<DirectX::CommonStates> GetCommonD3DStates() const = 0;
	virtual std::shared_ptr<DirectX::SpriteFont> GetSpriteFont() const = 0;
	#endif

};

#endif