/***************************************************************
D3DTexture.h: A class that encapsulates a Direct3D Texture.
Can load both a variety of formats including .dds, .jpg,
.png

Author: Valentin Hinov
Date: 04/09/2013
**************************************************************/

#ifndef _D3DTEXTURE_H
#define _D3DTEXTURE_H

#include <atlbase.h>

#include "D3dIncludes.h"

class D3DTexture {
public:
	D3DTexture();
	~D3DTexture();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, WCHAR* filename, HWND hwnd = nullptr);

	ID3D11ShaderResourceView* GetTexture();

	void GetTextureSize(int &width, int &height);

private:
	bool IsDDSTextureFile(WCHAR* filename) const;

private:
	CComPtr<ID3D11ShaderResourceView> mTexture;
};

#endif