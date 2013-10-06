#include "OrthoColorShader.h"


OrthoColorShader::OrthoColorShader(void) {
}


OrthoColorShader::~OrthoColorShader(void) {
}

bool OrthoColorShader::Render(ID3D11DeviceContext* context, int indexCount, D3DXMATRIX *worldMatrix) {
	// setup matrix buffer
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber = 0;

	// Lock the constant buffer so it can be written to.
	HRESULT result = context->Map(mMatrixBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result)) {
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Transpose the matrix to prepare them for the shader.
	D3DXMatrixTranspose(&dataPtr->world, worldMatrix);

	// Unlock the constant buffer.
	context->Unmap(mMatrixBuffer.get(), 0);

	// Finally set the constant buffer in the vertex shader with the updated values.
	context->VSSetConstantBuffers(bufferNumber, 1, &mMatrixBuffer._Myptr);

	// Render
	RenderShader(context,indexCount);

	return true;
}

ShaderDescription OrthoColorShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/orthocolor.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "ColorVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/orthocolor.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "ColorPixelShader";

	shaderDescription.polygonLayout = new D3D11_INPUT_ELEMENT_DESC[2];

	shaderDescription.polygonLayout[0].SemanticName = "POSITION";
	shaderDescription.polygonLayout[0].SemanticIndex = 0;
	shaderDescription.polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	shaderDescription.polygonLayout[0].InputSlot = 0;
	shaderDescription.polygonLayout[0].AlignedByteOffset = 0;
	shaderDescription.polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[0].InstanceDataStepRate = 0;

	shaderDescription.polygonLayout[1].SemanticName = "COLOR";
	shaderDescription.polygonLayout[1].SemanticIndex = 0;
	shaderDescription.polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	shaderDescription.polygonLayout[1].InputSlot = 0;
	shaderDescription.polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	shaderDescription.polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	shaderDescription.polygonLayout[1].InstanceDataStepRate = 0;

	shaderDescription.numLayoutElements = 2;

	return shaderDescription;
}

bool OrthoColorShader::SpecificInitialization(ID3D11Device* device) {
	// setup the world matrix
	D3D11_BUFFER_DESC matrixBufferDesc;
	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	bool result = device->CreateBuffer(&matrixBufferDesc, NULL, &mMatrixBuffer._Myptr);
	if(FAILED(result)) {
		return false;
	}
	return true;
}