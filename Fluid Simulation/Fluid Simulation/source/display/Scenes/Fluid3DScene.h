/***************************************************************
Fluid3DScene.h: Describes a scene that displays a 3D fluid
simulation using Direct3D

Author: Valentin Hinov
Date: 24/10/2013
***************************************************************/
#ifndef _FLUID3DSCENE_H
#define _FLUID3DSCENE_H

#include "../../utilities/AtlInclude.h"

#include <vector>
#include <memory>
#include "IScene.h"
#include "../../objects/PrimitiveGameObject.h"

class Camera;
class D3DGraphicsObject;
class InputSystem;
class FluidSimulation;
struct CTwBar;

using namespace std;

class Fluid3DScene : public IScene {
public:
	Fluid3DScene();
	~Fluid3DScene();

	bool Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd);

	void InitCamera();

	void Update(float delta);
	bool Render();
	void RenderOverlay(std::shared_ptr<DirectX::SpriteBatch> spriteBatch, std::shared_ptr<DirectX::SpriteFont> spriteFont);

private:
	bool InitSimulations(HWND hwnd);
	void InitGameObjects();
	void UpdateCamera(float delta) const;
	void HandleInput();
	void HandleMousePicking();

private:
	unique_ptr<Camera>	mCamera;
	
	shared_ptr<FluidSimulation> pPickedSimulation;
	vector<shared_ptr<FluidSimulation>> mSimulations;
	vector<PrimitiveGameObject>	mPrimitiveObjects;

	D3DGraphicsObject* pD3dGraphicsObj;

	CTwBar *mTwBar;
	InputSystem *pInputSystem;

private:
	int  mNumRenderedFluids;
	int  mNumFluidsUpdating;
	bool mPaused;
};

#endif