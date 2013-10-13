/***************************************************************
D3DTexture.cpp: A class that encapsulates a Direct3D Texture

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#include "D3DTexture.h"
#include "WICTextureLoader.h"

using namespace DirectX;

D3DTexture::D3DTexture() {

}

D3DTexture::~D3DTexture() {

}

bool D3DTexture::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, WCHAR* filename) {
	// Load the texture in.
	
	HRESULT result = CreateWICTextureFromFile(device,context,filename,nullptr,&mTexture);
	if(FAILED(result)) {
		return false;
	}

	return true;
}

ID3D11ShaderResourceView* D3DTexture::GetTexture() {
	return mTexture;
}