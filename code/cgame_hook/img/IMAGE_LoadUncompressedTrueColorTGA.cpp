//////////////////////////////////////////////////////////////////////////////////////////
//	IMAGE_LoadUncompressedTrueColorTGA.cpp
//	Load an uncompressed true color TGA (24 or 32 bit)
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

//Load an uncompressed true color TGA (24 or 32 bit)
bool IMAGE::LoadUncompressedTrueColorTGA(char * filename)
{
	unsigned char uncompTGAHeader[12]={0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char TGAcompare[12];
	unsigned char header[6];

	printf("Loading %s in LoadUncompressedTGA\n", filename);

	FILE * file=fopen(filename, "rb");

	if(!file)
	{
		printf("Unable to open %s\n", filename);
		return false;
	}

	//read the "compare" header
	fread(TGAcompare, 1, sizeof(TGAcompare), file);
	if(memcmp(uncompTGAHeader, TGAcompare, sizeof(uncompTGAHeader))!=0)
	{
		printf("%s is not an uncompressed true color TGA\n", filename);
		return false;
	}

	//read in header
	fread(header, 1, sizeof(header), file);

	//Save data into class member variables
	width=	header[1]*256+header[0];
	height=	header[3]*256+header[2];
	bpp=	header[4];

	if(width<=0 || height<=0 || (bpp!=24 && bpp!=32))
	{
		fclose(file);
		printf("%s's height or width is less than zero, or the TGA is not 24/32 bpp\n", filename);
		return false;
	}

	//Set format
	if(bpp==24)
		format=GL_RGB;
	if(bpp==32)
		format=GL_RGBA;

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
		fread(&data[row*stride], 1, width*(bpp/8), file);
	}
	
	fclose(file);

	//Data is in BGR format
	//swap b and r
	for(unsigned int row=0; row<height; ++row)
	{
		for(unsigned int i=0; i<width; ++i)
		{
			//Repeated XOR to swap bytes 0 and 2
			data[(row*stride)+i*(bpp/8)] ^= data[(row*stride)+i*(bpp/8)+2] ^=
			data[(row*stride)+i*(bpp/8)] ^= data[(row*stride)+i*(bpp/8)+2];
		}
	}

	printf("Loaded %s Correctly!\n", filename);
	return true;
}