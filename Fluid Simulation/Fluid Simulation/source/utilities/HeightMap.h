/********************************************************************
HeightMap.h: Represents a height map structure to be used for a terrain

Author:	Valentin Hinov
Date: 24/3/2014
*********************************************************************/

#include <vector>

struct HeightMap {
	std::vector<float> heights;
	float maxHeight;
	int	terrainWidth;
	int	terrainDepth;

	~HeightMap() {
		heights.clear();
	}
	void ScaleHeights(float factor); // scale all heights on the heightmap by a factor
	void SmoothHeights(float factor);	// smooths out the heights in the terrain by the given factor
};