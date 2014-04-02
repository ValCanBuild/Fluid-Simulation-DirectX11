/********************************************************************
TgaHeader: Struct that describes a tga file header

Author:	Valentin Hinov
Date: 24/3/2014
*********************************************************************/

#ifndef _TGAHEADER_H
#define _TGAHEADER_H

typedef struct
{
	unsigned char Header[12];									// TGA File Header
} TGAHeader;


typedef struct
{
	unsigned char		header[6];								// First 6 Useful Bytes From The Header
	unsigned int		bytesPerPixel;							// Holds Number Of Bytes Per Pixel Used In The TGA File
	unsigned int		imageSize;								// Used To Store The Image Size When Setting Aside Ram
	unsigned int		temp;									// Temporary Variable
	unsigned int		type;	
	unsigned int		height;									// Height of Image
	unsigned int		width;									// Width ofImage
	unsigned int		bpp;									// Bits Per Pixel
} TGA;

#endif
