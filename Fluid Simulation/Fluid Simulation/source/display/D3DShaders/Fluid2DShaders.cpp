/*************************************************************
Fluid2DShaders.cpp: Contains all the shader classes needed for
2D fluid simulation

Author: Valentin Hinov
Date: 11/09/2013
**************************************************************/
#include "Fluid2DShaders.h"
#include "../D3DGraphicsObject.h"

D3D11_INPUT_ELEMENT_DESC *CreateCommonInputLayout() {
	D3D11_INPUT_ELEMENT_DESC *polygonLayout = new D3D11_INPUT_ELEMENT_DESC[2];

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	return polygonLayout;
}

///////ADVECTION SHADER BEGIN////////
AdvectionShader::AdvectionShader() {

}

AdvectionShader::~AdvectionShader() {

}

bool AdvectionShader::Render(D3DGraphicsObject* graphicsObject, int indexCount, float timeStep, float dissipation, ID3D11ShaderResourceView* velocityField, ID3D11ShaderResourceView* advectTarget) {
	// set the parameters inside the vertex shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBuffer* dataPtr;

	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (InputBuffer*)mappedResource.pData;
	int width,height;
	graphicsObject->GetScreenDimensions(width,height);
	dataPtr->fTextureWidth = (float)width;
	dataPtr->fTextureHeight = (float)height;
	dataPtr->fTimeStep = timeStep;
	dataPtr->fDissipation = dissipation;

	context->Unmap(mInputBuffer,0);

	// Set the buffer inside the pixel shader
	context->PSSetConstantBuffers(0,1,&(mInputBuffer.p));

	// Set the parameters inside the pixel shader
	context->PSSetShaderResources(0,1,&velocityField);
	context->PSSetShaderResources(1,1,&advectTarget);

	// Set the pixel shader sampler
	context->PSSetSamplers(0,1,&(mSampleState.p));
	
	// Render
	RenderShader(context,indexCount);

	return true;
}

ShaderDescription AdvectionShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vOrthotexture.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "TextureVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pAdvection.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "AdvectionPixelShader";

	shaderDescription.polygonLayout = CreateCommonInputLayout();

	shaderDescription.numLayoutElements = 2;

	return shaderDescription;
}

bool AdvectionShader::SpecificInitialization(ID3D11Device* device) {
	D3D11_BUFFER_DESC inputBufferDesc;
	// Setup the description of the dynamic screen size constant buffer that is in the vertex shader.
	inputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	inputBufferDesc.ByteWidth = sizeof(InputBuffer);
	inputBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	inputBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inputBufferDesc.MiscFlags = 0;
	inputBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	HRESULT result = device->CreateBuffer(&inputBufferDesc, NULL, &mInputBuffer);
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
///////ADVECTION SHADER END////////


///////IMPULSE SHADER BEGIN////////
ImpulseShader::ImpulseShader() {

}

ImpulseShader::~ImpulseShader() {

}

bool ImpulseShader::Render(D3DGraphicsObject* graphicsObject, int indexCount, Vector2 point, Vector2 fill, float radius, ID3D11ShaderResourceView* originalState) {
// set the parameters inside the vertex shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBuffer* dataPtr;

	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (InputBuffer*)mappedResource.pData;
	dataPtr->vPoint = point;
	dataPtr->vFillColor = fill;
	dataPtr->fRadius = radius;
	dataPtr->padding = Vector3();

	context->Unmap(mInputBuffer,0);

	// Set the buffer inside the pixel shader
	context->PSSetConstantBuffers(0,1,&(mInputBuffer.p));

	// Set the texture inside the pixel shader
	context->PSSetShaderResources(0,1,&originalState);

	// Set the sampler inside the pixel shader
	context->PSSetSamplers(0,1,&(mSampleState.p));
	
	// Render
	RenderShader(context,indexCount);

	return true;
}

ShaderDescription ImpulseShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vOrthotexture.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "TextureVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pImpulse.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "ImpulsePixelShader";

	shaderDescription.polygonLayout = CreateCommonInputLayout();

	shaderDescription.numLayoutElements = 2;

	return shaderDescription;
}

