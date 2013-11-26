/***************************************************************
D3DTexture.h: A class that encapsulates a Direct3D Texture

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
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

private:
	CComPtr<ID3D11ShaderResourceView> mTexture;
};

#endif