
cbuffer ScreenSizeBuffer {
    float screenWidth;
	float screenHeight;
    float2 padding;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType {
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType {
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float2 texCoord1 : TEXCOORD1;
	float2 texCoord2 : TEXCOORD2;
	float2 texCoord3 : TEXCOORD3;
	float2 texCoord4 : TEXCOORD4;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType WaveVertexShader(VertexInputType input) {
    PixelInputType output;

    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	output.position = input.position;
    
    // Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

    // Determine the floating point size of a texel for a screen with this specific width.
    float texelSizeU = 1.0f / screenWidth;
	float texelSizeV = 1.0f / screenHeight;

    // Create UV coordinates for the pixel and its four horizontal neighbors on either side.
    output.texCoord1 = input.tex + float2(texelSizeU, 0.0f);	//i+1, 0
    output.texCoord2 = input.tex - float2(texelSizeU, 0.0f);	//i-1, 0
	output.texCoord3 = input.tex + float2(0.0f, texelSizeV);	//0, j+1
    output.texCoord4 = input.tex - float2(0.0f, texelSizeV);	//0, j-1


    return output;
}