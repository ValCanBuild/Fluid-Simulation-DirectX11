/*************************************************************
BaseD3DShader.cpp: Semi-Abstract class that describes a Direct3D
shader object. This parent class is in charge of the main 
initialization and rendering as well as some utility functions.

Author: Valentin Hinov
Date: 03/09/2013
Version: 1.0
**************************************************************/
#include <fstream>
#include <d3dcompiler.h>
#include "BaseD3DShader.h"

#pragma comment(lib, "D3DCompiler.lib")

using namespace std;

bool BaseD3DShader::Initialize (ID3D11Device* device, HWND hwnd) {
	ShaderDescription shaderDesc = GetShaderDescription();	

	ID3D10Blob* vertexShaderBuffer = 0;
	ID3D10Blob* errorMessage = 0;

	int flags = 0;
#if defined (_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	flags = D3D10_SHADER_ENABLE_STRICTNESS;
#endif

	// Create vertex shader
	HRESULT result = D3DCompileFromFile(shaderDesc.vertexShaderDesc.shaderFilename,nullptr,nullptr,shaderDesc.vertexShaderDesc.shaderFunctionName, "vs_5_0",
										flags, 0, &vertexShaderBuffer, &errorMessage);

	if(FAILED(result)) {
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, shaderDesc.vertexShaderDesc.shaderFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		else {
			MessageBox(hwnd, shaderDesc.vertexShaderDesc.shaderFilename, L"Missing Vertex Shader File", MB_OK);
		}

		return false;
	}

	ID3D10Blob* pixelShaderBuffer = 0;

	// Create pixel shader
	result = D3DCompileFromFile(shaderDesc.pixelShaderDesc.shaderFilename,nullptr,nullptr,shaderDesc.pixelShaderDesc.shaderFunctionName, "ps_5_0",
										flags, 0, &pixelShaderBuffer, &errorMessage);

	if(FAILED(result)) {
		// If the shader failed to compile it should have writen something to the error message.
		if(errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, shaderDesc.pixelShaderDesc.shaderFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		else {
			MessageBox(hwnd, shaderDesc.pixelShaderDesc.shaderFilename, L"Missing Pixel Shader File", MB_OK);
		}

		return false;
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &mVertexShader);
	if(FAILED(result)) {
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mPixelShader);
	if(FAILED(result)) {
		return false;
	}

	/*ID3D11ShaderReflection* reflector = nullptr;
	D3DReflect(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void**) &reflector);

	ID3D11ShaderReflectionConstantBuffer* pBuffer = reflector->GetConstantBufferByIndex(0);

	D3D11_SHADER_BUFFER_DESC desc;
	pBuffer->GetDesc(&desc);

	reflector->Release();
	reflector = nullptr;*/
	
	// Create polygon layout
	result = device->CreateInputLayout(shaderDesc.polygonLayout,shaderDesc.numLayoutElements, 
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &mLayout);

	if (FAILED(result)) {
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	// Perform any child-specific initializations
	SpecificInitialization(device);

	return true;
}

void BaseD3DShader::RenderShader(ID3D11DeviceContext* context, int indexCount) {
	// Set the vertex input layout.
	context->IASetInputLayout(mLayout);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	context->VSSetShader(mVertexShader, NULL, 0);
	context->PSSetShader(mPixelShader, NULL, 0);

	// Render the triangle.
	context->DrawIndexed(indexCount, 0, 0);
}

void BaseD3DShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename) {
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;

	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for(i=0; i<bufferSize; i++) {
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
}