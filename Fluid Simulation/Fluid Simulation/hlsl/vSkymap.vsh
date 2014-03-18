
cbuffer MatrixBuffer {
	matrix wvpMatrix;
	matrix worldMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType {
	float4 position : SV_Position;
	float3 normal : NORMAL; // unused
	float2 texC : TEXCOORD; // unused
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float3 texCoord : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType SkymapVertexShader(VertexInputType input) {
	PixelInputType output;
	
	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;
	
	//Set Pos to xyww instead of xyzw, so that z will always be 1 (furthest from camera)
	output.position = mul(input.position, wvpMatrix).xyww;

	output.texCoord = input.position.xyz;

	return output;
}