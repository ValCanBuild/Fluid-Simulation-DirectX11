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
using namespace DirectX;

bool BaseD3DShader::Initialize (ID3D11Device* device, HWND hwnd) {
	ShaderDescription shaderDesc = GetShaderDescription();	
	ID3D10Blob* errorMessage = 0;

	int flags = 0;
#if defined (_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	flags = D3D10_SHADER_ENABLE_STRICTNESS;
#endif

	HRESULT result;

	// Create vertex shader if one exists
	if (shaderDesc.vertexShaderDesc.shaderFilename) {
		CComPtr<ID3D10Blob> vertexShaderBuffer;

		result = D3DCompileFromFile(shaderDesc.vertexShaderDesc.shaderFilename,nullptr,nullptr,shaderDesc.vertexShaderDesc.shaderFunctionName, "vs_5_0",
											flags, 0, &vertexShaderBuffer, &errorMessage);

		if(FAILED(result)) {
			// If the shader failed to compile it should have writen something to the error message.
			if(errorMessage) {
				OutputShaderErrorMessage(errorMessage, hwnd, shaderDesc.vertexShaderDesc.shaderFilename);
			}
			// If there was  nothing in the error message then it simply could not find the file itself.
			else {
				MessageBox(hwnd, shaderDesc.vertexShaderDesc.shaderFilename, L"Wrong Vertex Shader File", MB_OK);
			}

			return false;
		}
		// Create the vertex shader from the buffer.
		result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &mVertexShader);
		if(FAILED(result)) {
			return false;
		}

		// Create polygon layout
		result = device->CreateInputLayout(shaderDesc.polygonLayout,shaderDesc.numLayoutElements, 
			vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &mLayout);

		if (FAILED(result)) {
			return false;
		}
	}

	// Create pixel shader if one exists
	if (shaderDesc.pixelShaderDesc.shaderFilename) {
		CComPtr<ID3D10Blob> pixelShaderBuffer;

		result = D3DCompileFromFile(shaderDesc.pixelShaderDesc.shaderFilename,nullptr,nullptr,shaderDesc.pixelShaderDesc.shaderFunctionName, "ps_5_0",
											flags, 0, &pixelShaderBuffer, &errorMessage);

		if(FAILED(result)) {
			// If the shader failed to compile it should have writen something to the error message.
			if(errorMessage) {
				OutputShaderErrorMessage(errorMessage, hwnd, shaderDesc.pixelShaderDesc.shaderFilename);
			}
			// If there was  nothing in the error message then it simply could not find the file itself.
			else {
				MessageBox(hwnd, shaderDesc.pixelShaderDesc.shaderFilename, L"Wrong Pixel Shader File", MB_OK);
			}

			return false;
		}
		// Create the pixel shader from the buffer.
		result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mPixelShader);
		if(FAILED(result)) {
			return false;
		}
	}

	// Create compute shader if one exists
	if (shaderDesc.computeShaderDesc.shaderFilename) {
		CComPtr<ID3D10Blob> computeShaderBuffer;
		
		result = D3DCompileFromFile(shaderDesc.computeShaderDesc.shaderFilename,nullptr,nullptr,shaderDesc.computeShaderDesc.shaderFunctionName, "cs_5_0",
											flags, 0, &computeShaderBuffer, &errorMessage);

		if(FAILED(result)) {
			// If the shader failed to compile it should have written something to the error message.
			if(errorMessage) {
				OutputShaderErrorMessage(errorMessage, hwnd, shaderDesc.computeShaderDesc.shaderFilename);
			}
			// If there was  nothing in the error message then it simply could not find the file itself.
			else {
				MessageBox(hwnd, shaderDesc.computeShaderDesc.shaderFilename, L"Wrong Compute Shader File", MB_OK);
			}

			return false;
		}
		// Create the compute shader from the buffer.
		result = device->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), NULL, &mComputeShader);
		if(FAILED(result)) {
			return false;
		}
	}

	// Perform any child-specific initializations
	SpecificInitialization(device);

	return true;
}

void BaseD3DShader::RenderShader(ID3D11DeviceContext* context, int indexCount) {
	// Set the vertex input layout.
	context->IASetInputLayout(mLayout);

	Apply(context);

	// Render the triangle.
	context->DrawIndexed(indexCount, 0, 0);
}

void BaseD3DShader::Apply(_In_ ID3D11DeviceContext* deviceContext) {
	// Set the vertex and pixel shaders that will be used to render
	deviceContext->VSSetShader(mVertexShader, NULL, 0);
	deviceContext->PSSetShader(mPixelShader, NULL, 0);
}

void BaseD3DShader::SetComputeShader(ID3D11DeviceContext* context) const {
	context->CSSetShader(mComputeShader,NULL,0);
}

void BaseD3DShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename) const {
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

void BaseD3DShader::GetVertexShaderBytecode(_Out_ void const** pShaderByteCode, _Out_ size_t* pByteCodeLength) {

}

ID3D11InputLayout* BaseD3DShader::GetInputLayout() const {
	return mLayout;
}
