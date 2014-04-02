#include "TerrainTextureShader.h"
#include <CommonStates.h>
#include <VertexTypes.h>

TerrainTextureShader::TerrainTextureShader(void)  {
}


TerrainTextureShader::~TerrainTextureShader(void) {

}

void TerrainTextureShader::SetVertexBufferValues(ID3D11DeviceContext* context, const Matrix &wvpMatrix, const Matrix &worldMatrix) const {
	// set buffer values
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferVertex* dataPtr;

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mConstantVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("TerrainTextureShader: failed to map buffer in SetVertexBufferValues function"));
	}

	dataPtr = (InputBufferVertex*)mappedResource.pData;
	dataPtr->worldMatrix = worldMatrix.Transpose();
	dataPtr->wvpMatrix = wvpMatrix.Transpose();

	context->Unmap(mConstantVertexBuffer,0);	
}

void TerrainTextureShader::SetPixelBufferValues(ID3D11DeviceContext* context, float heightsArray[3]) const {
	// set buffer values
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBufferPixel* dataPtr;

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mConstantPixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("TerrainTextureShader: failed to map buffer in SetPixelBufferValues function"));
	}

	dataPtr = (InputBufferPixel*)mappedResource.pData;
	for (int i = 0; i < 3; ++i) {
		dataPtr->heights[i] = heightsArray[i];
	}

	context->Unmap(mConstantPixelBuffer, 0);	
}

void TerrainTextureShader::BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) {
	deviceContext->VSSetConstantBuffers(0, 1, &(mConstantVertexBuffer.p));
	deviceContext->PSSetConstantBuffers(0, 1, &(mConstantPixelBuffer.p));
	deviceContext->PSSetSamplers(0, 1, &(mSampleState.p));
	deviceContext->PSSetShaderResources(0, 3, pTextures);
}

ShaderDescription TerrainTextureShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vTerrainTexture.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "TerrainTextureVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pTerrainTexture.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "TerrainTexturePixelShader";

	shaderDescription.numLayoutElements = DirectX::VertexPositionNormalTexture::InputElementCount;
	shaderDescription.polygonLayout = new D3D11_INPUT_ELEMENT_DESC[shaderDescription.numLayoutElements];

	for (int i = 0; i < shaderDescription.numLayoutElements; ++i) {
		shaderDescription.polygonLayout[i] = DirectX::VertexPositionNormalTexture::InputElements[i];
	}

	return shaderDescription;
}

bool TerrainTextureShader::SpecificInitialization(ID3D11Device* device) {
	// Setup the sampler description
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	HRESULT result = device->CreateSamplerState(&samplerDesc, &mSampleState);
	if(FAILED(result)) {
		return false;
	}

	// Create the vertex constant buffer
	bool bResult = BuildDynamicBuffer<InputBufferVertex>(device, &mConstantVertexBuffer);
	if(!bResult) {
		return false;
	}

	// Create the pixel constant buffer
	bResult = BuildDynamicBuffer<InputBufferPixel>(device, &mConstantPixelBuffer);
	if(!bResult) {
		return false;
	}


	return true;
}

void TerrainTextureShader::SetTextures(ID3D11ShaderResourceView *textures[3]) {
	for (int i = 0; i < 3; i++) {
		pTextures[i] = textures[i];
	}
}

void TerrainTextureShader::Render(ID3D11DeviceContext* context, int indexCount) {
	RenderShader(context, indexCount);
}
