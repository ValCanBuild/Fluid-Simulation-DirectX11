#include "TextureShader.h"
#include <CommonStates.h>
#include <VertexTypes.h>

TextureShader::TextureShader(void) : pTexture(nullptr) {
}


TextureShader::~TextureShader(void) {
	pTexture = nullptr;
}

void TextureShader::SetVertexBufferValues(ID3D11DeviceContext* context, const Matrix &wvpMatrix) const {
	// set buffer values
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBuffer* dataPtr;

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("TextureShader: failed to map buffer in SetVertexBufferValues function"));
	}

	dataPtr = (InputBuffer*)mappedResource.pData;
	dataPtr->wvpMatrix = wvpMatrix.Transpose();

	context->Unmap(mInputBuffer,0);	
}

void TextureShader::BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) {
	deviceContext->VSSetConstantBuffers(0, 1, &(mInputBuffer.p));
	deviceContext->PSSetShaderResources(0, 1, &pTexture);
}

ShaderDescription TextureShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vTexture.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "TextureVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pTexture.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "TexturePixelShader";

	shaderDescription.numLayoutElements = DirectX::VertexPositionNormalTexture::InputElementCount;
	shaderDescription.polygonLayout = new D3D11_INPUT_ELEMENT_DESC[shaderDescription.numLayoutElements];

	for (int i = 0; i < shaderDescription.numLayoutElements; ++i) {
		shaderDescription.polygonLayout[i] = DirectX::VertexPositionNormalTexture::InputElements[i];
	}

	return shaderDescription;
}

bool TextureShader::SpecificInitialization(ID3D11Device* device) {
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

	// Create the buffer
	D3D11_BUFFER_DESC inputBufferDesc;
	inputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	inputBufferDesc.ByteWidth = sizeof(InputBuffer);
	inputBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	inputBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inputBufferDesc.MiscFlags = 0;
	inputBufferDesc.StructureByteStride = 0;
	// General buffer
	HRESULT hresult = device->CreateBuffer(&inputBufferDesc, NULL, &mInputBuffer);
	if(FAILED(hresult)) {
		return false;
	}

	return true;
}

void TextureShader::SetTexture(ID3D11ShaderResourceView *texture) {
	pTexture = texture;
}

void TextureShader::Render(ID3D11DeviceContext* context, int indexCount) {
	RenderShader(context, indexCount);
}
