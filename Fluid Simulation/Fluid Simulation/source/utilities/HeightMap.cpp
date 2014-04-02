/********************************************************************
HeightMap.cpp: Implementation of heightmap

Author:	Valentin Hinov
Date: 24/3/2014
*********************************************************************/
#include "HeightMap.h"

//Smooth out each vertex depending on the nearest vertices to it using the float as the smoothing factor on how much to decrease/increase height
void HeightMap::SmoothHeights(float factor) {
	maxHeight = 0;//reset max height 
	float k = factor;
	for (int i = 1; i < terrainWidth; i++){
		for (int j = 0; j < terrainDepth; j++){
			int index = j*terrainWidth + i;
			heights[index] = heights[j*terrainWidth + i - 1]*(1-k) + heights[index] * k;
			if (heights[index] > maxHeight){
				maxHeight = heights[index];
			}
		}
	}

	for (int i = terrainWidth-2; i < -1; i--){
		for (int j = 0; j < terrainDepth; j++){
			int index = j*terrainWidth + i;
			heights[index] = heights[j*terrainWidth + i + 1]*(1-k) + heights[index] * k;
			if (heights[index] > maxHeight){
				maxHeight = heights[index];
			}
		}
	}

	for (int i = 0; i < terrainWidth; i++){
		for (int j = 1; j < terrainDepth; j++){
			int index = j*terrainWidth + i;
			heights[index] = heights[(j-1)*terrainWidth + i]*(1-k) + heights[index] * k;
			if (heights[index] > maxHeight){
				maxHeight = heights[index];
			}
		}
	}

	for (int i = 0; i < terrainWidth; i++){
		for (int j = terrainDepth; j < -1; j--){
			int index = j*terrainWidth + i;
			heights[index] = heights[(j+1)*terrainWidth + i]*(1-k) + heights[index] * k;
			if (heights[index] > maxHeight){
				maxHeight = heights[index];
			}
		}
	}
}

void HeightMap::ScaleHeights(float factor) {
	for (float &height : heights) {
		height *= factor;
	}
	maxHeight *= factor;
}
