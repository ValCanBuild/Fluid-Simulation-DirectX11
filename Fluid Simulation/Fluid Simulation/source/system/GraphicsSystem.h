/***************************************************************
GraphicsSystem.h: In charge of displaying content on the screen.
Calls Render on the current active Scene and renders the GUI
overlay. 

Author: Valentin Hinov
Date: 02/09/2013
**************************************************************/
#ifndef _GRAPHICSYSTEM_H_
#define _GRAPHICSYSTEM_H_

#include <windows.h>
#include <memory>
#include <string>

#include "../display/IGraphicsObject.h"

class IScene;

#if defined (D3D)
	#include "SpriteFont.h"	
	using namespace DirectX;
#endif

using namespace std;

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;
const int SCREEN_WIDTH_WINDOWED = 800;
const int SCREEN_HEIGHT_WINDOWED = 600;

class GraphicsSystem
{
public:
	GraphicsSystem();
	GraphicsSystem(const GraphicsSystem&);
	~GraphicsSystem();

	bool Initialize(int, int, HWND);
	bool Frame(float delta) const;	// delta time in seconds

	bool TakeScreenshot(LPCWSTR name) const;

	void SetMonitorData(int fps, int cpuUsage);

	bool ResetScene();

private:
	bool Render() const;
	bool RenderOverlay() const;

private:
	HWND mHwnd;

private:
	unique_ptr<IGraphicsObject> mGraphicsObj;
	unique_ptr<IScene> mCurrentScene;

	unique_ptr<SpriteBatch> mSpriteBatch;
	unique_ptr<SpriteFont>  mSpriteFont;

	int	mFps, mCpuUsage;
	wstring	mCardName;
	wstring mVideoMemory;
};

#endif