/***************************************************************
D2DParticle.h: A Direct3D sprite renderable that adheres to
rigid body physics

Author: Valentin Hinov
Date: 01/12/2013
**************************************************************/
#include <string>

#include "D2DParticle.h"

#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "../utilities/Physics.h"
#include "../utilities/D3DTexture.h"
#include "../utilities/Screen.h"
#include "../system/ServiceProvider.h"
#include "../utilities/math/MathUtils.h"


using namespace DirectX;

#pragma warning( disable : 4244)	// ignore implicit conversion warning

D2DParticle::D2DParticle(float mass, Vector3 &position, float scale) :
	 mWidth(0), mHeight(0)
{
	transform = shared_ptr<Transform>(new Transform(this, position,Vector3(0,0,0),Vector3(scale)));	
	rigidBody2D = shared_ptr<RigidBody2D>(new RigidBody2D(this));
	rigidBody2D->mMass = mass;
	rigidBody2D->mRadius = scale;

	mColor = RandomColor();
}

D2DParticle::~D2DParticle() {
}

bool D2DParticle::Initialize(D3DGraphicsObject *pGraphicsObj, HWND hwnd, WCHAR *textureFilename) {
	mTexture = unique_ptr<D3DTexture>(new D3DTexture());
	bool result = mTexture->Initialize(pGraphicsObj->GetDevice(),pGraphicsObj->GetDeviceContext(),textureFilename,hwnd);
	
	if (result) {
		mTexture->GetTextureSize(mWidth,mHeight);
		mWidth *= transform->scale.x;
		mHeight *= transform->scale.y;

		Vector3 center = Vector3(transform->position.x + mWidth*0.5f, transform->position.y + mHeight*0.5f,0.0f);
		bounds = shared_ptr<Bounds>(new Bounds(this,BOUNDS_TYPE_SPHERE,center,Vector3(),mWidth/2));
	}

	return result;
}

void D2DParticle::FixedUpdate(float dt, vector<D2DParticle*> &obstacles) {
	for (D2DParticle *pObstacle : obstacles) {
		rigidBody2D->CollisionCheck(dt,pObstacle->rigidBody2D.get(),COLLISION_TYPE2D_CIRCLE);
	}
	rigidBody2D->UpdateBodyEuler(dt);
	EnforceBoundaries();

	bounds->UpdateCenter(Vector3(transform->position.x + mWidth/2, transform->position.y + mHeight/2,0.0f));
}

void D2DParticle::Render(DirectX::SpriteBatch *spriteBatch) {
	shared_ptr<SpriteFont> spriteFont = ServiceProvider::Instance().GetGraphicsSystem()->GetSpriteFont();
	
	spriteBatch->Draw(mTexture->GetTexture(),transform->position,nullptr,mColor,transform->rotation.z,g_XMZero,transform->scale);
	wstring text = std::to_wstring((int)rigidBody2D->GetMass());
	
	Vector2 stringSize = spriteFont->MeasureString(text.c_str());
	Vector2 stringPos = Vector2((stringSize.x - mWidth)*0.5f,(stringSize.y - mHeight)*0.5f);
	spriteFont->DrawString(spriteBatch,text.c_str(),transform->position,Colors::Black,0,stringPos);

}

void D2DParticle::EnforceBoundaries() {
	int screenWidth = Screen::width;

	if (transform->position.x > screenWidth - mWidth) {
		transform->position.x = screenWidth - mWidth;
	}
	else if (transform->position.x < 0) {
		transform->position.x = 0;
	}
}