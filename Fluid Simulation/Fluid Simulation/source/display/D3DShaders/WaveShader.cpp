#include "WaveShader.h"
#include "../D3DGraphicsObject.h"

WaveShader::WaveShader(void) {
}


WaveShader::~WaveShader(void) {
}

bool WaveShader::Render(D3DGraphicsObject* graphicsObject, int indexCount, ID3D11ShaderResourceView* texNow, ID3D11ShaderResourceView* texPrev) {
	// set the parameters inside the vertex shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ScreenSizeBuffer* dataPtr;

	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mScreenSizeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (ScreenSizeBuffer*)mappedResource.pData;
	int width,height;
	graphicsObject->GetScreenDimensions(width,height);
	dataPtr->screenWidth = (float)width;
	dataPtr->screenHeight = (float)height;
	dataPtr->padding = Vector2(0,0);

	context->Unmap(mScreenSizeBuffer,0);

	// Now set the constant buffer in the vertex shader with the updated values.
	context->VSSetConstantBuffers(0, 1, &(mScreenSizeBuffer.p));

	// Set the parameters inside the pixel shader
	context->PSSetShaderResources(0,1,&texNow);
	context->PSSetShaderResources(1,1,&texPrev);

	context->PSSetSamplers(0,1,&(mSampleState.p));
	
	// Render
	RenderShader(context,indexCount);

	return true;
}

void WaveShader::Compute(_In_ D3DGraphicsObject* graphicsObject, const Vector3 &mousePos, int pressed, _In_ ID3D11ShaderResourceView* texNow, _In_ ID3D11ShaderResourceView* texPrev, _In_ ID3D11UnorderedAccessView* result) const {
	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// set buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBuffer* dataPtr;

	// Lock the input constant buffer so it can be written to.
	HRESULT hr = context->Map(mInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(hr)) {
		return;
	}

	dataPtr = (InputBuffer*)mappedResource.pData;
	dataPtr->mouse = mousePos;
	dataPtr->mousePressed = pressed;

	context->Unmap(mInputBuffer,0);

	// Now set the constant buffer in the compute shader with the updated values.
	context->CSSetConstantBuffers(0, 1, &(mInputBuffer.p));

	context->CSSetShaderResources(0,1,&texNow);
	context->CSSetShaderResources(1,1,&texPrev);
	context->CSSetUnorderedAccessViews(0,1,&result,nullptr);

	int width,height;
	graphicsObject->GetScreenDimensions(width,height);
	UINT numThreadX = (UINT)ceil(width/32.0f);
	UINT numThreadY = (UINT)ceil(height/32.0f);
	SetComputeShader(context);
	context->Dispatch(numThreadX,numThreadY,1);

	ID3D11UnorderedAccessView *const pUAV[1] = {NULL};
	context->CSSetUnorderedAccessViews(0,1,pUAV,nullptr);
	ID3D11ShaderResourceView *const pSRV[2] = {NULL,NULL};
	context->CSSetShaderResources(0,2,pSRV);

	
}

ShaderDescription WaveShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vWave_equation.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "WaveVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pWave_equation.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "WavePixelShader";

	shaderDescription.computeShaderDesc.shaderFilename = L"hlsl/cWaveEquation.hlsl";
	shaderDescription.computeShaderDesc.shaderFunctionName = "WaveComputeShader";

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
	D3D11_BUFFER_DESC bufferDesc;
	// Setup the description of the dynamic screen size constant buffer that is in the vertex shader.
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(ScreenSizeBuffer);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	HRESULT result = device->CreateBuffer(&bufferDesc, NULL, &mScreenSizeBuffer);
	if(FAILED(result)) {
		return false;
	}

	bufferDesc.ByteWidth = sizeof(InputBuffer);
	result = device->CreateBuffer(&bufferDesc, NULL, &mInputBuffer);
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
	result = device->CreateSamplerState(&samplerDesc, &mSampleState);
	if(FAILED(result)) {
		return false;
	}

	return true;
}