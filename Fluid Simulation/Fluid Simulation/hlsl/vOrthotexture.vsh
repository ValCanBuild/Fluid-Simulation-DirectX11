//////////////
// TYPEDEFS //
//////////////
struct VertexInputType {
	float4 position : POSITION;
	float2 texC : TEXCOORD0;
};

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 texC : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType TextureVertexShader(VertexInputType input) {
	PixelInputType output;
	
	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	output.position = input.position;
	
	// Store the input color for the pixel shader to use.
	output.texC = input.texC;
	
	return output;
}