bool ImpulseShader::SpecificInitialization(ID3D11Device* device) {
	D3D11_BUFFER_DESC inputBufferDesc;
	// Setup the description of the dynamic screen size constant buffer that is in the vertex shader.
	inputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	inputBufferDesc.ByteWidth = sizeof(InputBuffer);
	inputBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	inputBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inputBufferDesc.MiscFlags = 0;
	inputBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	HRESULT result = device->CreateBuffer(&inputBufferDesc, NULL, &mInputBuffer);
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
///////IMPULSE SHADER END////////


///////JACOBI SHADER BEGIN////////
JacobiShader::JacobiShader() {

}

JacobiShader::~JacobiShader() {

}

bool JacobiShader::Render(D3DGraphicsObject* graphicsObject, int indexCount, float alpha, float inverseBeta, ID3D11ShaderResourceView* pressureField, ID3D11ShaderResourceView* divergence) {
// set the parameters inside the vertex shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBuffer* dataPtr;

	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (InputBuffer*)mappedResource.pData;
	int width,height;
	graphicsObject->GetScreenDimensions(width,height);
	dataPtr->fTextureWidth = (float)width;
	dataPtr->fTextureHeight = (float)height;
	dataPtr->fAlpha = alpha;
	dataPtr->fInverseBeta = inverseBeta;

	context->Unmap(mInputBuffer,0);

	// Set the buffer inside the pixel shader
	context->PSSetConstantBuffers(0,1,&(mInputBuffer.p));

	// Set the parameters inside the pixel shader
	context->PSSetShaderResources(0,1,&pressureField);
	context->PSSetShaderResources(1,1,&divergence);

	// Set the pixel shader sampler
	context->PSSetSamplers(0,1,&(mSampleState.p));
	
	// Render
	RenderShader(context,indexCount);

	return true;
}

ShaderDescription JacobiShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vOrthotexture.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "TextureVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pJacobisolver.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "JacobiPixelShader";

	shaderDescription.polygonLayout = CreateCommonInputLayout();

	shaderDescription.numLayoutElements = 2;

	return shaderDescription;
}

bool JacobiShader::SpecificInitialization(ID3D11Device* device) {
	D3D11_BUFFER_DESC inputBufferDesc;
	// Setup the description of the dynamic screen size constant buffer that is in the vertex shader.
	inputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	inputBufferDesc.ByteWidth = sizeof(InputBuffer);
	inputBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	inputBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inputBufferDesc.MiscFlags = 0;
	inputBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	HRESULT result = device->CreateBuffer(&inputBufferDesc, NULL, &mInputBuffer);
	if(FAILED(result)) {
		return false;
	}

	// Setup the sampler description
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;	// Jacobi requires exact texel values, disable linear interpolation
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
///////JACOBI SHADER END////////


///////DIVERGENCE SHADER BEGIN////////
DivergenceShader::DivergenceShader() {

}

DivergenceShader::~DivergenceShader() {

}

bool DivergenceShader::Render(D3DGraphicsObject* graphicsObject, int indexCount, float halfInverseCellSize, ID3D11ShaderResourceView* targetField) {
// set the parameters inside the vertex shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBuffer* dataPtr;

	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (InputBuffer*)mappedResource.pData;
	int width,height;
	graphicsObject->GetScreenDimensions(width,height);
	dataPtr->fTextureWidth = (float)width;
	dataPtr->fTextureHeight = (float)height;
	dataPtr->fHalfInverseCellSize = halfInverseCellSize;
	dataPtr->padding = 0.0f;

	context->Unmap(mInputBuffer,0);

	// Set the buffer inside the pixel shader
	context->PSSetConstantBuffers(0,1,&(mInputBuffer.p));

	// Set the parameters inside the pixel shader
	context->PSSetShaderResources(0,1,&targetField);

	// Set the pixel shader sampler
	context->PSSetSamplers(0,1,&(mSampleState.p));
	
	// Render
	RenderShader(context,indexCount);

	return true;
}

ShaderDescription DivergenceShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vOrthotexture.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "TextureVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pDivergence.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "DivergencePixelShader";

	shaderDescription.polygonLayout = CreateCommonInputLayout();

	shaderDescription.numLayoutElements = 2;

	return shaderDescription;
}

