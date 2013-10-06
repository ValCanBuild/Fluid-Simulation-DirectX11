/*************************************************************
BaseD3DShader.cpp: Semi-Abstract class that describes a Direct3D
shader object. This parent class is in charge of the main 
initialization and rendering as well as some utility functions.

Author: Valentin Hinov
Date: 03/09/2013
Version: 1.0
**************************************************************/
#include <d3dx11async.h>
#include <fstream>

#include "BaseD3DShader.h"

using namespace std;

bool BaseD3DShader::Initialize (ID3D11Device* device, HWND hwnd) {
	ShaderDescription shaderDesc = GetShaderDescription();	

	ID3D10Blob* vertexShaderBuffer = 0;
	ID3D10Blob* errorMessage = 0;

	// Create vertex shader
	HRESULT result = D3DX11CompileFromFile(shaderDesc.vertexShaderDesc.shaderFilename, NULL, NULL, shaderDesc.vertexShaderDesc.shaderFunctionName, "vs_5_0", 
								D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &vertexShaderBuffer, &errorMessage, NULL);

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
	result = D3DX11CompileFromFile(shaderDesc.pixelShaderDesc.shaderFilename, NULL, NULL, shaderDesc.pixelShaderDesc.shaderFunctionName, "ps_5_0", 
								D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, &pixelShaderBuffer, &errorMessage, NULL);

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
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &(mVertexShader._Myptr));
	if(FAILED(result)) {
		return false;
	}

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &(mPixelShader._Myptr));
	if(FAILED(result)) {
		return false;
	}

	// Create polygon layout
	result = device->CreateInputLayout(shaderDesc.polygonLayout,shaderDesc.numLayoutElements, 
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &(mLayout._Myptr));

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
	context->IASetInputLayout(mLayout.get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	context->VSSetShader(mVertexShader.get(), NULL, 0);
	context->PSSetShader(mPixelShader.get(), NULL, 0);

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