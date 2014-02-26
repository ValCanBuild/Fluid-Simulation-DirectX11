#include "TextureShader.h"
#include <CommonStates.h>

TextureShader::TextureShader(void) {
}


TextureShader::~TextureShader(void) {
}

void TextureShader::PrepareForRender(ID3D11DeviceContext* context, ID3D11ShaderResourceView* texture, const Matrix *wvpMatrix) {
	// set buffer values
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBuffer* dataPtr;

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		throw std::runtime_error(std::string("TextureShader: failed to map buffer in PrepareForRender function"));
	}

	dataPtr = (InputBuffer*)mappedResource.pData;
	dataPtr->wvpMatrix = wvpMatrix->Transpose();
	//dataPtr->worldMatrix = worldMatrix->Transpose();
	//dataPtr->viewMatrix =  viewMatrix->Transpose();
	//dataPtr->projMatrix =  projMatrix->Transpose();

	context->Unmap(mInputBuffer,0);

	context->VSSetConstantBuffers(0,1,&(mInputBuffer.p));
	context->PSSetShaderResources(0,1,&texture);

	// Set the buffer inside the pixel shader
}


bool TextureShader::Render(ID3D11DeviceContext* context, int indexCount) {

	// Set the parameters inside the shader
	//context->PSSetShaderResources(0,1,&texture);

	// Set the samplers inside the pixel shader
	//context->PSSetSamplers(0,1,&(mSampleState.p));
	//context->VSSetConstantBuffers(0,1,&(mInputBuffer.p));

	// Render
	RenderShader(context,indexCount);

	return true;
}

ShaderDescription TextureShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vTexture.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "TextureVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pTexture.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "TexturePixelShader";

	shaderDescription.numLayoutElements = 3;

	shaderDescription.polygonLayout = new D3D11_INPUT_ELEMENT_DESC[shaderDescription.numLayoutElements];

	shaderDescription.polygonLayout[0].SemanticName = "SV_Position";
	shaderDescription.polygonLayout[0].SemanticIndex = 0;
	shaderDescription.polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	shaderDescription.polygonLayout[0].InputSlot = 0;
	shaderDescription.polygonLayout[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	shaderDescription.polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[0].InstanceDataStepRate = 0;

	shaderDescription.polygonLayout[1].SemanticName = "NORMAL";
	shaderDescription.polygonLayout[1].SemanticIndex = 0;
	shaderDescription.polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	shaderDescription.polygonLayout[1].InputSlot = 0;
	shaderDescription.polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	shaderDescription.polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[1].InstanceDataStepRate = 0;

	shaderDescription.polygonLayout[2].SemanticName = "TEXCOORD";
	shaderDescription.polygonLayout[2].SemanticIndex = 0;
	shaderDescription.polygonLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	shaderDescription.polygonLayout[2].InputSlot = 0;
	shaderDescription.polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	shaderDescription.polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[2].InstanceDataStepRate = 0;


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
