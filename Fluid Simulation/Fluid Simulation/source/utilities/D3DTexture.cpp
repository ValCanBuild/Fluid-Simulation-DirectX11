/***************************************************************
D3DTexture.cpp: A class that encapsulates a Direct3D Texture

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/
#include <string>
#include "D3DTexture.h"
#include "WICTextureLoader.h"

using namespace DirectX;

D3DTexture::D3DTexture() {

}

D3DTexture::~D3DTexture() {

}

bool D3DTexture::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, WCHAR* filename, HWND hwnd) {
	// Load the texture in.
	
	HRESULT result = CreateWICTextureFromFile(device,context,filename,nullptr,&mTexture);
	if(FAILED(result)) {
		std::wstring name = filename;
		std::wstring text = L"Error creating texture " + name;
		MessageBox(hwnd, text.c_str(), L"Error", MB_OK);
		return false;
	}

	return true;
}

ID3D11ShaderResourceView* D3DTexture::GetTexture() {
	return mTexture;
}