/*************************************************************
BaseD3DShader.h: Semi-Abstract class that describes a Direct3D
shader object. This parent class is in charge of the main 
initialization and rendering as well as some utility functions.

Author: Valentin Hinov
Date: 03/09/2013
Version: 1.0
**************************************************************/

#ifndef _BASED3DSHADER_H
#define _BASED3DSHADER_H

#include <unordered_map>
#include <stdexcept>
#include <Effects.h>
#include "../../utilities/AtlInclude.h"

#include "../../utilities/D3dIncludes.h"

struct ShaderFileDescription {
	WCHAR* shaderFilename;
	char* shaderFunctionName;

	ShaderFileDescription() {
		shaderFilename = nullptr;
		shaderFunctionName = nullptr;
	}

	~ShaderFileDescription() {
		shaderFunctionName = nullptr;	
		shaderFilename = nullptr;
	}
};
/**
ShaderDescription is a struct every shader must provide in order to be
properly initialized.
**/
struct ShaderDescription {	
	ShaderFileDescription vertexShaderDesc;
	ShaderFileDescription pixelShaderDesc;
	ShaderFileDescription computeShaderDesc;

	D3D11_INPUT_ELEMENT_DESC *polygonLayout;
	int numLayoutElements;

	ShaderDescription() {
		polygonLayout = nullptr;
		numLayoutElements = 0;
	}
	~ShaderDescription() {
		polygonLayout = nullptr;
	}
};

class BaseD3DShader : public DirectX::IEffect {
public:
	~BaseD3DShader() {}

	bool Initialize (ID3D11Device* device, HWND hwnd);	

	// Child classes supply this function to bind any constant buffers, samplers, etc... before rendering
	virtual void BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) {};
	// IEffect methods.
	void Apply(_In_ ID3D11DeviceContext* deviceContext) override;

	void GetVertexShaderBytecode(_Out_ void const** pShaderByteCode, _Out_ size_t* pByteCodeLength) override;

	ID3D11InputLayout* GetInputLayout() const;

protected:
	BaseD3DShader() {} // only child classes can be constructed

	// This renders an object using the provided Pixel and Vertex Shaders given the index count
	void RenderShader(ID3D11DeviceContext* context, int indexCount);

	void SetComputeShader(ID3D11DeviceContext* context) const;

	// Every child of this class has to provide an implementation of these functions in order to get the correct shader name
	virtual ShaderDescription GetShaderDescription() = 0;
	// At the end of the Initialize function this function will be called in order to do any child-specific initialization
	virtual bool SpecificInitialization(ID3D11Device* device) {return true;};

private:
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename) const;

	bool CreatePixelShader(ShaderFileDescription &shaderFileDesc, int flags, HWND hwnd, ID3D11Device* device);
	bool CreateVertexShaderAndInputLayout(ShaderFileDescription &shaderFileDesc, int flags, HWND hwnd, ID3D11Device* device, D3D11_INPUT_ELEMENT_DESC *polygonLayout, int numElements);

	bool CreateComputeShader(ShaderFileDescription &shaderFileDesc, int flags, HWND hwnd, ID3D11Device* device);

private:
	CComPtr<ID3D11VertexShader>		mVertexShader;
	CComPtr<ID3D11PixelShader>		mPixelShader;
	CComPtr<ID3D11ComputeShader>	mComputeShader;
	CComPtr<ID3D11InputLayout>		mLayout;

private:
	struct ShaderFileDescriptionHash {
		std::size_t operator()(const ShaderFileDescription& k) const	{
			return std::hash<WCHAR*>()(k.shaderFilename) ^
				(std::hash<char*>()(k.shaderFunctionName) << 1);
		}
	};

	struct ShaderFileDescriptionEqual {
		bool operator()(const ShaderFileDescription& a, const ShaderFileDescription &b) const {
			int first = wcscmp(a.shaderFilename, b.shaderFilename);
			int second = strcmp(a.shaderFunctionName, b.shaderFunctionName);

			return (first == 0) && (second == 0);
		}
	};

	// Static map of used shaders
	static std::unordered_map<ShaderFileDescription, CComPtr<ID3D11PixelShader>, ShaderFileDescriptionHash, ShaderFileDescriptionEqual> pixelShaderMap;
	static std::unordered_map<ShaderFileDescription, CComPtr<ID3D11ComputeShader>, ShaderFileDescriptionHash, ShaderFileDescriptionEqual> computeShaderMap;

	struct VertexShaderData {
		CComPtr<ID3D11VertexShader> vertexShader;
		CComPtr<ID3D11InputLayout> inputLayout;
	};

	static std::unordered_map<ShaderFileDescription, VertexShaderData, ShaderFileDescriptionHash, ShaderFileDescriptionEqual> vertexShaderMap;
};

template<typename T>
bool BuildDynamicBuffer(_In_ ID3D11Device *device, _Out_ ID3D11Buffer **pOutBuffer) {
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(T);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	// General buffer
	HRESULT hresult = device->CreateBuffer(&bufferDesc, NULL, pOutBuffer);

	return !FAILED(hresult);
}

#endif