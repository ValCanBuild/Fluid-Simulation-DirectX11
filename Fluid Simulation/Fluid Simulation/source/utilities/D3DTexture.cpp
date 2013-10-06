/***************************************************************
D3DTexture.cpp: A class that encapsulates a Direct3D Texture

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#include "D3DTexture.h"

D3DTexture::D3DTexture() {

}

D3DTexture::~D3DTexture() {

}

bool D3DTexture::Initialize(ID3D11Device* device, WCHAR* filename) {
	// Load the texture in.
	HRESULT result = D3DX11CreateShaderResourceViewFromFile(device, filename, NULL, NULL, &mTexture._Myptr, NULL);
	if(FAILED(result)) {
		return false;
	}

	return true;
}

ID3D11ShaderResourceView* D3DTexture::GetTexture() {
	return mTexture.get();
}