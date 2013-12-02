/***************************************************************
D2DParticle.h: A Direct3D sprite renderable that adheres to
rigid body physics

Author: Valentin Hinov
Date: 01/12/2013
**************************************************************/
#ifndef _D2DPARTICLE_H
#define _D2DPARTICLE_H

#include "GameObject.h"
#include "../display/D3DGraphicsObject.h"
#include <memory>
#include <vector>

class D3DTexture;

namespace DirectX
{
	class SpriteBatch;
}

class D2DParticle : public GameObject{
public:
	D2DParticle(float mass = 1.0f, Vector3 &position = Vector3(0,0,0), float scale = 0.5f);
	D2DParticle(const D2DParticle& other);
	~D2DParticle();

	bool Initialize(D3DGraphicsObject *pGraphicsObj, HWND hwnd, WCHAR *textureFilename);

	void FixedUpdate(float dt, vector<D2DParticle*> &obstacles);	// update that happens at a fixed time step for proper rigid body simulation
	void Render(DirectX::SpriteBatch *spriteBatch);	//render using the given sprite batch

	void EnforceBoundaries();	
private:
	int			mWidth;
	int			mHeight;

// Rendering variables
private:
	unique_ptr<D3DTexture> mTexture;
	Color	mColor;
};

#endif