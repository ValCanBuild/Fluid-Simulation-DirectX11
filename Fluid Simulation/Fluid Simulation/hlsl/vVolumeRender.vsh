
cbuffer MatrixBuffer {
	matrix wvpMatrix;
	matrix worldMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType {
	float4 position : POSITION;
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float3 worldPosition: TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType VolumeRenderVertexShader(VertexInputType input) {
	PixelInputType output;
	
	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;
	
	// calculate against world matrix to perform any movement/scaling
	output.position = mul(input.position, wvpMatrix);

	output.worldPosition = mul(output.position, worldMatrix).xyz;

	return output;
}