/*************************************************************
VolumeRenderShader.h: Implementation of a shader that renders
a 3d volume texture using raymarching

Author: Valentin Hinov
Date: 19/02/2014
**************************************************************/
#ifndef _VOLUMERENDERSHADER_H
#define _VOLUMERENDERSHADER_H

#include "BaseD3DShader.h"

class D3DGraphicsObject;
class Transform;

struct SmokeProperties {
	Color vSmokeColor;
	float fSmokeAbsorption;
	int iNumSamples;
	
	SmokeProperties();
	SmokeProperties(Color color, float absorption, int numSamples) : 
		vSmokeColor(color), fSmokeAbsorption(absorption), iNumSamples(numSamples) {}
};

class VolumeRenderShader : public BaseD3DShader {
public:
	VolumeRenderShader(const D3DGraphicsObject * const d3dGraphicsObject);
	~VolumeRenderShader();

	void BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) override;

	void SetVertexBufferValues(Matrix &wvpMatrix, Matrix &worldMatrix) const;
	void SetPixelBufferValues(Transform &transform, Vector3 &vEyePos, Vector3 &vDimensions);
	void SetSmokeProperties(SmokeProperties &smokeProperties) const;

	void SetVolumeValuesTexture(ID3D11ShaderResourceView *volumeValues);
	void ApplySamplers();

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);
	D3D11_INPUT_ELEMENT_DESC *GetInputElementDescription();
private:
	const D3DGraphicsObject * pD3dGraphicsObject;

	struct VertexInputBuffer {
		Matrix wvpMatrix;
		Matrix worldMatrix;
	};

	struct PixelInputBuffer {
		Vector3 vDimensions;			
		float  padding0;	

		Vector3 vEyePos;	
		float  padding1;	

		Vector3 vTranslate; 
		float  padding2;	

		Vector3 vScale;
		float  padding3;	
	};

	struct PixelSmokePropertiesBuffer {
		SmokeProperties smokeProperties;
		Vector2 padding;
	};

	CComPtr<ID3D11Buffer>		mVertexInputBuffer;
	CComPtr<ID3D11Buffer>		mPixelInputBuffer;
	CComPtr<ID3D11Buffer>		mPixelSmokePropertiesBuffer;
	CComPtr<ID3D11SamplerState> mSampleState;

	ID3D11ShaderResourceView *  pVolumeValuesTexture;
};

#endif