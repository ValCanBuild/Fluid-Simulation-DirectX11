/********************************************************************
FluidResources.cpp: Implementation of FluidResources

Author:	Valentin Hinov
Date: 19/3/2014
*********************************************************************/

#include "FluidResources.h"

using namespace std;

CommonFluidResources CommonFluidResources::CreateResources(ID3D11Device * device, const Vector3 &textureSize, HWND hwnd) {
	HRESULT hr;

	CommonFluidResources resources;

	// Create shared texture description
	D3D11_TEXTURE3D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE3D_DESC));
	textureDesc.Width = (UINT) textureSize.x;
	textureDesc.Height = (UINT) textureSize.y;
	textureDesc.Depth = (UINT) textureSize.z;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R16_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create pressure shader params
	CComPtr<ID3D11Texture3D> pressureText[2];	
	for (int i = 0; i < 2; ++i) {
		hr = device->CreateTexture3D(&textureDesc, NULL, &pressureText[i]);
		if (FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the pressure Texture Object", L"Error", MB_OK);
		}
		// Create the SRV and UAV.
		hr = device->CreateShaderResourceView(pressureText[i], NULL, &resources.pressureSP[i].mSRV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the pressure SRV", L"Error", MB_OK);
		}

		hr = device->CreateUnorderedAccessView(pressureText[i], NULL, &resources.pressureSP[i].mUAV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the pressure UAV", L"Error", MB_OK);
		}
	}

	// Create divergence shader params
	CComPtr<ID3D11Texture3D> divergenceText;
	hr = device->CreateTexture3D(&textureDesc, NULL, &divergenceText);
	// Create the SRV and UAV.
	hr = device->CreateShaderResourceView(divergenceText, NULL, &resources.divergenceSP.mSRV);
	if(FAILED(hr)) {
		MessageBox(hwnd, L"Could not create the divergence SRV", L"Error", MB_OK);
	}
	hr = device->CreateUnorderedAccessView(divergenceText, NULL, &resources.divergenceSP.mUAV);
	if(FAILED(hr)) {
		MessageBox(hwnd, L"Could not create the divergence UAV", L"Error", MB_OK);
	}

	// Create the temp shader params
	CComPtr<ID3D11Texture3D> tempText[2];
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // 3 components for velocity in 3D + alpha
	for (int i = 0; i < 2; ++i) {
		hr = device->CreateTexture3D(&textureDesc, NULL, &tempText[i]);
		if (FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the velocity Texture Object", L"Error", MB_OK);
		}
		hr = device->CreateShaderResourceView(tempText[i], NULL, &resources.tempSP[i].mSRV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the velocity SRV", L"Error", MB_OK);

		}
		hr = device->CreateUnorderedAccessView(tempText[i], NULL, &resources.tempSP[i].mUAV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the velocity UAV", L"Error", MB_OK);
		}
	}

	return resources;
}

