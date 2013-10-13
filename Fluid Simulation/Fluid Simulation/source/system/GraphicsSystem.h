/***************************************************************
GraphicsSystem.h: In charge of displaying all content on the
screen.

Author: Valentin Hinov
Date: 02/09/2013
**************************************************************/
#ifndef _GRAPHICSYSTEM_H_
#define _GRAPHICSYSTEM_H_

#include <windows.h>
#include <memory>

#include "../display/IGraphicsObject.h"

class IScene;

using namespace std;

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsSystem
////////////////////////////////////////////////////////////////////////////////
class GraphicsSystem
{
public:
	GraphicsSystem();
	GraphicsSystem(const GraphicsSystem&);
	~GraphicsSystem();

	bool Initialize(int, int, HWND);
	bool Frame(float delta);	// delta time in seconds

	bool TakeScreenshot(LPCWSTR name) const;

private:
	bool Render();

private:
	unique_ptr<IGraphicsObject> mGraphicsObj;
	unique_ptr<IScene> mCurrentScene;
};

#endif