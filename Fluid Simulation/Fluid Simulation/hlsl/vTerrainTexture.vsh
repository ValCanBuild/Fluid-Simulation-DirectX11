
cbuffer MatrixBuffer : register (b0) {
	float4x4 worldMatrix;
	float4x4 wvpMatrix;
};

struct VertexInputType {
	float3 position : SV_Position;
	float3 normal : NORMAL;
	float2 texC : TEXCOORD;
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float3 positionW : POSITION;
	float2 texC : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType TerrainTextureVertexShader(VertexInputType input) {
	PixelInputType output;
	
	// Change the position vector to be 4 units for proper matrix calculations.

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(float4(input.position, 1.0f), wvpMatrix);
	output.positionW = mul(input.position, worldMatrix);
	
	// Store the input color for the pixel shader to use.
	output.texC = input.texC;
	
	return output;
}