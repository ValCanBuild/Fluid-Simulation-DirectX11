/***************************************************************
D2DSprite.h: A Direct3D sprite game object

Author: Valentin Hinov
Date: 01/12/2013
**************************************************************/
#include <string>

#include "D2DSprite.h"

#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "../utilities/Physics.h"
#include "../utilities/D3DTexture.h"
#include "../utilities/Screen.h"
#include "../system/ServiceProvider.h"
#include "../utilities/math/MathUtils.h"

#define DTHRUST 1.0f
#define MAXTHRUST 2000.0f
#define MINTHRUST 0.0f
#define STEERINGFORCE 3.0f

using namespace DirectX;

#pragma warning( disable : 4244)	// ignore implicit conversion warning

D2DSprite::D2DSprite(Vector3 &position, float scale) :
	 mWidth(0), mHeight(0), mThrustForce(0.0f)
{
	transform = shared_ptr<Transform>(new Transform(this, position,Vector3(0,0,0),Vector3(scale)));	
	rigidBody2D = shared_ptr<RigidBody2D>(new RigidBody2D(this));
	rigidBody2D->mRadius = scale;

	rigidBody2D->mMass = 100.0f;
	rigidBody2D->SetIntertia(500.0f);

	mColor = RandomColor();
}

D2DSprite::~D2DSprite() {
}

bool D2DSprite::Initialize(D3DGraphicsObject *pGraphicsObj, HWND hwnd, WCHAR *textureFilename) {
	mTexture = unique_ptr<D3DTexture>(new D3DTexture());
	bool result = mTexture->Initialize(pGraphicsObj->GetDevice(),pGraphicsObj->GetDeviceContext(),textureFilename,hwnd);
	
	if (result) {
		mTexture->GetTextureSize(mWidth,mHeight);
		mWidth *= transform->scale.x;
		mHeight *= transform->scale.y;
		
		rigidBody2D->mWidth = mHeight;
		rigidBody2D->mLength = mWidth;
		rigidBody2D->mHeight = 5.0f;

		rigidBody2D->mDragCenter = Vector3(-0.25f*rigidBody2D->mLength,0.0f,0.0f);

		rigidBody2D->mThrustCenter = Vector3(-0.5f*rigidBody2D->mLength,0.0f,0.0f);

		rigidBody2D->mPortBowThruster = Vector3(0.5f*rigidBody2D->mLength,-0.5f*rigidBody2D->mWidth,0.0f);

		rigidBody2D->mStarboardBowThruster = Vector3(0.5f*rigidBody2D->mLength,0.5f*rigidBody2D->mWidth,0.0f);

		rigidBody2D->mProjectedArea = (rigidBody2D->mLength + rigidBody2D->mWidth)*0.5f * rigidBody2D->mHeight; // an approximation


		Vector3 center = Vector3(transform->position.x + mWidth*0.5f, transform->position.y + mHeight*0.5f,0.0f);
		bounds = shared_ptr<Bounds>(new Bounds(this,BOUNDS_TYPE_BOX,center,Vector3(mWidth*0.5f,mHeight*0.5f,0.0f)));
	}

	return result;
}

void D2DSprite::FixedUpdate(float dt) {
	if (mThrustForce > 0.0f) {
		rigidBody2D->AddForce(Vector3(1.0f,0.0f,0.0f)*mThrustForce);
	}
	if (rigidBody2D->mPThrust.y > 0.0f) {
		float generatedTorque = rigidBody2D->mPortBowThruster.Cross(rigidBody2D->mPThrust).z;
		rigidBody2D->AddTorque(generatedTorque);
		rigidBody2D->AddForce(rigidBody2D->mPThrust);
	}
	if (rigidBody2D->mSThrust.y < 0.0f) {
		float generatedTorque = rigidBody2D->mStarboardBowThruster.Cross(rigidBody2D->mSThrust).z;
		rigidBody2D->AddTorque(generatedTorque);
		rigidBody2D->AddForce(rigidBody2D->mSThrust);
	}

	rigidBody2D->UpdateBodyEuler(dt);
	EnforceBoundaries();

	bounds->UpdateCenter(Vector3(transform->position.x + mWidth/2, transform->position.y + mHeight/2,0.0f));
}

void D2DSprite::Render(DirectX::SpriteBatch *spriteBatch) {
	shared_ptr<SpriteFont> spriteFont = ServiceProvider::Instance().GetGraphicsSystem()->GetSpriteFont();
	
	spriteBatch->Draw(mTexture->GetTexture(),transform->position,nullptr,mColor,transform->rotation.z,g_XMZero,transform->scale);
}

void D2DSprite::EnforceBoundaries() {
	int screenWidth = Screen::width;
	int screenHeight = Screen::height;

	if (transform->position.x > screenWidth - mWidth) {
		transform->position.x = screenWidth - mWidth;
	}
	else if (transform->position.x < 0) {
		transform->position.x = 0;
	}
	if (transform->position.y > screenHeight - mHeight) {
		transform->position.y = screenHeight - mHeight;
	}
	else if (transform->position.y < 0) {
		transform->position.y = 0;
	}
}

void D2DSprite::SetThrusters(bool p, bool s) {
	rigidBody2D->mPThrust.x = 0;
	rigidBody2D->mPThrust.y = 0;
	rigidBody2D->mSThrust.x = 0;
	rigidBody2D->mSThrust.y = 0;

	if (p) {
		rigidBody2D->mPThrust.y = STEERINGFORCE;		
	}
	if (s) {
		rigidBody2D->mSThrust.y = -STEERINGFORCE;
	}
}

void D2DSprite::ModulateThrust(bool up) {
	double dT = up ? DTHRUST : -DTHRUST;

	mThrustForce += dT;
	
	if(mThrustForce > MAXTHRUST) {
		mThrustForce = MAXTHRUST;
	}
	else if (mThrustForce < MINTHRUST) {
		mThrustForce = MINTHRUST;
	}
}