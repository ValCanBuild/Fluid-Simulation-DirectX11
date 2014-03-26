/*************************************************************
SmokeRenderShader.h: Implementation of a shader that renders
a 3d smoke effect using raymarching

Author: Valentin Hinov
Date: 19/02/2014
**************************************************************/
#ifndef _SMOKERENDERSHADER_H
#define _SMOKERENDERSHADER_H

#include "BaseD3DShader.h"

class D3DGraphicsObject;
class Transform;

struct SmokeProperties {
	Color vSmokeColor;
	float fSmokeAbsorption;
	float fFireAbsorption;
	int iNumSamples;
	
	SmokeProperties();
	SmokeProperties(Color color, float smokeAbsorption, float fireAbsorption, int numSamples) : 
		vSmokeColor(color), fSmokeAbsorption(smokeAbsorption), fFireAbsorption(fireAbsorption), iNumSamples(numSamples) {}
};

class SmokeRenderShader : public BaseD3DShader {
public:
	SmokeRenderShader(const D3DGraphicsObject * const d3dGraphicsObject);
	~SmokeRenderShader();

	void SetVertexBufferValues(const Matrix &wvpMatrix, const Matrix &worldMatrix) const;
	void SetTransform(const Transform &transform) const;
	void SetCameraPosition(const Vector3 &camPos) const;
	void SetSmokeProperties(const SmokeProperties &smokeProperties) const;

	void SetVolumeValuesTexture(ID3D11ShaderResourceView *volumeValues);

protected:
	void BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) override;

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);
private:
	const D3DGraphicsObject * pD3dGraphicsObject;

	struct VertexInputBuffer {
		Matrix wvpMatrix;
		Matrix worldMatrix;
	};

	struct PixelBufferPerFrame {
		Vector3 vEyePos;	
		float  padding1;	
	};

	struct PixelBufferPerObject {
		Vector3 vTranslate; 
		float  padding2;	

		Vector3 vScale;
		float  padding3;	
	};

	struct PixelSmokePropertiesBuffer {
		SmokeProperties smokeProperties;
		float padding;
	};

	CComPtr<ID3D11Buffer>		mVertexInputBuffer;
	CComPtr<ID3D11Buffer>		mPixelBufferPerFrame;
	CComPtr<ID3D11Buffer>		mPixelBufferPerObject;
	CComPtr<ID3D11Buffer>		mPixelSmokePropertiesBuffer;

	ID3D11ShaderResourceView *  pVolumeValuesTexture;
};

#endif