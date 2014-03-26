/*************************************************************
FireRenderShader.h: Implementation of a shader that renders
a 3d volume fire effect. Shares all of the smoke render
shader functionality.

Author: Valentin Hinov
Date: 26/03/2014
**************************************************************/
#ifndef _FIRERENDERSHADER_H
#define _FIRERENDERSHADER_H

#include "SmokeRenderShader.h"

class D3DGraphicsObject;
class Transform;

class FireRenderShader : public SmokeRenderShader {
public:
	FireRenderShader(const D3DGraphicsObject * const d3dGraphicsObject);
	~FireRenderShader();
	
	void SetReactionValuesTexture(ID3D11ShaderResourceView *reactionValues);
	void SetFireGradientTexture(ID3D11ShaderResourceView *fireGradient);

private:
	ShaderDescription GetShaderDescription() override;
	void BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) override;

	ID3D11ShaderResourceView *  pReactionValuesTexture;
	ID3D11ShaderResourceView *  pFireGradient;
};

#endif