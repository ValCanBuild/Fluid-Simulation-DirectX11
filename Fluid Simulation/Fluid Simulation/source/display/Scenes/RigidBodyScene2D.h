/***************************************************************
RigidBodyScene2D.h: Describes a scene that simulates 2D rigid body
dynamics

Author: Valentin Hinov
Date: 26/11/2013
***************************************************************/
#ifndef _RigidBodyScene2D_H
#define _RigidBodyScene2D_H

#include <atlbase.h>
#if defined (_DEBUG)
#pragma comment(lib,"atlsd.lib")
#endif

#include <vector>
#include <memory>
#include "IScene.h"

#include "GeometricPrimitive.h"

class Camera;
class D3DGraphicsObject;
class D2DParticle;

struct CTwBar;
typedef CTwBar TwBar;

using namespace std;

namespace DirectX
{
	class SpriteBatch;
}

class RigidBodyScene2D : public IScene {
public:
	RigidBodyScene2D();
	~RigidBodyScene2D();

	bool Initialize(_In_ IGraphicsObject* graphicsObject, HWND hwnd);
	void Update(float delta);
	void FixedUpdate(float fixedDelta);
	bool Render();

private:
	D3DGraphicsObject* pD3dGraphicsObj;

	unique_ptr<DirectX::SpriteBatch> mSpriteBatch;
	
	vector<D2DParticle*> mParticleUnits;
	vector<D2DParticle*> mObstacles;

	bool mPaused;

private:
	TwBar *mTwBar;
};

#endif