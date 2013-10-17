
cbuffer InputBuffer : register (b0) {
	float3 mousePos;
	int mousePressed;
};

///////////////
// CONSTANTS //
///////////////
static const float fWaveSpeed = 3.0f;
static const float fTimeStep = 0.07f;

//Resources
Texture2D<float> InputTexNow : register( t0 );
Texture2D<float> InputTexPrev : register( t1 );
RWTexture2D<float> OutputTex : register( u0 );

[numthreads(32,32,1)]
void WaveComputeShader(uint3 dispatchThreadId : SV_DispatchThreadID) {
	uint i = dispatchThreadId.x;
	uint j = dispatchThreadId.y;

	float u = InputTexNow.Load(int3(i,j,0));
	float uPrev = InputTexPrev.Load(int3(i,j,0));

	float col1 = InputTexNow.Load(int3(i+1,j,0)); //i+1, 0
	float col2 = InputTexNow.Load(int3(i-1,j,0)); //i-1, 0
	float col3 = InputTexNow.Load(int3(i,j+1,0)); //0, j+1
	float col4 = InputTexNow.Load(int3(i,j-1,0)); //0, j-1

	float ctx2 = pow(fWaveSpeed*fTimeStep,2);
	float final = (2.0f*u) - uPrev + ( col1+col2+col3+col4 - (4.0f*u) )*ctx2;
	if (mousePressed != 0) {
		if (distance(mousePos.x,i) < 5 && distance(mousePos.y,j) < 5) {
			final = mousePos.z;
		}
	}
	
	OutputTex[dispatchThreadId.xy] = final;
}