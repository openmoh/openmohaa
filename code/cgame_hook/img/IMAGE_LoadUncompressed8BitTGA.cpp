//////////////////////////////////////////////////////////////////////////////////////////
//	IMAGE_LoadUncompressed8BitTGA.cpp
//	Load an uncompressed 8 Bit TGA 
//	Downloaded from: www.paulsprojects.net
//	Created:	16th November 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <GL/gl.h>
#include "IMAGE.h"

//Load an uncompressed 8 Bit TGA (24 or 32 bit)
bool IMAGE::LoadUncompressed8BitTGA(char * filename)
{
	unsigned char uncomp8Bit24TGAHeader[12]={0, 1, 1, 0, 0, 0, 1, 24, 0, 0, 0, 0};
	unsigned char uncomp8Bit32TGAHeader[12]={0, 1, 1, 0, 0, 0, 1, 32, 0, 0, 0, 0};
	unsigned char TGAcompare[12];
	unsigned char header[6];

	printf("Loading %s in LoadUncompressed8BitTGA\n", filename);

	FILE * file=fopen(filename, "rb");

	if(!file)
	{
		printf("Unable to open %s\n", filename);
		return false;
	}

	//read the "compare" header
	fread(TGAcompare, 1, sizeof(TGAcompare), file);
	if(	memcmp(uncomp8Bit24TGAHeader, TGAcompare, sizeof(uncomp8Bit24TGAHeader))!=0	&&
		memcmp(uncomp8Bit32TGAHeader, TGAcompare, sizeof(uncomp8Bit32TGAHeader))!=0)
	{
		printf("%s is not an uncompressed 8Bit TGA\n", filename);
		return false;
	}

	//read in header
	fread(header, 1, sizeof(header), file);

	//Save data into class member variables
	width=	header[1]*256+header[0];
	height=	header[3]*256+header[2];
	bpp=	header[4];

	if(width<=0 || height<=0 || bpp!=8)
	{
		fclose(file);
		printf("%s's height or width is less than zero, or the TGA is not 8 bpp\n", filename);
		return false;
	}
		
	paletted=true;
	paletteBpp=TGAcompare[7];
	if(paletteBpp==24)
		paletteFormat=GL_RGB;
	if(paletteBpp==32)
		paletteFormat=GL_RGBA;

	//make space for palette
	palette=new GLubyte[256*paletteBpp/8];
	if(!palette)
	{
		fclose(file);
		printf("unable to allocate memory for palette\n");
		return false;
	}

	//load in the palette
	fread(palette, 256*paletteBpp/8, 1, file);

	//Palette is in BGR format
	//swap b and r
	for(int i=0; i<256; ++i)
	{
		//Repeated XOR to swap bytes 0 and 2
		palette[i*(paletteBpp/8)] ^= palette[i*(paletteBpp/8)+2] ^=
			palette[i*(paletteBpp/8)] ^= palette[i*(paletteBpp/8)+2];
	}
	
	//Set format
	format=GL_COLOR_INDEX;

	//Calculate the stride in bytes for each row (allow for 4-byte padding)
	stride=CalculateStride();

	//Allocate space for the image data
	data=new GLubyte[stride*height];
	if(!data)
	{
		fclose(file);
		printf(	"Unable to allocate data for %s of size %d x %d\n", filename,
								stride, height);
		return false;
	}

	//read in the data a line at a time, and save it into the array,
	//hence making it 32-byte aligned
	for(unsigned int row=0; row<height; ++row)
	{
		fread(&data[row*stride], 1, width, file);
	}

	fclose(file);

	printf("Loaded %s Correctly!\n", filename);
	return true;
}