bool DivergenceShader::SpecificInitialization(ID3D11Device* device) {
	D3D11_BUFFER_DESC inputBufferDesc;
	// Setup the description of the dynamic screen size constant buffer that is in the vertex shader.
	inputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	inputBufferDesc.ByteWidth = sizeof(InputBuffer);
	inputBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	inputBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inputBufferDesc.MiscFlags = 0;
	inputBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	HRESULT result = device->CreateBuffer(&inputBufferDesc, NULL, &mInputBuffer);
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
///////DIVERGENCE SHADER END////////


///////SUBTRACT GRADIENT SHADER END////////
SubtractGradientShader::SubtractGradientShader() {

}

SubtractGradientShader::~SubtractGradientShader() {

}

bool SubtractGradientShader::Render(D3DGraphicsObject* graphicsObject, int indexCount, float gradientScale, ID3D11ShaderResourceView* velocityField, ID3D11ShaderResourceView* pressureField) {
// set the parameters inside the vertex shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBuffer* dataPtr;

	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (InputBuffer*)mappedResource.pData;
	int width,height;
	graphicsObject->GetScreenDimensions(width,height);
	dataPtr->fTextureWidth = (float)width;
	dataPtr->fTextureHeight = (float)height;
	dataPtr->fGradientScale = gradientScale;
	dataPtr->padding0 = 0.0f;

	context->Unmap(mInputBuffer,0);

	// Set the buffer inside the pixel shader
	context->PSSetConstantBuffers(0,1,&(mInputBuffer.p));

	// Set the parameters inside the pixel shader
	context->PSSetShaderResources(0,1,&velocityField);
	context->PSSetShaderResources(1,1,&pressureField);

	// Set the pixel shader sampler
	context->PSSetSamplers(0,1,&(mSampleStateVelocity.p));
	context->PSSetSamplers(1,1,&(mSampleStatePressure.p));
	
	// Render
	RenderShader(context,indexCount);

	return true;
}

ShaderDescription SubtractGradientShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vOrthotexture.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "TextureVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pSubtractgradient.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "SubtractGradientPixelShader";

	shaderDescription.polygonLayout = CreateCommonInputLayout();

	shaderDescription.numLayoutElements = 2;

	return shaderDescription;
}

bool SubtractGradientShader::SpecificInitialization(ID3D11Device* device) {
	D3D11_BUFFER_DESC inputBufferDesc;
	// Setup the description of the dynamic screen size constant buffer that is in the vertex shader.
	inputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	inputBufferDesc.ByteWidth = sizeof(InputBuffer);
	inputBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	inputBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inputBufferDesc.MiscFlags = 0;
	inputBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	HRESULT result = device->CreateBuffer(&inputBufferDesc, NULL, &mInputBuffer);
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
	result = device->CreateSamplerState(&samplerDesc, &mSampleStateVelocity);
	if(FAILED(result)) {
		return false;
	}

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;	// have to sample the pressure at precise point
	result = device->CreateSamplerState(&samplerDesc, &mSampleStatePressure);
	if(FAILED(result)) {
		return false;
	}


	return true;
}
///////SUBTRACT GRADIENT SHADER END////////


///////BUOYANCY SHADER BEGIN////////
BuoyancyShader::BuoyancyShader() {

}

BuoyancyShader::~BuoyancyShader() {

}

bool BuoyancyShader::Render(D3DGraphicsObject* graphicsObject, int indexCount, float timeStep, float buoyancy, float weight, float ambTemp, ID3D11ShaderResourceView* velocityField, ID3D11ShaderResourceView* temperatureField, ID3D11ShaderResourceView* density) {
	// set the parameters inside the vertex shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InputBuffer* dataPtr;

	ID3D11DeviceContext *context = graphicsObject->GetDeviceContext();

	// Lock the screen size constant buffer so it can be written to.
	HRESULT result = context->Map(mInputBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	dataPtr = (InputBuffer*)mappedResource.pData;
	dataPtr->fTimeStep = timeStep;
	dataPtr->fSigma = buoyancy;
	dataPtr->fKappa = weight;
	dataPtr->fAmbientTemperature = ambTemp;

	context->Unmap(mInputBuffer,0);

	// Set the buffer inside the pixel shader
	context->PSSetConstantBuffers(0,1,&(mInputBuffer.p));

	// Set the parameters inside the pixel shader
	context->PSSetShaderResources(0,1,&velocityField);
	context->PSSetShaderResources(1,1,&temperatureField);
	context->PSSetShaderResources(2,1,&density);

	// Set the pixel shader sampler
	context->PSSetSamplers(0,1,&(mSampleState.p));
	
	// Render
	RenderShader(context,indexCount);

	return true;
}

ShaderDescription BuoyancyShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vOrthotexture.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "TextureVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pBuoyancy.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "BuoyancyPixelShader";

	shaderDescription.polygonLayout = CreateCommonInputLayout();

	shaderDescription.numLayoutElements = 2;

	return shaderDescription;
}

bool BuoyancyShader::SpecificInitialization(ID3D11Device* device) {
	D3D11_BUFFER_DESC inputBufferDesc;
	// Setup the description of the dynamic screen size constant buffer that is in the vertex shader.
	inputBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	inputBufferDesc.ByteWidth = sizeof(InputBuffer);
	inputBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	inputBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	inputBufferDesc.MiscFlags = 0;
	inputBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	HRESULT result = device->CreateBuffer(&inputBufferDesc, NULL, &mInputBuffer);
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
///////BUOYANCY SHADER END////////