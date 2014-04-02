/********************************************************************
HeightmapParser.cpp: HeightmapParser implementation

Author:	Valentin Hinov
Date: 24/3/2014
*********************************************************************/

#include "HeightmapParser.h"
#include <Windows.h>
#include <assert.h>
#include <memory>
#include "TgaHeader.h"

const unsigned char UTGAcompare[12] = {0,0,2, 0,0,0,0,0,0,0,0,0};	// Uncompressed TGA Header

TGAHeader Tgaheader;

HeightMap HeightmapParser::GenerateFromTGA(char* filename){
	//store the heightmap to a file
	FILE * fTGA = LoadTGA(filename);
	HeightMap heightMap;
	//compute the mHeights from that file
	if (fTGA != nullptr) {
		if (!ComputeHeightMap(fTGA, heightMap)) {
			MessageBox(nullptr, L"Could not create heightmap", L"ERROR", MB_OK);	// Display an error message
		}
	}
	return heightMap;
}

FILE * HeightmapParser::LoadTGA(char* filename){
	FILE * fTGA;										// File pointer to texture file
	errno_t err = fopen_s(&fTGA, filename, "rb");						// Open file for reading
	if(err != 0){									// If it didn't open....	
		MessageBox(nullptr, L"Could not open texture file", L"ERROR", MB_OK);	// Display an error message
		return nullptr;														// Exit function
	}

	//we need to see what type of TGA we are loading. 
	if(fread(&Tgaheader, sizeof(TGAHeader), 1, fTGA) == 0){					// Attempt to read 12 byte header from file	
		MessageBox(nullptr, L"Could not read file header", L"ERROR", MB_OK);		// If it fails, display an error message 
		if(fTGA != nullptr){													// Check to see if file is still open		
			fclose(fTGA);													// If it is, close it
		}
		return nullptr;														// Exit function
	}

	if(memcmp(UTGAcompare, &Tgaheader, sizeof(Tgaheader)) == 0) {			// See if header matches the predefined header
		return fTGA;														// If so - return the file
	}
	else{																	// If header matches neither type	
		MessageBox(nullptr, L"TGA file be type 2 or type 10 ", L"Invalid Image", MB_OK);	// Display an error
		fclose(fTGA);
		return nullptr;																// Exit function
	}

	return nullptr;							
}

bool HeightmapParser::ComputeHeightMap(_In_ FILE * fTGA, _Out_ HeightMap &heightMap){
	Reset(heightMap);
	TGA tga;
	if(fread(tga.header, sizeof(tga.header), 1, fTGA) == 0){				// Read TGA header											
		MessageBox(nullptr, L"Could not read info header", L"ERROR", MB_OK);	// Display error if not read
		if(fTGA != nullptr){													// if file is still open		
			fclose(fTGA);													// Close it
		}
		return false;														// Return failure
	}	

	unsigned int terrainTexture_width	= tga.header[1] * 256 + tga.header[0];			// Determine The TGA Width	(highbyte*256+lowbyte)
	unsigned int terrainTexture_height	= tga.header[3] * 256 + tga.header[2];			// Determine The TGA Height	(highbyte*256+lowbyte)
	unsigned int terrainTexture_bpp		= tga.header[4];								// Determine the bits per pixel

	tga.width		= terrainTexture_width;										// Copy width into local structure						
	tga.height		= terrainTexture_height;									// Copy height into local structure
	tga.bpp			= terrainTexture_bpp;										// Copy BPP into local structure	

	if((terrainTexture_width <= 0) || (terrainTexture_height <= 0) || ((terrainTexture_bpp != 24) && (terrainTexture_bpp !=32)))	// Make sure all information is valid
	{
		MessageBox(nullptr, L"Invalid texture information", L"ERROR", MB_OK);	// Display Error
		if(fTGA != nullptr){													// Check if file is still open		
			fclose(fTGA);													// If so, close it
		}
		return false;														// Return failed
	}

	tga.bytesPerPixel	= (tga.bpp / 8);												// Compute the number of BYTES per pixel
	tga.imageSize		= (tga.bytesPerPixel * tga.width * tga.height);					// Compute the total amout of memory needed to store data
	std::unique_ptr<unsigned char[]> terrainTexture_imageData(new unsigned char[tga.imageSize]); // Allocate that much memory

	if(terrainTexture_imageData == nullptr){											// If no space was allocated	
		MessageBox(nullptr, L"Could not allocate memory for image", L"ERROR", MB_OK);	// Display Error
		fclose(fTGA);														// Close the file
		return false;														// Return failed
	}

	if(fread(terrainTexture_imageData.get(), 1, tga.imageSize, fTGA) != tga.imageSize){	// Attempt to read image data	
		MessageBox(nullptr, L"Could not read image data", L"ERROR", MB_OK);			// Display Error
		fclose(fTGA);														// Close file
		return false;														// Return failed
	}

	// Byte Swapping Optimized By Steve Thomas
	for(int cswap = 0; cswap < (int)tga.imageSize; cswap += tga.bytesPerPixel){
		terrainTexture_imageData[cswap] ^= terrainTexture_imageData[cswap+2] ^=
			terrainTexture_imageData[cswap] ^= terrainTexture_imageData[cswap+2];
	}

	fclose(fTGA); // Close file*/

	heightMap.terrainWidth = tga.width;
	heightMap.terrainDepth = tga.height;

	///The actual height computing and storing
	for (int i = 0; i < heightMap.terrainWidth; i++){
		for (int j = 0; j < heightMap.terrainDepth; j++){
			float pointHeight = terrainTexture_imageData[(j*heightMap.terrainWidth+i)*3];
			heightMap.heights.push_back(pointHeight);
			if (pointHeight > heightMap.maxHeight){
				heightMap.maxHeight = pointHeight;
			}
		}
	}
	
	return true;															
}

void HeightmapParser::Reset(HeightMap &heightMap) {
	heightMap.maxHeight = 0;
	heightMap.heights.clear();
}
