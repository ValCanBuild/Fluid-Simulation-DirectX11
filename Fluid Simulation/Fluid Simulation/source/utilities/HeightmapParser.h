/********************************************************************
HeightmapParser.h: Parses a terrain heightmap and returns the height
data

Author:	Valentin Hinov
Date: 24/3/2014
*********************************************************************/

#ifndef	_HEIGHTMAPPARSER_H
#define	_HEIGHTMAPPARSER_H

#include <stdio.h>
#include "HeightMap.h"

class HeightmapParser {
public:
	static HeightMap GenerateFromTGA(char* filename);

private:
	HeightmapParser(); // only static methods may be used

private:
	static FILE	*LoadTGA(char* filename);		// loads in terrain heightmap to a file returns a pointer to the file
	static bool	ComputeHeightMap(_In_ FILE * fTGA, _Out_ HeightMap &heightMap);	// computes the heights from the TGA file
	static void Reset(HeightMap &heightMap);
};

#endif