#include "OrthoTextureShader.h"


OrthoTextureShader::OrthoTextureShader(void) {
}


OrthoTextureShader::~OrthoTextureShader(void) {
}

bool OrthoTextureShader::Render(ID3D11DeviceContext* context, int indexCount, ID3D11ShaderResourceView* texture) {

	// Set the parameters inside the shader
	context->PSSetShaderResources(0,1,&texture);

	// Render
	RenderShader(context,indexCount);

	return true;
}

ShaderDescription OrthoTextureShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/orthotexture.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "TextureVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/orthotexture.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "TexturePixelShader";

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

bool OrthoTextureShader::SpecificInitialization(ID3D11Device* device) {
	
	return true;
}