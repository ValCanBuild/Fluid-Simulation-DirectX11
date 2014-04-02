/***************************************************************
D3DTexture.cpp: A class that encapsulates a Direct3D Texture

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/
#include "D3DTexture.h"

#include <string>
#include <wrl.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

using namespace Microsoft::WRL;

using namespace DirectX;

D3DTexture::D3DTexture() : mTexture(nullptr) {

}

D3DTexture::~D3DTexture() {

}

bool D3DTexture::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, WCHAR* filename, HWND hwnd) {
	// Load the texture in.
	HRESULT result;
	if (IsDDSTextureFile(filename)) {
		result = DirectX::CreateDDSTextureFromFile(device,filename,nullptr,&mTexture);
	}
	else {
		result = DirectX::CreateWICTextureFromFile(device,context,filename,nullptr,&mTexture);
	}
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

void D3DTexture::GetTextureSize(int &width, int &height) {
	// Convert resource view to underlying resource.
	ComPtr<ID3D11Resource> resource;

	mTexture->GetResource(&resource);
	
	// Cast to texture.
	ComPtr<ID3D11Texture2D> texture2D;
	
	if (FAILED(resource.As(&texture2D))) {
		throw std::exception("SpriteBatch can only draw Texture2D resources");
	}

	// Query the texture size.
	D3D11_TEXTURE2D_DESC desc;

	texture2D->GetDesc(&desc);
	
	// Convert to vector format.
	width = desc.Width;
	height = desc.Height;
}

bool D3DTexture::IsDDSTextureFile(WCHAR* filename) const {
	WCHAR * dds = L".dds";
	WCHAR * contains = wcsstr(filename, dds);
	return contains != nullptr;
}
