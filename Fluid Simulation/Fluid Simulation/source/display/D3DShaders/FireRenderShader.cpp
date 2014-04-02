/*************************************************************
FireRenderShader.cpp: Implementation of the fire render
shader

Author: Valentin Hinov
Date: 26/03/2014
**************************************************************/

#include "FireRenderShader.h"
#include <VertexTypes.h>
#include "../D3DGraphicsObject.h"
#include "../../objects/Transform.h"

FireRenderShader::FireRenderShader(const D3DGraphicsObject * const d3dGraphicsObject) : 
	SmokeRenderShader(d3dGraphicsObject), pReactionValuesTexture(nullptr), pFireGradient(nullptr)
{
}

FireRenderShader::~FireRenderShader() {
	pReactionValuesTexture = nullptr;
	pFireGradient = nullptr;
}

ShaderDescription FireRenderShader::GetShaderDescription() {
	ShaderDescription shaderDescription;

	shaderDescription.vertexShaderDesc.shaderFilename = L"hlsl/vVolumeRender.vsh";
	shaderDescription.vertexShaderDesc.shaderFunctionName = "VolumeRenderVertexShader";

	shaderDescription.pixelShaderDesc.shaderFilename = L"hlsl/pVolumeRender.psh";
	shaderDescription.pixelShaderDesc.shaderFunctionName = "FireVolumeRenderPixelShader";

	shaderDescription.numLayoutElements = DirectX::VertexPositionNormalTexture::InputElementCount;
	shaderDescription.polygonLayout = new D3D11_INPUT_ELEMENT_DESC[shaderDescription.numLayoutElements];

	for (int i = 0; i < shaderDescription.numLayoutElements; ++i) {
		shaderDescription.polygonLayout[i] = DirectX::VertexPositionNormalTexture::InputElements[i];
	}

	return shaderDescription;
}

void FireRenderShader::BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) {
	SmokeRenderShader::BindShaderResources(deviceContext);
	ID3D11ShaderResourceView *const pSRVs[2] = {pReactionValuesTexture, pFireGradient};
	deviceContext->PSSetShaderResources(1, 2, pSRVs);
}

void FireRenderShader::SetReactionValuesTexture(ID3D11ShaderResourceView *reactionValues) {
	pReactionValuesTexture = reactionValues;
}

void FireRenderShader::SetFireGradientTexture(ID3D11ShaderResourceView *fireGradient) {
	pFireGradient = fireGradient;
}
