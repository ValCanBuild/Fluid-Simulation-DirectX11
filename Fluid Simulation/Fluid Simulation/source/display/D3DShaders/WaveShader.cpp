#include "WaveShader.h"
#include "../D3DGraphicsObject.h"

WaveShader::WaveShader(void) {
}


WaveShader::~WaveShader(void) {
}

bool WaveShader::Render(D3DGraphicsObject* graphicsObject, int indexCount, ID3D11ShaderResourceView** texArr) {
	// set the parameters inside the vertex shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ScreenSizeBuffer* dataPtr;

	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mScreenSizeBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (ScreenSizeBuffer*)mappedResource.pData;
	int width,height;
	graphicsObject->GetScreenDimensions(width,height);
	dataPtr->screenWidth = (float)width;
	dataPtr->screenHeight = (float)height;
	dataPtr->padding = Vector2f(0,0);

	context->Unmap(mScreenSizeBuffer.get(),0);

	// Now set the constant buffer in the vertex shader with the updated values.
	context->VSSetConstantBuffers(0, 1, &(mScreenSizeBuffer._Myptr));
	
	// Set the parameters inside the pixel shader
	context->PSSetShaderResources(0,2,texArr);

	context->PSSetSamplers(0,1,&mSampleState._Myptr);

	// Render
	RenderShader(context,indexCount);

	return true;
}

ShaderDescription WaveShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/wave_equation.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "WaveVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/wave_equation.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "WavePixelShader";

	shaderDescription.polygonLayout = new D3D11_INPUT_ELEMENT_DESC[2];

	shaderDescription.polygonLayout[0].SemanticName = "POSITION";
	shaderDescription.polygonLayout[0].SemanticIndex = 0;
	shaderDescription.polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	shaderDescription.polygonLayout[0].InputSlot = 0;
	shaderDescription.polygonLayout[0].AlignedByteOffset = 0;
	shaderDescription.polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[0].InstanceDataStepRate = 0;

	shaderDescription.polygonLayout[1].SemanticName = "TEXCOORD";
	shaderDescription.polygonLayout[1].SemanticIndex = 0;
	shaderDescription.polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	shaderDescription.polygonLayout[1].InputSlot = 0;
	shaderDescription.polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	shaderDescription.polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[1].InstanceDataStepRate = 0;

	shaderDescription.numLayoutElements = 2;

	return shaderDescription;
}

bool WaveShader::SpecificInitialization(ID3D11Device* device) {
	D3D11_BUFFER_DESC screenSizeBufferDesc;
	// Setup the description of the dynamic screen size constant buffer that is in the vertex shader.
	screenSizeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	screenSizeBufferDesc.ByteWidth = sizeof(ScreenSizeBuffer);
	screenSizeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	screenSizeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	screenSizeBufferDesc.MiscFlags = 0;
	screenSizeBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	HRESULT result = device->CreateBuffer(&screenSizeBufferDesc, NULL, &mScreenSizeBuffer._Myptr);
	if(FAILED(result)) {
		return false;
	}

	// Setup the sampler description
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &mSampleState._Myptr);
	if(FAILED(result)) {
		return false;
	}

	return true;
}