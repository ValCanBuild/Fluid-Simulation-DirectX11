/*************************************************************
BaseD3DShader.cpp: Semi-Abstract class that describes a Direct3D
shader object. This parent class is in charge of the main 
initialization and rendering as well as some utility functions.

Author: Valentin Hinov
Date: 03/09/2013
Version: 1.0
**************************************************************/
#include "BaseD3DShader.h"

#include <fstream>
#include <d3dcompiler.h>

#pragma comment(lib, "D3DCompiler.lib")

using namespace std;
using namespace DirectX;

unordered_map<ShaderFileDescription, CComPtr<ID3D11PixelShader>, BaseD3DShader::ShaderFileDescriptionHash, BaseD3DShader::ShaderFileDescriptionEqual> BaseD3DShader::pixelShaderMap;
unordered_map<ShaderFileDescription, CComPtr<ID3D11ComputeShader>, BaseD3DShader::ShaderFileDescriptionHash, BaseD3DShader::ShaderFileDescriptionEqual> BaseD3DShader::computeShaderMap;
unordered_map<ShaderFileDescription, BaseD3DShader::VertexShaderData, BaseD3DShader::ShaderFileDescriptionHash, BaseD3DShader::ShaderFileDescriptionEqual> BaseD3DShader::vertexShaderMap;


bool BaseD3DShader::Initialize (ID3D11Device* device, HWND hwnd) {
	ShaderDescription shaderDesc = GetShaderDescription();	

	int flags = 0;
#if defined (_DEBUG)
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	flags = D3D10_SHADER_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	bool result;

	// Create vertex shader and input layout if one exists
	if (shaderDesc.vertexShaderDesc.shaderFilename) {
		CComPtr<ID3D10Blob> vertexShaderBuffer;
		result = CreateVertexShaderAndInputLayout(shaderDesc.vertexShaderDesc, flags, hwnd, device, shaderDesc.polygonLayout, shaderDesc.numLayoutElements);
		if (!result) {
			return false;
		}
	}

	// Create pixel shader if one exists
	if (shaderDesc.pixelShaderDesc.shaderFilename) {
		result = CreatePixelShader(shaderDesc.pixelShaderDesc, flags, hwnd, device);
		if (!result) {
			return false;
		}
	}

	// Create compute shader if one exists
	if (shaderDesc.computeShaderDesc.shaderFilename) {
		result = CreateComputeShader(shaderDesc.computeShaderDesc, flags, hwnd, device);
		if (!result) {
			return false;
		}
	}

	// Perform any child-specific initializations
	SpecificInitialization(device);

	// Clean up
	if (shaderDesc.polygonLayout) {
		delete [] shaderDesc.polygonLayout;
		shaderDesc.polygonLayout = nullptr;
	}

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
	// Allow the shader to bind any buffers it may need
	BindShaderResources(deviceContext);
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

bool BaseD3DShader::CreateVertexShaderAndInputLayout(ShaderFileDescription &shaderFileDesc, int flags, HWND hwnd, ID3D11Device* device, D3D11_INPUT_ELEMENT_DESC *polygonLayout, int numElements) {
	// first check if shader with this description exists in the vertex shader map
	if (vertexShaderMap.count(shaderFileDesc) == 1) {
		VertexShaderData shaderData = vertexShaderMap[shaderFileDesc];
		mVertexShader = shaderData.vertexShader;
		mLayout = shaderData.inputLayout;
		return true;
	}
	
	ID3D10Blob* errorMessage = nullptr;
	ID3D10Blob* vertexShaderBuffer;

	HRESULT result = D3DCompileFromFile(shaderFileDesc.shaderFilename,nullptr,nullptr,shaderFileDesc.shaderFunctionName, "vs_5_0",
		flags, 0, &vertexShaderBuffer, &errorMessage);

	if(FAILED(result)) {
		// If the shader failed to compile it should have written something to the error message.
		if(errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, shaderFileDesc.shaderFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		else {
			MessageBox(hwnd, shaderFileDesc.shaderFilename, L"Wrong Vertex Shader File", MB_OK);
		}

		return false;
	}
	
	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &mVertexShader);
	if(FAILED(result)) {
		return false;
	}

	// Create the polygon layout
	result = device->CreateInputLayout(polygonLayout, numElements, 
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &mLayout);

	if(FAILED(result)) {
		return false;
	}

	// Add to map
	vertexShaderMap[shaderFileDesc].vertexShader = mVertexShader;
	vertexShaderMap[shaderFileDesc].inputLayout = mLayout;

	return true;
}

bool BaseD3DShader::CreatePixelShader(ShaderFileDescription &shaderFileDesc, int flags, HWND hwnd, ID3D11Device* device) {
	// first check if shader with this description exists in the pixel shader map
	if (pixelShaderMap.count(shaderFileDesc) == 1) {
		mPixelShader = pixelShaderMap[shaderFileDesc];
		return true;
	}
	
	ID3D10Blob* errorMessage = nullptr;
	CComPtr<ID3D10Blob> pixelShaderBuffer;

	HRESULT result = D3DCompileFromFile(shaderFileDesc.shaderFilename,nullptr,nullptr,shaderFileDesc.shaderFunctionName, "ps_5_0",
		flags, 0, &pixelShaderBuffer, &errorMessage);

	if(FAILED(result)) {
		// If the shader failed to compile it should have written something to the error message.
		if(errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, shaderFileDesc.shaderFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		else {
			MessageBox(hwnd, shaderFileDesc.shaderFilename, L"Wrong Pixel Shader File", MB_OK);
		}

		return false;
	}
	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &mPixelShader);
	if(FAILED(result)) {
		return false;
	}

	// Add to pixel shader map
	pixelShaderMap[shaderFileDesc] = mPixelShader;

	return true;
}

bool BaseD3DShader::CreateComputeShader(ShaderFileDescription &shaderFileDesc, int flags, HWND hwnd, ID3D11Device* device) {
	// first check if shader with this description exists in the compute shader map
	if (computeShaderMap.count(shaderFileDesc) == 1) {
		mComputeShader = computeShaderMap[shaderFileDesc];
		return true;
	}
	
	CComPtr<ID3D10Blob> computeShaderBuffer;
	ID3D10Blob* errorMessage = nullptr;

	HRESULT result = D3DCompileFromFile(shaderFileDesc.shaderFilename,nullptr,nullptr,shaderFileDesc.shaderFunctionName, "cs_5_0",
		flags, 0, &computeShaderBuffer, &errorMessage);

	if(FAILED(result)) {
		// If the shader failed to compile it should have written something to the error message.
		if(errorMessage) {
			OutputShaderErrorMessage(errorMessage, hwnd, shaderFileDesc.shaderFilename);
		}
		// If there was  nothing in the error message then it simply could not find the file itself.
		else {
			MessageBox(hwnd, shaderFileDesc.shaderFilename, L"Wrong Compute Shader File", MB_OK);
		}

		return false;
	}

	// Create the compute shader from the buffer.
	result = device->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), NULL, &mComputeShader);
	if(FAILED(result)) {
		return false;
	}

	computeShaderMap[shaderFileDesc] = mComputeShader;

	return true;
}

void BaseD3DShader::GetVertexShaderBytecode(_Out_ void const** pShaderByteCode, _Out_ size_t* pByteCodeLength) {
	throw std::runtime_error(std::string("BaseD3DShader: GetVertexShaderBytecode called and it has no effect"));
}

ID3D11InputLayout* BaseD3DShader::GetInputLayout() const {
	return mLayout;
}