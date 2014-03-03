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

class VolumeRenderShader : public BaseD3DShader {
public:
	VolumeRenderShader(const D3DGraphicsObject * const d3dGraphicsObject);
	~VolumeRenderShader();

	void BindShaderResources(_In_ ID3D11DeviceContext* deviceContext) override;

	void SetVertexBufferValues(Matrix &wvpMatrix, Matrix &worldMatrix) const;
	void SetPixelBufferValues(Transform &transform, Vector3 &vEyePos, Vector3 &vDimensions, ID3D11ShaderResourceView* volumeValues);
	void SetSmokeProperties(Color &color, float absorption, int numSamples) const;
	void ApplySamplers();

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

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

	struct PixelSmokeProperties {
		Color vSmokeColor;
		float fSmokeAbsorption;
		int iNumSamples;

		Vector2 padding;
	};

	CComPtr<ID3D11Buffer>		mVertexInputBuffer;
	CComPtr<ID3D11Buffer>		mPixelInputBuffer;
	CComPtr<ID3D11Buffer>		mPixelSmokePropertiesBuffer;
	CComPtr<ID3D11SamplerState> mSampleState;

	ID3D11ShaderResourceView *  pVolumeValuesTexture;
};

#endif