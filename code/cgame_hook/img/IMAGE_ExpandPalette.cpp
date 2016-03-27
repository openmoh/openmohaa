//////////////////////////////////////////////////////////////////////////////////////////
//	IMAGE_CalculateStride.cpp
//	Convert a paletted image into a non-paletted one
//	Downloaded from: www.paulsprojects.net
//	Created:	9th November 2002
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

//Convert a paletted image into a non-paletted one
void IMAGE::ExpandPalette(void)
{
	//Do not expand non-paletted images, or those with no data
	if(!paletted || !data)
		return;

	//Calculate the stride of the expanded image
	unsigned int newStride=CalculateStride(paletteBpp);

	//Create space for the expanded image data
	GLubyte * newData=new GLubyte[newStride*height];
	if(!newData)
	{
		printf("Unable to create memory for expanded data\n");
		return;
	}

	//Loop through and fill in the unpaletted data
	for(unsigned int row=0; row<height; ++row)
	{
		for(unsigned int i=0; i<width; ++i)
		{
			unsigned int currentOldPixel=(row*stride)+i;
			unsigned int currentNewPixel=(row*newStride)+i*(paletteBpp/8);
			GLubyte currentPaletteEntry=data[((row*stride)+i)];
		
			for(unsigned int j=0; j<paletteBpp/8; ++j)
			{
				newData[currentNewPixel+j]=
					palette[currentPaletteEntry*(paletteBpp/8)+j];
			}
		}
	}

	//Update class member variables
	paletted=false;

	bpp=paletteBpp;
	format=paletteFormat;
	stride=newStride;

	if(data)
		delete [] data;
	data=newData;
}