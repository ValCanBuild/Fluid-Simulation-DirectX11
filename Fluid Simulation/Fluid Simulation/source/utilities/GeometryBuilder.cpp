#include "GeometryBuilder.h"
#include "../display/D3DRenderer.h"
#include <VertexTypes.h>
#include <vector>

using namespace DirectX;

struct VertexColor {
	Vector3 pos;
	Vector4 color;

	VertexColor(Vector3 pos, Vector4 color) : pos(pos), color(color) {
		
	}
};

bool BuildCubeNormalTexture(ID3D11Device* device, D3DRenderer *targetRenderer) {
	// Load the vertex array with data.
	std::vector<VertexColor> vertices;
	vertices.push_back(VertexColor(Vector3(-1.0f, -1.0f, 0.0f),	Vector4(1.0f))); // Bottom left.	
	vertices.push_back(VertexColor(Vector3(0.0f, 1.0f, 0.0f), Vector4(1.0f))); // Top Left
	vertices.push_back(VertexColor(Vector3(1.0f, -1.0f, 0.0f), Vector4(1.0f))); // Top Right
	
		//POS						   //COLOR					   
		//back
	//vertices.push_back(VertexPositionTexture(Vector3(-1.0f, -1.0f, -1.0f),	Vector2(0.0f, 0.0f))); // Bottom left.	
	//vertices.push_back(VertexPositionTexture(Vector3(-1.0f, +1.0f, -1.0f),  Vector2(0.0f, 1.0f))); // Top Left
	//vertices.push_back(VertexPositionTexture(Vector3(+1.0f, +1.0f, -1.0f),  Vector2(1.0f, 1.0f))); // Top Right
	//vertices.push_back(VertexPositionTexture(Vector3(+1.0f, -1.0f, -1.0f),  Vector2(1.0f, 0.0f))); // Bottom Right

		//front
	//vertices.push_back(VertexPositionTexture(Vector3(-1.0f, -1.0f, 1.0f), Vector2(0.0f, 0.0f))); // Bottom left.	
	//vertices.push_back(VertexPositionTexture(Vector3(-1.0f, +1.0f, 1.0f), Vector2(0.0f, 1.0f))); // Top Left
	//vertices.push_back(VertexPositionTexture(Vector3(+1.0f, +1.0f, 1.0f), Vector2(1.0f, 1.0f))); // Top Right
	//vertices.push_back(VertexPositionTexture(Vector3(+1.0f, -1.0f, 1.0f), Vector2(1.0f, 0.0f))); // Bottom Right
	

	DWORD indices[] ={
		// Back
	0, 1, 2,
	//2, 3, 0,
	//// Front
	//6, 5, 4,
	//4, 7, 6,
	//// Left
	//2, 7, 3,
	//2, 6, 7,
	//// Right
	//0, 4, 1,
	//5, 1, 4,
	//// Top
	//6, 2, 1, 
	//5, 6, 1,
	//
	//// Bottom
	//0, 3, 7,
	//0, 7, 4 
	};

	// Set the number of vertices in the vertex array.
	int vertexCount = vertices.size();
	// Set the number of indices in the index array.
	int indexCount = sizeof(indices)/sizeof(indices[0]);

	bool result = targetRenderer->InitializeBuffers(device,indices,vertices.data(),sizeof(VertexColor),vertexCount,indexCount);

	if (!result){
		return false;
	}
	return true;
}