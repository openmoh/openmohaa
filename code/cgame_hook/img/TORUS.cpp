//////////////////////////////////////////////////////////////////////////////////////////
//	TORUS.cpp
//	functions for torus with tangents etc
//	Downloaded from: www.paulsprojects.net
//	Created:	20th July 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////
#include <glb_local.h>
#include <windows.h>
#include <stdio.h>
#include <GL\gl.h>
#include "TORUS.h"

TORUS::TORUS()
{
	InitTorus();
}

TORUS::~TORUS()
{
	if(indices)
		delete [] indices;
	indices=NULL;

	if(vertices)
		delete [] vertices;
	vertices=NULL;
}

bool TORUS::InitTorus()
{
	numVertices=(torusPrecision+1)*(torusPrecision+1);
	numIndices=2*torusPrecision*torusPrecision*3;

	vertices=new TORUS_VERTEX[numVertices];
	if(!vertices)
	{
		printf("Unable to allocate memory for torus vertices\n");
		return false;
	}

	indices=new unsigned int[numIndices];
	if(!indices)
	{
		printf("Unable to allocate memory for torus indices\n");
		return false;
	}
	
	//calculate the first ring - inner radius 4, outer radius 1.5
	for(int i=0; i<torusPrecision+1; i++)
	{
		vertices[i].position=Vector(150.0f, 0.0f, 0.0f).GetRotatedZ(i*360.0f/torusPrecision)+
									Vector(400.0f, 0.0f, 0.0f);
		
		vertices[i].s=0.0f;
		vertices[i].t=(float)i/torusPrecision;

		vertices[i].sTangent = Vector( 0.0f, 0.0f, -1.0f );
		vertices[i].tTangent=Vector(0.0f, -1.0f, 0.0f).GetRotatedZ(i*360.0f/torusPrecision);
		vertices[i].normal=vertices[i].tTangent.
			CrossProduct( vertices[ i ].tTangent, vertices[ i ].sTangent );
	}

	//rotate this to get other rings
	for(int ring=1; ring<torusPrecision+1; ring++)
	{
		for(int i=0; i<torusPrecision+1; i++)
		{
			vertices[ring*(torusPrecision+1)+i].position=vertices[i].position.GetRotatedY(ring*360.0f/torusPrecision);
			
			vertices[ring*(torusPrecision+1)+i].s=2.0f*ring/torusPrecision;
			vertices[ring*(torusPrecision+1)+i].t=vertices[i].t;

			vertices[ring*(torusPrecision+1)+i].sTangent=vertices[i].sTangent.GetRotatedY(ring*360.0f/torusPrecision);
			vertices[ring*(torusPrecision+1)+i].tTangent=vertices[i].tTangent.GetRotatedY(ring*360.0f/torusPrecision);
			vertices[ring*(torusPrecision+1)+i].normal=vertices[i].normal.GetRotatedY(ring*360.0f/torusPrecision);
		}
	}

	//calculate the indices
	for(int ring=0; ring<torusPrecision; ring++)
	{
		for(int i=0; i<torusPrecision; i++)
		{
			indices[((ring*torusPrecision+i)*2)*3+0]=ring*(torusPrecision+1)+i;
			indices[((ring*torusPrecision+i)*2)*3+1]=(ring+1)*(torusPrecision+1)+i;
			indices[((ring*torusPrecision+i)*2)*3+2]=ring*(torusPrecision+1)+i+1;
			indices[((ring*torusPrecision+i)*2+1)*3+0]=ring*(torusPrecision+1)+i+1;
			indices[((ring*torusPrecision+i)*2+1)*3+1]=(ring+1)*(torusPrecision+1)+i;
			indices[((ring*torusPrecision+i)*2+1)*3+2]=(ring+1)*(torusPrecision+1)+i+1;
		}
	}

	return true;
}