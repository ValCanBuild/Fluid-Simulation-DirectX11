/***************************************************************
D2DSprite.h: A Direct3D sprite game object

Author: Valentin Hinov
Date: 01/12/2013
**************************************************************/
#ifndef _D2DSPRITE_H
#define _D2DSPRITE_H

#include "GameObject.h"
#include "../display/D3DGraphicsObject.h"
#include <memory>
#include <vector>

class D3DTexture;

namespace DirectX
{
	class SpriteBatch;
}

class D2DSprite : public GameObject{
public:
	D2DSprite(Vector3 &position, float scale = 1.0f);
	D2DSprite(const D2DSprite& other);
	~D2DSprite();

	bool Initialize(D3DGraphicsObject *pGraphicsObj, HWND hwnd, WCHAR *textureFilename);

	void FixedUpdate(float dt);	// update that happens at a fixed time step for proper rigid body simulation
	void Render(DirectX::SpriteBatch *spriteBatch);	//render using the given sprite batch

	void EnforceBoundaries();	

	void SetThrusters(bool p, bool s);
	void ModulateThrust(bool up);

private:
	int			mWidth;
	int			mHeight;

// Physics
private:
	float		mThrustForce;

// Rendering variables
private:
	unique_ptr<D3DTexture> mTexture;
	Color	mColor;
};

#endif