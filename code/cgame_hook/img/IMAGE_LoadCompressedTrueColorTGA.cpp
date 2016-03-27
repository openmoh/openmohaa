//////////////////////////////////////////////////////////////////////////////////////////
//	IMAGE_LoadCompressedTrueColorTGA.cpp
//	Load a compressed true color TGA (24 or 32 bit)
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

//Load an compressed true color TGA (24 or 32 bit)
bool IMAGE::LoadCompressedTrueColorTGA(char * filename)
{
	unsigned char compTGAHeader[12]={0, 0, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char TGAcompare[12];
	unsigned char header[6];

	printf("Loading %s in LoadCompressedTGA\n", filename);

	FILE * file=fopen(filename, "rb");

	if(!file)
	{
		printf("Unable to open %s\n", filename);
		return false;
	}

	//read the "compare" header
	fread(TGAcompare, 1, sizeof(TGAcompare), file);
	if(memcmp(compTGAHeader, TGAcompare, sizeof(compTGAHeader))!=0)
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

	//allocate space for temporary data storage
	GLubyte * tempData=new GLubyte[width*height*bpp/8];
	if(!tempData)
	{
		printf("unable to allocate memory for temporary image data\n");
		fclose(file);
		return false;
	}

	//read in the image data
	int pixelCount=width*height;
	int currentPixel=0;
	int currentByte=0;
	unsigned char * colorBuffer=new unsigned char[bpp/8];

	do
	{
		unsigned char chunkHeader=0;

		if(fread(&chunkHeader, sizeof(unsigned char), 1, file)==0)
		{
			printf("could not read RLE chunk header\n");
			fclose(file);
			return false;
		}

		if(chunkHeader<128)		//read raw colour values
		{
			++chunkHeader;

			for(int counter=0; counter<chunkHeader; ++counter)
			{
				if(fread(colorBuffer, 1, bpp/8, file) != bpp/8)
				{
					printf("unable to read image data\n");
					fclose(file);
					return false;
				}

				//transfer pixel colour to data(swapping R and B values)
				tempData[currentByte+0]=colorBuffer[2];
				tempData[currentByte+1]=colorBuffer[1];
				tempData[currentByte+2]=colorBuffer[0];

				if(bpp/8==4)
					tempData[currentByte+3]=colorBuffer[3];

				currentByte+=bpp/8;
				++currentPixel;

				if(currentPixel>pixelCount)
				{
					printf("too many pixels read\n");
					fclose(file);
					return false;
				}
			}
		}
		else	//chunkHeader>=128
		{
			chunkHeader-=127;

			if(fread(colorBuffer, 1, bpp/8, file) != bpp/8)
			{
				printf("unable to read image data\n");
				fclose(file);
				return false;
			}

			for(int counter=0; counter<chunkHeader; ++counter)
			{
				//transfer pixel colour to data(swapping R and B values)
				tempData[currentByte+0]=colorBuffer[2];
				tempData[currentByte+1]=colorBuffer[1];
				tempData[currentByte+2]=colorBuffer[0];

				if(bpp/8==4)
					tempData[currentByte+3]=colorBuffer[3];

				currentByte+=bpp/8;
				++currentPixel;

				if(currentPixel>pixelCount)
				{
					printf("too many pixels read\n");
					fclose(file);
					return false;
				}
			}
		}
	}while(currentPixel<pixelCount);

	//Calculate the stride in bytes for each row (allow for 4-byte padding)
	stride=CalculateStride();

	//Allocate space for the image data
	data=new GLubyte[stride*height];
	if(!data)
	{
		printf(	"Unable to allocate data for %s of size %d x %d\n", filename,
								stride, height);
		return false;
	}

	//copy in the data a line at a time, hence making it 32-byte aligned
	for(unsigned int row=0; row<height; ++row)
	{
		memcpy(&data[(row*stride)], &tempData[row*width*bpp/8], width*bpp/8);
	}
	
	//delete the temporary data
	if(tempData)
		delete [] tempData;
	tempData=NULL;

	if(colorBuffer)
		delete [] colorBuffer;
	colorBuffer=NULL;
	printf("Loaded %s Correctly!\n", filename);
	return true;
}