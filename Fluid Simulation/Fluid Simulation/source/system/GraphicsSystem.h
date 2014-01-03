/***************************************************************
GraphicsSystem.h: In charge of displaying content on the screen.
Calls Render on the current active Scene and renders the GUI
overlay. 

Author: Valentin Hinov
Date: 02/09/2013
**************************************************************/
#ifndef _GRAPHICSYSTEM_H_
#define _GRAPHICSYSTEM_H_

#include <string>

#include "IGraphicsSystem.h"
#include "../display/IGraphicsObject.h"

class IScene;

#if defined (D3D)
namespace DirectX 
{
	class SpriteFont;
	class SpriteBatch;
	class CommonStates;
}
#endif

using namespace std;

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;
const int SCREEN_WIDTH_WINDOWED = 800;
const int SCREEN_HEIGHT_WINDOWED = 600;

class GraphicsSystem : public IGraphicsSystem {
public:
	GraphicsSystem();
	GraphicsSystem(const GraphicsSystem&);
	~GraphicsSystem();

	bool Initialize(int, int, HWND);
	bool Frame(float delta) const;	// delta time in seconds
	void FixedFrame(float fixedDelta) const; // fixed delta time to be used for physics simulation

	bool TakeScreenshot(LPCWSTR name) const;

	const IGraphicsObject * const GetGraphicsObject() const;

	#if defined (D3D)
	shared_ptr<DirectX::CommonStates> GetCommonD3DStates() const;
	shared_ptr<DirectX::SpriteFont> GetSpriteFont() const;
	#endif

	void SetMonitorData(int fps, int cpuUsage);

	bool ResetScene();

	IScene * const GetCurrentScene() const;

private:
	bool Render() const;
	bool RenderOverlay() const;

private:
	HWND mHwnd;

private:
	unique_ptr<IGraphicsObject> mGraphicsObj;
	unique_ptr<IScene> mCurrentScene;

	unique_ptr<DirectX::SpriteBatch> mSpriteBatch;

	shared_ptr<DirectX::SpriteFont>  mSpriteFont;
	shared_ptr<DirectX::CommonStates>  mCommonStates;

	int	mFps, mCpuUsage;
	wstring	mCardName;
	wstring mVideoMemory;

	bool mSceneFixedUpdatePaused; // pause physics update for the active scene
	bool mReverseFixedTimestep; // pass a negative timestep value to active scene
};

#endif