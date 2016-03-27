//////////////////////////////////////////////////////////////////////////////////////////
//	IMAGE_LoadAlpha.cpp
//	Use the color indices from a paletted image as the alpha channel for this image
//	Downloaded from: www.paulsprojects.net
//	Created:	17th November 2002
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

//Load an image from a file
bool IMAGE::LoadAlpha(const IMAGE & alphaImage)
{
	//Cannot load alpha channel into a paletted image
	if(paletted)
	{
		printf("Cannot load alpha channel into a paletted image\n");
		return false;
	}

	//Check alpha image is paletted
	if(!alphaImage.paletted)
	{
		printf("Alpha image is non-paletted\n");
		return false;
	}

	//Check the images are the same size
	if(alphaImage.width!=width || alphaImage.height!=height)
	{
		printf("Alpha image is not the same size as color image\n");
		return false;
	}

	//Create space for new data
	int newStride=CalculateStride(32);

	GLubyte * tempData=new GLubyte[newStride*height];
	if(!tempData)
	{
		printf("Unable to allocate memory for temporary image data\n");
		return false;
	}

	//Fill in the temporary data
	for(unsigned int row=0; row<height; ++row)
	{
		for(unsigned int i=0; i<width; ++i)
		{
			tempData[(row*newStride)+i*4+0]=data[(row*stride)+i*bpp/8+0];
			tempData[(row*newStride)+i*4+1]=data[(row*stride)+i*bpp/8+1];
			tempData[(row*newStride)+i*4+2]=data[(row*stride)+i*bpp/8+2];

			tempData[(row*newStride)+i*4+3]=alphaImage.data[(row*alphaImage.stride)+i];
		}
	}

	//delete the old data and swap pointers
	if(data)
		delete [] data;
	data=tempData;

	//update bpp and format
	bpp=32;
	format=GL_RGBA;

	return true;
}