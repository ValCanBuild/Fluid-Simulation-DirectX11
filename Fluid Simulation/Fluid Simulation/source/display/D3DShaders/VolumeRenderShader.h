/*************************************************************
VolumeRenderShader.h: Implementation of the volume render
shader

Author: Valentin Hinov
Date: 19/02/2014
**************************************************************/
#ifndef _VOLUMERENDERSHADER_H
#define _VOLUMERENDERSHADER_H

#include "BaseD3DShader.h"

class D3DGraphicsObject;
class Camera;

class VolumeRenderShader : public BaseD3DShader {
public:
	VolumeRenderShader(const D3DGraphicsObject * const d3dGraphicsObject);
	~VolumeRenderShader();

	void SetDynamicBufferValues(Vector3 &position, const Camera *pCamera, float zoom, Vector3& dimensions);
	void Compute(_In_ ID3D11ShaderResourceView* targetToRender, _In_ ID3D11UnorderedAccessView* result);

private:
	ShaderDescription GetShaderDescription();
	bool SpecificInitialization(ID3D11Device* device);

private:
	const D3DGraphicsObject * pD3dGraphicsObject;

	CComPtr<ID3D11SamplerState> mSampleState;

	struct InputBuffer {
		Vector3 vDimensions;	
		float fZoom;					

		Vector3 vWorldPos;				
		float  padding0;

		Vector3 vEyePos;	
		float   padding1;
		UINT  vViewportDimensions[2];	

		Vector2 padding2;
	};

	CComPtr<ID3D11Buffer>		mInputBuffer;
};

#endif