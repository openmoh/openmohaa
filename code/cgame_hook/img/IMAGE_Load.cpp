//////////////////////////////////////////////////////////////////////////////////////////
//	IMAGE_Load.cpp
//	Main Image loading function
//	Downloaded from: www.paulsprojects.net
//	Created:	20th July 2002
//	Modified:	9th November 2002	-	Corrected stride for TGA textures
//									-	Split up .cpp files
//									-	Added support for paletted textures
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
bool IMAGE::Load(char * filename)
{
	//Clear the member variables if already used
	bpp=0;
	width=height=0;
	stride=0;
	format=0;

	if(data)
		delete []  data;
	data=NULL;

	paletted=false;
	paletteBpp=0;
	paletteFormat=0;
	if(palette)
		delete [] palette;
	palette=NULL;

	//Call the correct loading function based on the filename
	int filenameLength=strlen(filename);

	if(	strncmp(filename+filenameLength-3, "BMP", 3)==0 ||
		strncmp(filename+filenameLength-3, "bmp", 3)==0)
		return LoadBMP(filename);

	if(	strncmp(filename+filenameLength-3, "TGA", 3)==0 ||
		strncmp(filename+filenameLength-3, "tga", 3)==0)
		return LoadTGA(filename);

	printf("%s does not end with \"bmp\" or \"tga\"\n", filename);
	return false;
}
