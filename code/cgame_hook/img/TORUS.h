//////////////////////////////////////////////////////////////////////////////////////////
//	TORUS.h
//	class declaration to create torus with tangents
//	Downloaded from: www.paulsprojects.net
//	Created:	20th July 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	

#ifndef TORUS_H
#define TORUS_H

class TORUS_VERTEX
{
public:
	Vector position;
	float s, t;
	Vector sTangent, tTangent;
	Vector normal;
	Vector tangentSpaceLight;
};

class TORUS
{
public:
	TORUS();
	~TORUS();

	bool InitTorus();

	int numVertices;
	int numIndices;

	unsigned int * indices;
	TORUS_VERTEX * vertices;
};

const int torusPrecision=48;

#endif	//TORUS_H
