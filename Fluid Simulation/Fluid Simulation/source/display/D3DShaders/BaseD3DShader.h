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

#include <stdexcept>
#include <Effects.h>
#include "../../utilities/AtlInclude.h"

#include "../../utilities/D3dIncludes.h"

/**
ShaderDescription is a struct every shader must provide in order to be
properly initialized.
**/
struct ShaderDescription {
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

	ShaderFileDescription vertexShaderDesc;
	ShaderFileDescription pixelShaderDesc;
	ShaderFileDescription computeShaderDesc;

	D3D11_INPUT_ELEMENT_DESC *polygonLayout;
	int numLayoutElements;

	ShaderDescription() {
		polygonLayout = nullptr;
		numLayoutElements = 0;
	}
};

class BaseD3DShader : public DirectX::IEffect {
public:
	BaseD3DShader(){}
	~BaseD3DShader(){}

	bool Initialize (ID3D11Device* device, HWND hwnd);	

	// Child classes supply this function to bind any constant buffers, samplers, etc... before rendering
	virtual void BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) {};
	// IEffect methods.
	void Apply(_In_ ID3D11DeviceContext* deviceContext) override;

	void GetVertexShaderBytecode(_Out_ void const** pShaderByteCode, _Out_ size_t* pByteCodeLength) override;

	ID3D11InputLayout* GetInputLayout() const;

private:
	void OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename) const;

protected:
	// This renders an object using the provided Pixel and Vertex Shaders given the index count
	void RenderShader(ID3D11DeviceContext* context, int indexCount);

	void SetComputeShader(ID3D11DeviceContext* context) const;

	// Every child of this class has to provide an implementation of these functions in order to get the correct shader name
	virtual ShaderDescription GetShaderDescription() = 0;
	// At the end of the Initialize function this function will be called in order to do any child-specific initialization
	virtual bool SpecificInitialization(ID3D11Device* device) = 0;
private:
	CComPtr<ID3D11VertexShader>		mVertexShader;
	CComPtr<ID3D11PixelShader>		mPixelShader;
	CComPtr<ID3D11ComputeShader>	mComputeShader;
	CComPtr<ID3D11InputLayout>		mLayout;
};

#endif