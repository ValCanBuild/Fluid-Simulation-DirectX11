/***************************************************************
D3DTexture.h: A class that encapsulates a Direct3D Texture

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/

#ifndef _D3DTEXTURE_H
#define _D3DTEXTURE_H

#include <memory>

#include "D3dIncludes.h"

class D3DTexture {
public:
	D3DTexture();
	~D3DTexture();

	bool Initialize(ID3D11Device* device, WCHAR* filename);

	ID3D11ShaderResourceView* GetTexture();

private:
	std::unique_ptr<ID3D11ShaderResourceView,COMDeleter> mTexture;
};

#endif