FluidResourcesPerObject FluidResourcesPerObject::CreateResourcesSmoke(ID3D11Device * device, const Vector3 &textureSize, HWND hwnd) {
	FluidResourcesPerObject resources;

	// Create the velocity shader params
	CComPtr<ID3D11Texture3D> velocityText[2];
	D3D11_TEXTURE3D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE3D_DESC));
	textureDesc.Width = (UINT) textureSize.x;
	textureDesc.Height = (UINT) textureSize.y;
	textureDesc.Depth = (UINT) textureSize.z;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;	// 3 components for velocity in 3D + alpha
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	for (int i = 0; i < 2; ++i) {
		HRESULT hr = device->CreateTexture3D(&textureDesc, NULL, &velocityText[i]);
		if (FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the velocity Texture Object", L"Error", MB_OK);
		}
		hr = device->CreateShaderResourceView(velocityText[i], NULL, &resources.velocitySP[i].mSRV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the velocity SRV", L"Error", MB_OK);

		}
		hr = device->CreateUnorderedAccessView(velocityText[i], NULL, &resources.velocitySP[i].mUAV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the velocity UAV", L"Error", MB_OK);
		}
	}

	// Create the obstacle shader params
	CComPtr<ID3D11Texture3D> obstacleText;
	textureDesc.Format = DXGI_FORMAT_R8_SINT;
	HRESULT hresult = device->CreateTexture3D(&textureDesc, NULL, &obstacleText);
	if (FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the obstacle Texture Object", L"Error", MB_OK);
	}
	hresult = device->CreateShaderResourceView(obstacleText, NULL, &resources.obstacleSP.mSRV);
	if(FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the obstacle SRV", L"Error", MB_OK);

	}
	hresult = device->CreateUnorderedAccessView(obstacleText, NULL, &resources.obstacleSP.mUAV);
	if(FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the obstacle UAV", L"Error", MB_OK);
	}

	// Create the density shader params
	CComPtr<ID3D11Texture3D> densityText[2];
	textureDesc.Format = DXGI_FORMAT_R16_FLOAT;
	for (int i = 0; i < 2; ++i) {
		HRESULT hr = device->CreateTexture3D(&textureDesc, NULL, &densityText[i]);
		if (FAILED(hr)){
			MessageBox(hwnd, L"Could not create the density Texture Object", L"Error", MB_OK);
		}
		// Create the SRV and UAV.
		hr = device->CreateShaderResourceView(densityText[i], NULL, &resources.densitySP[i].mSRV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the density SRV", L"Error", MB_OK);
		}
		hr = device->CreateUnorderedAccessView(densityText[i], NULL, &resources.densitySP[i].mUAV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the density UAV", L"Error", MB_OK);
		}
	}

	// Create the temperature shader params
	CComPtr<ID3D11Texture3D> temperatureText[2];
	for (int i = 0; i < 2; ++i) {
		HRESULT hr = device->CreateTexture3D(&textureDesc, NULL, &temperatureText[i]);
		if (FAILED(hr)){
			MessageBox(hwnd, L"Could not create the temperature Texture Object", L"Error", MB_OK);
		}
		// Create the SRV and UAV.
		hr = device->CreateShaderResourceView(temperatureText[i], NULL, &resources.temperatureSP[i].mSRV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the temperature SRV", L"Error", MB_OK);
		}

		hr = device->CreateUnorderedAccessView(temperatureText[i], NULL, &resources.temperatureSP[i].mUAV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the temperature UAV", L"Error", MB_OK);
		}
	}

	// Create the vorticity shader params
	CComPtr<ID3D11Texture3D> vorticityText;
	hresult = device->CreateTexture3D(&textureDesc, NULL, &vorticityText);
	if (FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the vorticity Texture Object", L"Error", MB_OK);
	}
	hresult = device->CreateShaderResourceView(vorticityText, NULL, &resources.vorticitySP.mSRV);
	if(FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the vorticity SRV", L"Error", MB_OK);

	}
	hresult = device->CreateUnorderedAccessView(vorticityText, NULL, &resources.vorticitySP.mUAV);
	if(FAILED(hresult)) {
		MessageBox(hwnd, L"Could not create the vorticity UAV", L"Error", MB_OK);
	}


	return resources;
}

FluidResourcesPerObject FluidResourcesPerObject::CreateResourcesFire(ID3D11Device * device, const Vector3 &textureSize, HWND hwnd) {
	FluidResourcesPerObject resources = CreateResourcesSmoke(device, textureSize, hwnd);

	CComPtr<ID3D11Texture3D> reactionText[2];
	D3D11_TEXTURE3D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE3D_DESC));
	textureDesc.Width = (UINT) textureSize.x;
	textureDesc.Height = (UINT) textureSize.y;
	textureDesc.Depth = (UINT) textureSize.z;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R16_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	for (int i = 0; i < 2; ++i) {
		HRESULT hr = device->CreateTexture3D(&textureDesc, NULL, &reactionText[i]);
		if (FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the reaction Texture Object", L"Error", MB_OK);
		}
		hr = device->CreateShaderResourceView(reactionText[i], NULL, &resources.reactionSP[i].mSRV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the reaction SRV", L"Error", MB_OK);

		}
		hr = device->CreateUnorderedAccessView(reactionText[i], NULL, &resources.reactionSP[i].mUAV);
		if(FAILED(hr)) {
			MessageBox(hwnd, L"Could not create the reaction UAV", L"Error", MB_OK);
		}
	}

	return resources;
}