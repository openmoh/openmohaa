/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// q_math.c -- stateless support routines that are included in each code module

// Some of the vector functions are static inline in q_shared.h. q3asm
// doesn't understand static functions though, so we only want them in
// one file. That's what this is about.
#ifdef Q3_VM
#define __Q3_VM_MATH
#endif

#include "q_shared.h"

#define X 0
#define Y 1
#define Z 2
#define W 3

vec3_t	vec3_origin = {0,0,0};
vec3_t	axisDefault[3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
matrix_t matrixIdentity = {	1, 0, 0, 0,
							0, 1, 0, 0,
							0, 0, 1, 0,
							0, 0, 0, 1};

vec4_t		colorBlack	= {0, 0, 0, 1};
vec4_t		colorRed	= {1, 0, 0, 1};
vec4_t		colorGreen	= {0, 1, 0, 1};
vec4_t		colorBlue	= {0, 0, 1, 1};
vec4_t		colorYellow	= {1, 1, 0, 1};
vec4_t		colorMagenta= {1, 0, 1, 1};
vec4_t		colorCyan	= {0, 1, 1, 1};
vec4_t		colorWhite	= {1, 1, 1, 1};
vec4_t		colorLtGrey	= {0.75, 0.75, 0.75, 1};
vec4_t		colorMdGrey	= {0.5, 0.5, 0.5, 1};
vec4_t		colorDkGrey	= {0.25, 0.25, 0.25, 1};

vec4_t	g_color_table[8] =
	{
	{0.0, 0.0, 0.0, 1.0},
	{1.0, 0.0, 0.0, 1.0},
	{0.0, 1.0, 0.0, 1.0},
	{1.0, 1.0, 0.0, 1.0},
	{0.0, 0.0, 1.0, 1.0},
	{0.0, 1.0, 1.0, 1.0},
	{1.0, 0.0, 1.0, 1.0},
	{1.0, 1.0, 1.0, 1.0},
	};


vec3_t	bytedirs[NUMVERTEXNORMALS] =
{
{-0.525731f, 0.000000f, 0.850651f}, {-0.442863f, 0.238856f, 0.864188f}, 
{-0.295242f, 0.000000f, 0.955423f}, {-0.309017f, 0.500000f, 0.809017f}, 
{-0.162460f, 0.262866f, 0.951056f}, {0.000000f, 0.000000f, 1.000000f}, 
{0.000000f, 0.850651f, 0.525731f}, {-0.147621f, 0.716567f, 0.681718f}, 
{0.147621f, 0.716567f, 0.681718f}, {0.000000f, 0.525731f, 0.850651f}, 
{0.309017f, 0.500000f, 0.809017f}, {0.525731f, 0.000000f, 0.850651f}, 
{0.295242f, 0.000000f, 0.955423f}, {0.442863f, 0.238856f, 0.864188f}, 
{0.162460f, 0.262866f, 0.951056f}, {-0.681718f, 0.147621f, 0.716567f}, 
{-0.809017f, 0.309017f, 0.500000f},{-0.587785f, 0.425325f, 0.688191f}, 
{-0.850651f, 0.525731f, 0.000000f},{-0.864188f, 0.442863f, 0.238856f}, 
{-0.716567f, 0.681718f, 0.147621f},{-0.688191f, 0.587785f, 0.425325f}, 
{-0.500000f, 0.809017f, 0.309017f}, {-0.238856f, 0.864188f, 0.442863f}, 
{-0.425325f, 0.688191f, 0.587785f}, {-0.716567f, 0.681718f, -0.147621f}, 
{-0.500000f, 0.809017f, -0.309017f}, {-0.525731f, 0.850651f, 0.000000f}, 
{0.000000f, 0.850651f, -0.525731f}, {-0.238856f, 0.864188f, -0.442863f}, 
{0.000000f, 0.955423f, -0.295242f}, {-0.262866f, 0.951056f, -0.162460f}, 
{0.000000f, 1.000000f, 0.000000f}, {0.000000f, 0.955423f, 0.295242f}, 
{-0.262866f, 0.951056f, 0.162460f}, {0.238856f, 0.864188f, 0.442863f}, 
{0.262866f, 0.951056f, 0.162460f}, {0.500000f, 0.809017f, 0.309017f}, 
{0.238856f, 0.864188f, -0.442863f},{0.262866f, 0.951056f, -0.162460f}, 
{0.500000f, 0.809017f, -0.309017f},{0.850651f, 0.525731f, 0.000000f}, 
{0.716567f, 0.681718f, 0.147621f}, {0.716567f, 0.681718f, -0.147621f}, 
{0.525731f, 0.850651f, 0.000000f}, {0.425325f, 0.688191f, 0.587785f}, 
{0.864188f, 0.442863f, 0.238856f}, {0.688191f, 0.587785f, 0.425325f}, 
{0.809017f, 0.309017f, 0.500000f}, {0.681718f, 0.147621f, 0.716567f}, 
{0.587785f, 0.425325f, 0.688191f}, {0.955423f, 0.295242f, 0.000000f}, 
{1.000000f, 0.000000f, 0.000000f}, {0.951056f, 0.162460f, 0.262866f}, 
{0.850651f, -0.525731f, 0.000000f},{0.955423f, -0.295242f, 0.000000f}, 
{0.864188f, -0.442863f, 0.238856f}, {0.951056f, -0.162460f, 0.262866f}, 
{0.809017f, -0.309017f, 0.500000f}, {0.681718f, -0.147621f, 0.716567f}, 
{0.850651f, 0.000000f, 0.525731f}, {0.864188f, 0.442863f, -0.238856f}, 
{0.809017f, 0.309017f, -0.500000f}, {0.951056f, 0.162460f, -0.262866f}, 
{0.525731f, 0.000000f, -0.850651f}, {0.681718f, 0.147621f, -0.716567f}, 
{0.681718f, -0.147621f, -0.716567f},{0.850651f, 0.000000f, -0.525731f}, 
{0.809017f, -0.309017f, -0.500000f}, {0.864188f, -0.442863f, -0.238856f}, 
{0.951056f, -0.162460f, -0.262866f}, {0.147621f, 0.716567f, -0.681718f}, 
{0.309017f, 0.500000f, -0.809017f}, {0.425325f, 0.688191f, -0.587785f}, 
{0.442863f, 0.238856f, -0.864188f}, {0.587785f, 0.425325f, -0.688191f}, 
{0.688191f, 0.587785f, -0.425325f}, {-0.147621f, 0.716567f, -0.681718f}, 
{-0.309017f, 0.500000f, -0.809017f}, {0.000000f, 0.525731f, -0.850651f}, 
{-0.525731f, 0.000000f, -0.850651f}, {-0.442863f, 0.238856f, -0.864188f}, 
{-0.295242f, 0.000000f, -0.955423f}, {-0.162460f, 0.262866f, -0.951056f}, 
{0.000000f, 0.000000f, -1.000000f}, {0.295242f, 0.000000f, -0.955423f}, 
{0.162460f, 0.262866f, -0.951056f}, {-0.442863f, -0.238856f, -0.864188f}, 
{-0.309017f, -0.500000f, -0.809017f}, {-0.162460f, -0.262866f, -0.951056f}, 
{0.000000f, -0.850651f, -0.525731f}, {-0.147621f, -0.716567f, -0.681718f}, 
{0.147621f, -0.716567f, -0.681718f}, {0.000000f, -0.525731f, -0.850651f}, 
{0.309017f, -0.500000f, -0.809017f}, {0.442863f, -0.238856f, -0.864188f}, 
{0.162460f, -0.262866f, -0.951056f}, {0.238856f, -0.864188f, -0.442863f}, 
{0.500000f, -0.809017f, -0.309017f}, {0.425325f, -0.688191f, -0.587785f}, 
{0.716567f, -0.681718f, -0.147621f}, {0.688191f, -0.587785f, -0.425325f}, 
{0.587785f, -0.425325f, -0.688191f}, {0.000000f, -0.955423f, -0.295242f}, 
{0.000000f, -1.000000f, 0.000000f}, {0.262866f, -0.951056f, -0.162460f}, 
{0.000000f, -0.850651f, 0.525731f}, {0.000000f, -0.955423f, 0.295242f}, 
{0.238856f, -0.864188f, 0.442863f}, {0.262866f, -0.951056f, 0.162460f}, 
{0.500000f, -0.809017f, 0.309017f}, {0.716567f, -0.681718f, 0.147621f}, 
{0.525731f, -0.850651f, 0.000000f}, {-0.238856f, -0.864188f, -0.442863f}, 
{-0.500000f, -0.809017f, -0.309017f}, {-0.262866f, -0.951056f, -0.162460f}, 
{-0.850651f, -0.525731f, 0.000000f}, {-0.716567f, -0.681718f, -0.147621f}, 
{-0.716567f, -0.681718f, 0.147621f}, {-0.525731f, -0.850651f, 0.000000f}, 
{-0.500000f, -0.809017f, 0.309017f}, {-0.238856f, -0.864188f, 0.442863f}, 
{-0.262866f, -0.951056f, 0.162460f}, {-0.864188f, -0.442863f, 0.238856f}, 
{-0.809017f, -0.309017f, 0.500000f}, {-0.688191f, -0.587785f, 0.425325f}, 
{-0.681718f, -0.147621f, 0.716567f}, {-0.442863f, -0.238856f, 0.864188f}, 
{-0.587785f, -0.425325f, 0.688191f}, {-0.309017f, -0.500000f, 0.809017f}, 
{-0.147621f, -0.716567f, 0.681718f}, {-0.425325f, -0.688191f, 0.587785f}, 
{-0.162460f, -0.262866f, 0.951056f}, {0.442863f, -0.238856f, 0.864188f}, 
{0.162460f, -0.262866f, 0.951056f}, {0.309017f, -0.500000f, 0.809017f}, 
{0.147621f, -0.716567f, 0.681718f}, {0.000000f, -0.525731f, 0.850651f}, 
{0.425325f, -0.688191f, 0.587785f}, {0.587785f, -0.425325f, 0.688191f}, 
{0.688191f, -0.587785f, 0.425325f}, {-0.955423f, 0.295242f, 0.000000f}, 
{-0.951056f, 0.162460f, 0.262866f}, {-1.000000f, 0.000000f, 0.000000f}, 
{-0.850651f, 0.000000f, 0.525731f}, {-0.955423f, -0.295242f, 0.000000f}, 
{-0.951056f, -0.162460f, 0.262866f}, {-0.864188f, 0.442863f, -0.238856f}, 
{-0.951056f, 0.162460f, -0.262866f}, {-0.809017f, 0.309017f, -0.500000f}, 
{-0.864188f, -0.442863f, -0.238856f}, {-0.951056f, -0.162460f, -0.262866f}, 
{-0.809017f, -0.309017f, -0.500000f}, {-0.681718f, 0.147621f, -0.716567f}, 
{-0.681718f, -0.147621f, -0.716567f}, {-0.850651f, 0.000000f, -0.525731f}, 
{-0.688191f, 0.587785f, -0.425325f}, {-0.587785f, 0.425325f, -0.688191f}, 
{-0.425325f, 0.688191f, -0.587785f}, {-0.425325f, -0.688191f, -0.587785f}, 
{-0.587785f, -0.425325f, -0.688191f}, {-0.688191f, -0.587785f, -0.425325f}
};

//==============================================================

int		Q_rand( int *seed ) {
	*seed = (69069 * *seed + 1);
	return *seed;
}

float	Q_random( int *seed ) {
	return ( Q_rand( seed ) & 0xffff ) / (float)0x10000;
}

float	Q_crandom( int *seed ) {
	return 2.0 * ( Q_random( seed ) - 0.5 );
}


/*
grandom

This function produces a random number with a gaussian
distribution.  This also also known as a normal or bell
curve distribution; it has a mean value of zero and a
standard deviation of one.
*/
float grandom( void ) {
	double			v1;
	double			v2;
	double			s;
	float				x1;
	static float		x2 = 0;
	static int		toggle = 0;

	if( toggle ) {
		toggle = 0;
		return x2;
	}

	do {
		v1 = -1.0 + 2.0 * random();
		v2 = -1.0 + 2.0 * random();
		s = v1 * v1 + v2 * v2;
	} while( s >= 1.0 || s == 0 );

	s = sqrt( -2.0 * log( s ) / s );
	x1 = v1 * s;
	x2 = v2 * s;
	toggle = 1;
	return x1;
}


/*
erandom

This function produces a random number with a exponential
distribution and the specified mean value.
*/
float erandom( float mean ) {
	float	r;

	do {
		r = random();
	} while( r == 0.0 );

	return -mean * log( r );
}

//=======================================================

byte ClampByte(int i) {
	if(i < 0) {
		return 0;
	}
	if(i > 255) {
		return 255;
	}
	return i;
}

signed char ClampChar( int i ) {
	if ( i < -128 ) {
		return -128;
	}
	if ( i > 127 ) {
		return 127;
	}
	return i;
}

signed short ClampShort( int i ) {
	if ( i < -32768 ) {
		return -32768;
	}
	if ( i > 0x7fff ) {
		return 0x7fff;
	}
	return i;
}

//===========================================================================
//
// Global functions base on type double
//
//===========================================================================
#define SCALAR_EPSILON (0.000001f)
#define SCALAR_IDENTITY (0.0f)

double dEpsilon( void )
{
	return ( double )SCALAR_EPSILON;
}

double dIdentity( void )
{
	return ( double )SCALAR_IDENTITY;
}

double dSign( const double number )
{
	if( number >= 0.0 )
	{
		return 1;
	} else
	{
		return -1;
	}
}

double dClamp( const double value, const double min, const double max )
{
	assert( min <= max );
	if( value < min )
	{
		return min;
	}
	if( value > max )
	{
		return max;
	}
	return value;
}

double dDistance( const double value1, const double value2 )
{
	return fabs( value1 - value2 );
}

qboolean dCloseEnough( const double value1, const double value2, const double epsilon )
{
	return dDistance( value1, value2 ) < epsilon;
}

qboolean dSmallEnough( const double value, const double epsilon )
{
	return dDistance( dIdentity(), value ) < epsilon;
}

//===========================================================================
//
// Global functions base on type float
//
//===========================================================================
float fEpsilon( void )
{
	return SCALAR_EPSILON;
}

float fIdentity( void )
{
	return SCALAR_IDENTITY;
}

float fSign( const float number )
{
	if( number >= 0.0f )
	{
		return 1;
	} else
	{
		return -1;
	}
}

float	fClamp( const float value, const float min, const float max )
{
	assert( min <= max );
	if( value < min )
	{
		return min;
	}
	if( value > max )
	{
		return max;
	}
	return value;
}

float fDistance( const float value1, const float value2 )
{
	return fabs( value1 - value2 );
}

qboolean fCloseEnough( const float value1, const float value2, const float epsilon )
{
	return fDistance( value1, value2 ) < epsilon;
}

qboolean fSmallEnough( const float value, const float epsilon )
{
	return fDistance( fIdentity(), value ) < epsilon;
}

//===========================================================================
//
// Global functions base on type int
//
//===========================================================================
int iSign( const int number )
{
	if( number >= 0 )
	{
		return 1;
	} else
	{
		return -1;
	}
}

int iClamp( const int value, const int min, const int max )
{
	assert( min <= max );
	if( value < min )
	{
		return min;
	}
	if( value > max )
	{
		return max;
	}
	return value;
}


// this isn't a real cheap function to call!
int DirToByte( vec3_t dir ) {
	int		i, best;
	float	d, bestd;

	if ( !dir ) {
		return 0;
	}

	bestd = 0;
	best = 0;
	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		d = DotProduct (dir, bytedirs[i]);
		if (d > bestd)
		{
			bestd = d;
			best = i;
		}
	}

	return best;
}

void ByteToDir( int b, vec3_t dir ) {
	if ( b < 0 || b >= NUMVERTEXNORMALS ) {
		VectorCopy( vec3_origin, dir );
		return;
	}
	VectorCopy (bytedirs[b], dir);
}


unsigned ColorBytes3 (float r, float g, float b) {
	unsigned	i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;

	return i;
}

unsigned ColorBytes4 (float r, float g, float b, float a) {
	unsigned	i;

	( (byte *)&i )[0] = r * 255;
	( (byte *)&i )[1] = g * 255;
	( (byte *)&i )[2] = b * 255;
	( (byte *)&i )[3] = a * 255;

	return i;
}

float NormalizeColor( const vec3_t in, vec3_t out ) {
	float	max;
	
	max = in[0];
	if ( in[1] > max ) {
		max = in[1];
	}
	if ( in[2] > max ) {
		max = in[2];
	}

	if ( !max ) {
		VectorClear( out );
	} else {
		out[0] = in[0] / max;
		out[1] = in[1] / max;
		out[2] = in[2] / max;
	}
	return max;
}


void ClampColor(vec4_t color)
{
	int             i;

	for(i = 0; i < 4; i++)
	{
		if(color[i] < 0)
			color[i] = 0;

		if(color[i] > 1)
			color[i] = 1;
	}
}


/*
=====================
PlaneFromPoints

Returns false if the triangle is degenrate.
The normal will point out of the clock for clockwise ordered points
=====================
*/
qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c ) {
	vec3_t	d1, d2;

	VectorSubtract( b, a, d1 );
	VectorSubtract( c, a, d2 );
	CrossProduct( d2, d1, plane );
	if ( VectorNormalize( plane ) == 0 ) {
		return qfalse;
	}

	plane[3] = DotProduct( a, plane );
	return qtrue;
}

qboolean PlanesGetIntersectionPoint(const vec4_t plane1, const vec4_t plane2, const vec4_t plane3, vec3_t out)
{
	// http://www.cgafaq.info/wiki/Intersection_of_three_planes

	vec3_t	n1, n2, n3;
	vec3_t	n1n2, n2n3, n3n1;
	vec_t	denom;

	VectorNormalize2(plane1, n1);
	VectorNormalize2(plane2, n2);
	VectorNormalize2(plane3, n3);

	CrossProduct(n1, n2, n1n2);
	CrossProduct(n2, n3, n2n3);
	CrossProduct(n3, n1, n3n1);

	denom = DotProduct(n1, n2n3);

	// check if the denominator is zero (which would mean that no intersection is to be found
	if(denom == 0)
	{
		// no intersection could be found, return <0,0,0>
		VectorClear(out);
		return qfalse;
	}

	VectorClear(out);

	VectorMA(out, plane1[3], n2n3, out);
	VectorMA(out, plane2[3], n3n1, out);
	VectorMA(out, plane3[3], n1n2, out);

	VectorScale(out, 1.0f / denom, out);

	return qtrue;
}

void PlaneIntersectRay(const vec3_t rayPos, const vec3_t rayDir, const vec4_t plane, vec3_t res)
{
	vec3_t          dir;
	float           sect;
	float			distToPlane;
	float			planeDotRay;

	VectorNormalize2(rayDir, dir);

	distToPlane = DotProduct(plane, rayPos) - plane[3];
	planeDotRay = DotProduct(plane, dir);
	sect = -(distToPlane) / planeDotRay;

	VectorMA(rayPos, sect, dir, res);
}
vec_t PlaneNormalize(vec4_t plane)
{
	vec_t           length, ilength;

	length = sqrt(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
	if(length == 0)
	{
		VectorClear(plane);
		return 0;
	}

	ilength = 1.0 / length;
	plane[0] = plane[0] * ilength;
	plane[1] = plane[1] * ilength;
	plane[2] = plane[2] * ilength;
	plane[3] = plane[3] * ilength;

	return length;
}

/*
===============
RotatePointAroundVector

This is not implemented very well...
===============
*/
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point,
							 float degrees ) {
	float	m[3][3];
	float	im[3][3];
	float	zrot[3][3];
	float	tmpmat[3][3];
	float	rot[3][3];
	int	i;
	vec3_t vr, vup, vf;
	float	rad;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector( vr, dir );
	CrossProduct( vr, vf, vup );

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vup[0];
	m[1][1] = vup[1];
	m[2][1] = vup[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset( zrot, 0, sizeof( zrot ) );
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	rad = DEG2RAD( degrees );
	zrot[0][0] = cos( rad );
	zrot[0][1] = sin( rad );
	zrot[1][0] = -sin( rad );
	zrot[1][1] = cos( rad );

	Matrix3x3Multiply( m, zrot, tmpmat );
	Matrix3x3Multiply( tmpmat, im, rot );

	for ( i = 0; i < 3; i++ ) {
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}

/*
===============
RotateAroundDirection
===============
*/
void RotateAroundDirection( vec3_t axis[3], float yaw ) {

	// create an arbitrary axis[1] 
	PerpendicularVector( axis[1], axis[0] );

	// rotate it around axis[0] by yaw
	if ( yaw ) {
		vec3_t	temp;

		VectorCopy( axis[1], temp );
		RotatePointAroundVector( axis[1], axis[0], temp, yaw );
	}

	// cross to get axis[2]
	CrossProduct( axis[0], axis[1], axis[2] );
}

void vectoangles( const vec3_t value1, vec3_t angles ) {
	float	forward;
	float	yaw, pitch;
	
	if ( value1[1] == 0 && value1[0] == 0 ) {
		yaw = 0;
		if ( value1[2] > 0 ) {
			pitch = 90;
		}
		else {
			pitch = 270;
		}
	}
	else {
		if ( value1[0] ) {
			yaw = ( atan2 ( value1[1], value1[0] ) * 180 / M_PI );
		}
		else if ( value1[1] > 0 ) {
			yaw = 90;
		}
		else {
			yaw = 270;
		}
		if ( yaw < 0 ) {
			yaw += 360;
		}

		forward = sqrt ( value1[0]*value1[0] + value1[1]*value1[1] );
		pitch = ( atan2(value1[2], forward) * 180 / M_PI );
		if ( pitch < 0 ) {
			pitch += 360;
		}
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}


void VectorToAngles( const vec3_t vec, vec3_t angles )
{
	float	forward;
	float	yaw, pitch;

	if( vec[ 1 ] == 0 && vec[ 0 ] == 0 )
	{
		yaw = 0;
		if( vec[ 2 ] > 0 ) {
			pitch = 90;
		} else {
			pitch = 270;
		}
	}
	else
	{
		yaw = ( atan2( vec[ 1 ], vec[ 0 ] ) * ( 180.0f / M_PI ) );
		if( yaw < 0 ) {
			yaw += 360;
		}
		forward = sqrt( 1.0f - vec[ 2 ] * vec[ 2 ] );
		pitch = ( atan2( vec[ 2 ], forward ) * -( 180.0f / M_PI ) );
		if( pitch < 0 ) {
			pitch += 360;
		}
	}

	angles[ PITCH ] = pitch;
	angles[ YAW ] = yaw;
	angles[ ROLL ] = 0;
}

/*
=================
AnglesToAxis
=================
*/
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] ) {
	float		angle;
	static float		sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[ YAW ] * ( M_PI * 2 / 360 );
	sy = sin( angle );
	cy = cos( angle );
	angle = angles[ PITCH ] * ( M_PI * 2 / 360 );
	sp = sin( angle );
	cp = cos( angle );
	angle = angles[ ROLL ] * ( M_PI * 2 / 360 );
	sr = sin( angle );
	cr = cos( angle );

	axis[ 0 ][ 0 ] = cp*cy;
	axis[ 0 ][ 1 ] = cp*sy;
	axis[ 0 ][ 2 ] = -sp;

	axis[ 1 ][ 0 ] = ( sr*sp*cy + cr*-sy );
	axis[ 1 ][ 1 ] = ( sr*sp*sy + cr*cy );
	axis[ 1 ][ 2 ] = sr*cp;

	axis[ 2 ][ 0 ] = ( cr*sp*cy + -sr*-sy );
	axis[ 2 ][ 1 ] = ( cr*sp*sy + -sr*cy );
	axis[ 2 ][ 2 ] = cr*cp;
}

void AxisClear( vec3_t axis[3] ) {
	axis[0][0] = 1;
	axis[0][1] = 0;
	axis[0][2] = 0;
	axis[1][0] = 0;
	axis[1][1] = 1;
	axis[1][2] = 0;
	axis[2][0] = 0;
	axis[2][1] = 0;
	axis[2][2] = 1;
}

void AxisCopy( const vec3_t in[3], vec3_t out[3] ) {
	VectorCopy( in[0], out[0] );
	VectorCopy( in[1], out[1] );
	VectorCopy( in[2], out[2] );
}

void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal )
{
	float d;
	vec3_t n;
	float inv_denom;

	inv_denom =  DotProduct( normal, normal );
#ifndef Q3_VM
	assert( Q_fabs(inv_denom) != 0.0f ); // zero vectors get here
#endif
	inv_denom = 1.0f / inv_denom;

	d = DotProduct( normal, p ) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

/*
================
MakeNormalVectors

Given a normalized forward vector, create two
other perpendicular vectors
================
*/
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up) {
	float		d;

	// this rotate and negate guarantees a vector
	// not colinear with the original
	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = DotProduct (right, forward);
	VectorMA (right, -d, forward, right);
	VectorNormalize (right);
	CrossProduct (right, forward, up);
}


void VectorRotate( vec3_t in, vec3_t matrix[3], vec3_t out )
{
	out[0] = DotProduct( in, matrix[0] );
	out[1] = DotProduct( in, matrix[1] );
	out[2] = DotProduct( in, matrix[2] );
}

//============================================================================

#if !idppc
/*
** float q_rsqrt( float number )
*/
float Q_rsqrt( float number )
{
	union {
		float f;
		int i;
	} t;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	t.f  = number;
	t.i  = 0x5f3759df - ( t.i >> 1 );               // what the fuck?
	y  = t.f;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

float Q_fabs( float f ) {
	int tmp = * ( int * ) &f;
	tmp &= 0x7FFFFFFF;
	return * ( float * ) &tmp;
}
#endif

/*
=====================
Q_acos

the msvc acos doesn't always return a value between -PI and PI:

int i;
i = 1065353246;
acos(*(float*) &i) == -1.#IND0

	This should go in q_math but it is too late to add new traps
	to game and ui
=====================
*/
float Q_acos(float c) {
	float angle;

	angle = acos(c);

	if (angle > M_PI) {
		return (float)M_PI;
	}
	if (angle < -M_PI) {
		return (float)M_PI;
	}
	return angle;
}

//============================================================

/*
===============
LerpAngle

===============
*/
float LerpAngle (float from, float to, float frac) {
	float	a;

	if ( to - from > 180 ) {
		to -= 360;
	}
	if ( to - from < -180 ) {
		to += 360;
	}
	a = from + frac * (to - from);

	return a;
}

/*
================
R_ConcatRotations
================
*/
void R_ConcatRotations( float in1[ 3 ][ 3 ], float in2[ 3 ][ 3 ], float out[ 3 ][ 3 ] )
{
	out[ 0 ][ 0 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 0 ][ 1 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 0 ][ 2 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 1 ][ 0 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 1 ][ 1 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 1 ][ 2 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 2 ][ 0 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 2 ][ 1 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 2 ][ 2 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 2 ];
}


/*
================
R_ConcatTransforms
================
*/
void R_ConcatTransforms( float in1[ 3 ][ 4 ], float in2[ 3 ][ 4 ], float out[ 3 ][ 4 ] )
{
	out[ 0 ][ 0 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 0 ][ 1 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 0 ][ 2 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 0 ][ 3 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 3 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 0 ][ 3 ];
	out[ 1 ][ 0 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 1 ][ 1 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 1 ][ 2 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 1 ][ 3 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 3 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 1 ][ 3 ];
	out[ 2 ][ 0 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 2 ][ 1 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 2 ][ 2 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 2 ][ 3 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 3 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 3 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 3 ] + in1[ 2 ][ 3 ];
}

/*
===============
LerpAngleFromCurrent

===============
*/
float LerpAngleFromCurrent( float from, float to, float current, float frac ) {
	float	a;

	if( to - current > 180 ) {
		to -= 360;
	}
	if( to - current < -180 ) {
		to += 360;
	}
	a = from + frac * ( to - from );

	return a;
}

/*
=================
AngleSubtract

Always returns a value from -180 to 180
=================
*/
float	AngleSubtract( float a1, float a2 ) {
	float	a;

	a = a1 - a2;
	while ( a > 180 ) {
		a -= 360;
	}
	while ( a < -180 ) {
		a += 360;
	}
	return a;
}


void AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 ) {
	v3[0] = AngleSubtract( v1[0], v2[0] );
	v3[1] = AngleSubtract( v1[1], v2[1] );
	v3[2] = AngleSubtract( v1[2], v2[2] );
}


float	AngleMod(float a) {
	a = (360.0/65536) * ((int)(a*(65536/360.0)) & 65535);
	return a;
}


/*
=================
AngleNormalize360

returns angle normalized to the range [0 <= angle < 360]
=================
*/
float AngleNormalize360 ( float angle ) {
	return (360.0 / 65536) * ((int)(angle * (65536 / 360.0)) & 65535);
}


/*
=================
AngleNormalize180

returns angle normalized to the range [-180 < angle <= 180]
=================
*/
float AngleNormalize180 ( float angle ) {
	angle = AngleNormalize360( angle );
	if ( angle > 180.0 ) {
		angle -= 360.0;
	}
	return angle;
}


/*
=================
AngleDelta

returns the normalized delta from angle1 to angle2
=================
*/
float AngleDelta ( float angle1, float angle2 ) {
	return AngleNormalize180( angle1 - angle2 );
}


//============================================================


/*
=================
SetPlaneSignbits
=================
*/
void SetPlaneSignbits (cplane_t *out) {
	int	bits, j;

	// for fast box on planeside test
	bits = 0;
	for (j=0 ; j<3 ; j++) {
		if (out->normal[j] < 0) {
			bits |= 1<<j;
		}
	}
	out->signbits = bits;
}

//============================================================================


float	anglemod( float a )
{
#if 0
	if( a >= 0 )
		a -= 360 * ( int )( a / 360 );
	else
		a += 360 * ( 1 + ( int )( -a / 360 ) );
#endif
	a = ( 360.0 / 65536 ) * ( ( int )( a*( 65536 / 360.0 ) ) & 65535 );
	return a;
}

float angledist( float ang )
{
	float a;

	a = anglemod( ang );
	if( a > 180 )
	{
		a -= 360;
	}

	return a;
}

int		i;
vec3_t	corners[ 2 ];

/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2

// this is the slow, general version
int BoxOnPlaneSide2 (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	int		i;
	float	dist1, dist2;
	int		sides;
	vec3_t	corners[2];

	for (i=0 ; i<3 ; i++)
	{
		if (p->normal[i] < 0)
		{
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else
		{
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}
	dist1 = DotProduct (p->normal, corners[0]) - p->dist;
	dist2 = DotProduct (p->normal, corners[1]) - p->dist;
	sides = 0;
	if (dist1 >= 0)
		sides = 1;
	if (dist2 < 0)
		sides |= 2;

	return sides;
}

==================
*/

#if !id386

int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	float	dist1, dist2;
	int		sides;

// fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}

// general case
	switch (p->signbits)
	{
	case 0:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		break;
	}

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

	return sides;
}
#elif __GNUC__
// use matha.s
#else
#pragma warning( disable: 4035 )

__declspec( naked ) int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *p)
{
	static int bops_initialized;
	static int Ljmptab[8];

	__asm {

		push ebx
			
		cmp bops_initialized, 1
		je  initialized
		mov bops_initialized, 1
		
		mov Ljmptab[0*4], offset Lcase0
		mov Ljmptab[1*4], offset Lcase1
		mov Ljmptab[2*4], offset Lcase2
		mov Ljmptab[3*4], offset Lcase3
		mov Ljmptab[4*4], offset Lcase4
		mov Ljmptab[5*4], offset Lcase5
		mov Ljmptab[6*4], offset Lcase6
		mov Ljmptab[7*4], offset Lcase7
			
initialized:

		mov edx,dword ptr[4+12+esp]
		mov ecx,dword ptr[4+4+esp]
		xor eax,eax
		mov ebx,dword ptr[4+8+esp]
		mov al,byte ptr[17+edx]
		cmp al,8
		jge Lerror
		fld dword ptr[0+edx]
		fld st(0)
		jmp dword ptr[Ljmptab+eax*4]
Lcase0:
		fmul dword ptr[ebx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ebx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase1:
		fmul dword ptr[ecx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ebx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase2:
		fmul dword ptr[ebx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ecx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase3:
		fmul dword ptr[ecx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ecx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ebx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ecx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase4:
		fmul dword ptr[ebx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ebx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase5:
		fmul dword ptr[ecx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ebx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase6:
		fmul dword ptr[ebx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ecx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ecx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
		jmp LSetSides
Lcase7:
		fmul dword ptr[ecx]
		fld dword ptr[0+4+edx]
		fxch st(2)
		fmul dword ptr[ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[4+ecx]
		fld dword ptr[0+8+edx]
		fxch st(2)
		fmul dword ptr[4+ebx]
		fxch st(2)
		fld st(0)
		fmul dword ptr[8+ecx]
		fxch st(5)
		faddp st(3),st(0)
		fmul dword ptr[8+ebx]
		fxch st(1)
		faddp st(3),st(0)
		fxch st(3)
		faddp st(2),st(0)
LSetSides:
		faddp st(2),st(0)
		fcomp dword ptr[12+edx]
		xor ecx,ecx
		fnstsw ax
		fcomp dword ptr[12+edx]
		and ah,1
		xor ah,1
		add cl,ah
		fnstsw ax
		and ah,1
		add ah,ah
		add cl,ah
		pop ebx
		mov eax,ecx
		ret
Lerror:
		int 3
	}
}
#pragma warning( default: 4035 )

#endif

/*
=================
CalculateRotatedBounds
=================
*/
void CalculateRotatedBounds( vec3_t angles, vec3_t mins, vec3_t maxs )
{
	int i;
	vec3_t rotmins, rotmaxs;
	float trans[ 3 ][ 3 ];

	AnglesToAxis( angles, trans );
	ClearBounds( rotmins, rotmaxs );
	for( i = 0; i < 8; i++ )
	{
		vec3_t   tmp, rottemp;

		if( i & 1 )
			tmp[ 0 ] = mins[ 0 ];
		else
			tmp[ 0 ] = maxs[ 0 ];

		if( i & 2 )
			tmp[ 1 ] = mins[ 1 ];
		else
			tmp[ 1 ] = maxs[ 1 ];

		if( i & 4 )
			tmp[ 2 ] = mins[ 2 ];
		else
			tmp[ 2 ] = maxs[ 2 ];

		MatrixTransformVector( tmp, trans, rottemp );
		AddPointToBounds( rottemp, rotmins, rotmaxs );
	}
	VectorCopy( rotmins, mins );
	VectorCopy( rotmaxs, maxs );
}

/*
=================
CalculateRotatedBounds2
=================
*/
void CalculateRotatedBounds2( float trans[ 3 ][ 3 ], vec3_t mins, vec3_t maxs )
{
	int i;
	vec3_t rotmins, rotmaxs;

	ClearBounds( rotmins, rotmaxs );
	for( i = 0; i < 8; i++ )
	{
		vec3_t   tmp, rottemp;

		if( i & 1 )
			tmp[ 0 ] = mins[ 0 ];
		else
			tmp[ 0 ] = maxs[ 0 ];

		if( i & 2 )
			tmp[ 1 ] = mins[ 1 ];
		else
			tmp[ 1 ] = maxs[ 1 ];

		if( i & 4 )
			tmp[ 2 ] = mins[ 2 ];
		else
			tmp[ 2 ] = maxs[ 2 ];

		MatrixTransformVector( tmp, trans, rottemp );
		AddPointToBounds( rottemp, rotmins, rotmaxs );
	}
	VectorCopy( rotmins, mins );
	VectorCopy( rotmaxs, maxs );
}

#define BBOX_XBITS 9
#define BBOX_YBITS 8
#define BBOX_ZBOTTOMBITS 5
#define BBOX_ZTOPBITS 9

#define BBOX_MAX_X ( 1 << BBOX_XBITS )
#define BBOX_MAX_Y ( 1 << BBOX_YBITS )
#define BBOX_MAX_BOTTOM_Z ( 1 << ( BBOX_ZBOTTOMBITS - 1 ) )
#define BBOX_REALMAX_BOTTOM_Z ( 1 << BBOX_ZBOTTOMBITS )
#define BBOX_MAX_TOP_Z ( 1 << BBOX_ZTOPBITS )

/*
=================
BoundingBoxToInteger
=================
*/
int BoundingBoxToInteger( vec3_t mins, vec3_t maxs )
{
	int x, y, zd, zu, result;

	x = maxs[ 0 ];
	if( x < 0 )
		x = 0;
	if( x >= BBOX_MAX_X )
		x = BBOX_MAX_X - 1;

	y = maxs[ 1 ];
	if( y < 0 )
		y = 0;
	if( y >= BBOX_MAX_Y )
		y = BBOX_MAX_Y - 1;

	zd = mins[ 2 ] + BBOX_MAX_BOTTOM_Z;
	if( zd < 0 )
	{
		zd = 0;
	}
	if( zd >= BBOX_REALMAX_BOTTOM_Z )
	{
		zd = BBOX_REALMAX_BOTTOM_Z - 1;
	}

	zu = maxs[ 2 ];
	if( zu < 0 )
		zu = 0;
	if( zu >= BBOX_MAX_TOP_Z )
		zu = BBOX_MAX_TOP_Z - 1;

	result = x |
		( y << BBOX_XBITS ) |
		( zd << ( BBOX_XBITS + BBOX_YBITS ) ) |
		( zu << ( BBOX_XBITS + BBOX_YBITS + BBOX_ZBOTTOMBITS ) );

	return result;
}

/*
=================
IntegerToBoundingBox
=================
*/
void IntegerToBoundingBox( int num, vec3_t mins, vec3_t maxs )
{
	int x, y, zd, zu;

	x = num & ( BBOX_MAX_X - 1 );
	y = ( num >> ( BBOX_XBITS ) ) & ( BBOX_MAX_Y - 1 );
	zd = ( num >> ( BBOX_XBITS + BBOX_YBITS ) ) & ( BBOX_REALMAX_BOTTOM_Z - 1 );
	zd -= BBOX_MAX_BOTTOM_Z;
	zu = ( num >> ( BBOX_XBITS + BBOX_YBITS + BBOX_ZBOTTOMBITS ) ) & ( BBOX_MAX_TOP_Z - 1 );

	mins[ 0 ] = -x;
	mins[ 1 ] = -y;
	mins[ 2 ] = zd;

	maxs[ 0 ] = x;
	maxs[ 1 ] = y;
	maxs[ 2 ] = zu;
}

/*
=================
RadiusFromBounds
=================
*/
float RadiusFromBounds( const vec3_t mins, const vec3_t maxs ) {
	int		i;
	vec3_t	corner;
	float	a, b;

	for (i=0 ; i<3 ; i++) {
		a = fabs( mins[i] );
		b = fabs( maxs[i] );
		corner[i] = a > b ? a : b;
	}

	return VectorLength (corner);
}


#define BOUNDS_CLEAR_VALUE 99999

void ClearBounds( vec3_t mins, vec3_t maxs ) {
	mins[ 0 ] = mins[ 1 ] = mins[ 2 ] = BOUNDS_CLEAR_VALUE;
	maxs[ 0 ] = maxs[ 1 ] = maxs[ 2 ] = -BOUNDS_CLEAR_VALUE;
}

qboolean BoundsClear( vec3_t mins, vec3_t maxs )
{
	if(
		( mins[ 0 ] == BOUNDS_CLEAR_VALUE ) &&
		( mins[ 1 ] == BOUNDS_CLEAR_VALUE ) &&
		( mins[ 2 ] == BOUNDS_CLEAR_VALUE ) &&
		( maxs[ 0 ] == -BOUNDS_CLEAR_VALUE ) &&
		( maxs[ 1 ] == -BOUNDS_CLEAR_VALUE ) &&
		( maxs[ 2 ] == -BOUNDS_CLEAR_VALUE )
		)
	{
		return qtrue;
	}
	else
	{
		return qfalse;
	}
}

void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs ) {
	if( v[ 0 ] < mins[ 0 ] ) {
		mins[ 0 ] = v[ 0 ];
	}
	if( v[ 0 ] > maxs[ 0 ] ) {
		maxs[ 0 ] = v[ 0 ];
	}

	if( v[ 1 ] < mins[ 1 ] ) {
		mins[ 1 ] = v[ 1 ];
	}
	if( v[ 1 ] > maxs[ 1 ] ) {
		maxs[ 1 ] = v[ 1 ];
	}

	if( v[ 2 ] < mins[ 2 ] ) {
		mins[ 2 ] = v[ 2 ];
	}
	if( v[ 2 ] > maxs[ 2 ] ) {
		maxs[ 2 ] = v[ 2 ];
	}
}

qboolean BoundsIntersect(const vec3_t mins, const vec3_t maxs,
		const vec3_t mins2, const vec3_t maxs2)
{
	if ( maxs[0] < mins2[0] ||
		maxs[1] < mins2[1] ||
		maxs[2] < mins2[2] ||
		mins[0] > maxs2[0] ||
		mins[1] > maxs2[1] ||
		mins[2] > maxs2[2])
	{
		return qfalse;
	}

	return qtrue;
}

qboolean BoundsIntersectSphere(const vec3_t mins, const vec3_t maxs,
		const vec3_t origin, vec_t radius)
{
	if ( origin[0] - radius > maxs[0] ||
		origin[0] + radius < mins[0] ||
		origin[1] - radius > maxs[1] ||
		origin[1] + radius < mins[1] ||
		origin[2] - radius > maxs[2] ||
		origin[2] + radius < mins[2])
	{
		return qfalse;
	}

	return qtrue;
}

qboolean BoundsIntersectPoint(const vec3_t mins, const vec3_t maxs,
		const vec3_t origin)
{
	if ( origin[0] > maxs[0] ||
		origin[0] < mins[0] ||
		origin[1] > maxs[1] ||
		origin[1] < mins[1] ||
		origin[2] > maxs[2] ||
		origin[2] < mins[2])
	{
		return qfalse;
	}

	return qtrue;
}

void QuatToMat
	(
	const float q[ 4 ],
	float m[ 3 ][ 3 ]
	)

{
	float wx, wy, wz;
	float xx, yy, yz;
	float xy, xz, zz;
	float x2, y2, z2;

	x2 = q[ X ] + q[ X ];
	y2 = q[ Y ] + q[ Y ];
	z2 = q[ Z ] + q[ Z ];

	xx = q[ X ] * x2;
	xy = q[ X ] * y2;
	xz = q[ X ] * z2;

	yy = q[ Y ] * y2;
	yz = q[ Y ] * z2;
	zz = q[ Z ] * z2;

	wx = q[ W ] * x2;
	wy = q[ W ] * y2;
	wz = q[ W ] * z2;

	m[ 0 ][ 0 ] = 1.0 - ( yy + zz );
	m[ 0 ][ 1 ] = xy - wz;
	m[ 0 ][ 2 ] = xz + wy;

	m[ 1 ][ 0 ] = xy + wz;
	m[ 1 ][ 1 ] = 1.0 - ( xx + zz );
	m[ 1 ][ 2 ] = yz - wx;

	m[ 2 ][ 0 ] = xz - wy;
	m[ 2 ][ 1 ] = yz + wx;
	m[ 2 ][ 2 ] = 1.0 - ( xx + yy );
}

void MatToQuat
	(
	float srcMatrix[ 3 ][ 3 ],
	float destQuat[ 4 ]
	)

{
	double  	trace, s;
	int     	i, j, k;
	static int 	next[ 3 ] = { Y, Z, X };

	trace = srcMatrix[ X ][ X ] + srcMatrix[ Y ][ Y ] + srcMatrix[ Z ][ Z ];

	if( trace > 0.0 )
	{
		s = sqrt( trace + 1.0 );
		destQuat[ W ] = s * 0.5;
		s = 0.5 / s;

		destQuat[ X ] = ( srcMatrix[ Z ][ Y ] - srcMatrix[ Y ][ Z ] ) * s;
		destQuat[ Y ] = ( srcMatrix[ X ][ Z ] - srcMatrix[ Z ][ X ] ) * s;
		destQuat[ Z ] = ( srcMatrix[ Y ][ X ] - srcMatrix[ X ][ Y ] ) * s;
	}
	else
	{
		i = X;
		if( srcMatrix[ Y ][ Y ] > srcMatrix[ X ][ X ] )
			i = Y;
		if( srcMatrix[ Z ][ Z ] > srcMatrix[ i ][ i ] )
			i = Z;
		j = next[ i ];
		k = next[ j ];

		s = sqrt( ( srcMatrix[ i ][ i ] - ( srcMatrix[ j ][ j ] + srcMatrix[ k ][ k ] ) ) + 1.0 );
		destQuat[ i ] = s * 0.5;

		s = 0.5 / s;

		destQuat[ W ] = ( srcMatrix[ k ][ j ] - srcMatrix[ j ][ k ] ) * s;
		destQuat[ j ] = ( srcMatrix[ j ][ i ] + srcMatrix[ i ][ j ] ) * s;
		destQuat[ k ] = ( srcMatrix[ k ][ i ] + srcMatrix[ i ][ k ] ) * s;
	}
}

void EulerToQuat
	(
	float ang[ 3 ],
	float q[ 4 ]
	)

{
	float mat[ 3 ][ 3 ];
	int *i;

	i = ( int * )ang;
	if( !i[ 0 ] && !i[ 1 ] && !i[ 2 ] )
	{
		q[ 0 ] = 0;
		q[ 1 ] = 0;
		q[ 2 ] = 0;
		q[ 3 ] = 1.0f;
	}
	else
	{
		AnglesToAxis( ang, mat );
		MatToQuat( mat, q );
	}
}

float ProjectPointOnLine
	(
	vec_t *i_vStart,
	vec_t *i_vEnd,
	vec_t *i_vPoint,
	vec_t *o_vProj
	)

{
	float fDot;
	vec3_t vDir;
	vec3_t vDelta;

	VectorSubtract( i_vEnd, i_vStart, vDir );
	VectorNormalizeFast( vDir );

	VectorSubtract( i_vPoint, i_vStart, vDelta );
	fDot = DotProduct( vDelta, vDir );

	VectorScale( vDir, fDot, o_vProj );
	VectorAdd( o_vProj, i_vStart, o_vProj );

	return fDot;
}

float ProjectLineOnPlane
	(
	vec_t *vPlaneNorm,
	float fPlaneDist,
	vec_t *vStart,
	vec_t *vEnd,
	vec_t *vProj
	)

{
	float d1;
	float d2;
	float f;

	d1 = DotProduct( vStart, vPlaneNorm ) - fPlaneDist;
	d2 = DotProduct( vEnd, vPlaneNorm ) - fPlaneDist;

	if( d1 == d2 )
	{
		if( vProj )
		{
			VectorCopy( vStart, vProj );
		}

		return 0.0f;
	}
	else
	{
		f = d1 / ( d1 - d2 );

		if( vProj )
		{
			VectorSubtract( vEnd, vStart, vProj );
			VectorScale( vProj, f, vProj );
			VectorAdd( vProj, vStart, vProj );
		}

		return f;
	}
}

vec_t VectorNormalize( vec3_t v ) {
	// NOTE: TTimo - Apple G4 altivec source uses double?
	float	length, ilength;

	length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	length = sqrt (length);

	if ( length ) {
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}
		
	return length;
}

vec_t VectorNormalize2( const vec3_t v, vec3_t out ) {
	float	length, ilength;

	length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	length = sqrt (length);

	if (length)
	{
		ilength = 1/length;
		out[0] = v[0]*ilength;
		out[1] = v[1]*ilength;
		out[2] = v[2]*ilength;
	} else {
		VectorClear( out );
	}
		
	return length;
}

vec_t VectorNormalize2D( vec2_t v ) {
	float	length, ilength;

	length = v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ];
	length = sqrt( length );

	if( length ) {
		ilength = 1 / length;
		v[ 0 ] *= ilength;
		v[ 1 ] *= ilength;
	}

	return length;
}

vec_t VectorNormalize2D2( const vec2_t v, vec2_t out ) {
	float	length, ilength;

	length = v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ];
	length = sqrt( length );

	if( length )
	{
		ilength = 1 / length;
		out[ 0 ] = v[ 0 ] * ilength;
		out[ 1 ] = v[ 1 ] * ilength;
	}
	else {
		VectorClear2D( out );
	}

	return length;

}

void VectorPackTo01( vec3_t v ) {
	VectorNormalize( v );
	VectorScale( v, 0.5f, v );

	v[ 0 ] += 0.5f;
	v[ 1 ] += 0.5f;
	v[ 2 ] += 0.5f;
}

vec_t Q_rint( vec_t in )
{
	return floor( in + 0.5 );
}

void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc) {
	vecc[0] = veca[0] + scale*vecb[0];
	vecc[1] = veca[1] + scale*vecb[1];
	vecc[2] = veca[2] + scale*vecb[2];
}


vec_t _DotProduct( const vec3_t v1, const vec3_t v2 ) {
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out ) {
	out[0] = veca[0]-vecb[0];
	out[1] = veca[1]-vecb[1];
	out[2] = veca[2]-vecb[2];
}

void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out ) {
	out[0] = veca[0]+vecb[0];
	out[1] = veca[1]+vecb[1];
	out[2] = veca[2]+vecb[2];
}

void _VectorCopy( const vec3_t in, vec3_t out ) {
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

void _VectorScale( const vec3_t in, vec_t scale, vec3_t out ) {
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
}

void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out ) {
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
	out[3] = in[3]*scale;
}

int NearestPowerOfTwo(int val)
{
	int             answer;

	for(answer = 1; answer < val; answer <<= 1)
		;
	return answer;
}


int Q_log2( int val ) {
	int answer;

	answer = 0;
	while ( ( val>>=1 ) != 0 ) {
		answer++;
	}
	return answer;
}



/*
=================
PlaneTypeForNormal
=================
*/
/*
int	PlaneTypeForNormal (vec3_t normal) {
	if ( normal[0] == 1.0 )
		return PLANE_X;
	if ( normal[1] == 1.0 )
		return PLANE_Y;
	if ( normal[2] == 1.0 )
		return PLANE_Z;
	
	return PLANE_NON_AXIAL;
}
*/

/*
================
MatrixTransformVector
================
*/
void MatrixTransformVector
	(
	const vec3_t in,
	const float mat[ 3 ][ 3 ],
	vec3_t out
	)

{
	out[ 0 ] = ( in[ 0 ] * mat[ 0 ][ 0 ] ) + ( in[ 1 ] * mat[ 1 ][ 0 ] ) + ( in[ 2 ] * mat[ 2 ][ 0 ] );
	out[ 1 ] = ( in[ 0 ] * mat[ 0 ][ 1 ] ) + ( in[ 1 ] * mat[ 1 ][ 1 ] ) + ( in[ 2 ] * mat[ 2 ][ 1 ] );
	out[ 2 ] = ( in[ 0 ] * mat[ 0 ][ 2 ] ) + ( in[ 1 ] * mat[ 1 ][ 2 ] ) + ( in[ 2 ] * mat[ 2 ][ 2 ] );
}

/*
================
MatrixTransformVectorRight
================
*/
void MatrixTransformVectorRight
(
const float mat[ 3 ][ 3 ],
const vec3_t in,
vec3_t out
)

{
	out[ 0 ] = ( in[ 0 ] * mat[ 0 ][ 0 ] ) + ( in[ 1 ] * mat[ 0 ][ 1 ] ) + ( in[ 2 ] * mat[ 0 ][ 2 ] );
	out[ 1 ] = ( in[ 0 ] * mat[ 1 ][ 0 ] ) + ( in[ 1 ] * mat[ 1 ][ 1 ] ) + ( in[ 2 ] * mat[ 1 ][ 2 ] );
	out[ 2 ] = ( in[ 0 ] * mat[ 2 ][ 0 ] ) + ( in[ 1 ] * mat[ 2 ][ 1 ] ) + ( in[ 2 ] * mat[ 2 ][ 2 ] );
}

/*
================
Matrix3x3Multiply
================
*/
void Matrix3x3Multiply( const float in1[ 3 ][ 3 ], const float in2[ 3 ][ 3 ], float out[ 3 ][ 3 ] ) {
	out[ 0 ][ 0 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 0 ][ 1 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 0 ][ 2 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 1 ][ 0 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 1 ][ 1 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 1 ][ 2 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 2 ][ 0 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 2 ][ 1 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 2 ][ 2 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 2 ];
}


void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up ) {
	float		angle;
	static float		sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[ YAW ] * ( M_PI * 2 / 360 );
	sy = sin( angle );
	cy = cos( angle );
	angle = angles[ PITCH ] * ( M_PI * 2 / 360 );
	sp = sin( angle );
	cp = cos( angle );
	angle = angles[ ROLL ] * ( M_PI * 2 / 360 );
	sr = sin( angle );
	cr = cos( angle );

	if( forward )
	{
		forward[ 0 ] = cp*cy;
		forward[ 1 ] = cp*sy;
		forward[ 2 ] = -sp;
	}
	if( right )
	{
		right[ 0 ] = ( -1 * sr*sp*cy + -1 * cr*-sy );
		right[ 1 ] = ( -1 * sr*sp*sy + -1 * cr*cy );
		right[ 2 ] = -1 * sr*cp;
	}
	if( up )
	{
		up[ 0 ] = ( cr*sp*cy + -sr*-sy );
		up[ 1 ] = ( cr*sp*sy + -sr*cy );
		up[ 2 ] = cr*cp;
	}
}

void AngleVectorsLeft( const vec3_t angles, vec3_t forward, vec3_t left, vec3_t up )
{
	float		angle;
	static float		sr, sp, sy, cr, cp, cy;
	// static to help MS compiler fp bugs

	angle = angles[ YAW ] * ( M_PI * 2 / 360 );
	sy = sin( angle );
	cy = cos( angle );
	angle = angles[ PITCH ] * ( M_PI * 2 / 360 );
	sp = sin( angle );
	cp = cos( angle );

	if( forward )
	{
		forward[ 0 ] = cp*cy;
		forward[ 1 ] = cp*sy;
		forward[ 2 ] = -sp;
	}

	if( left || up )
	{
		angle = angles[ ROLL ] * ( M_PI * 2 / 360 );
		sr = sin( angle );
		cr = cos( angle );

		if( left )
		{
			left[ 0 ] = ( sr*sp*cy + cr*-sy );
			left[ 1 ] = ( sr*sp*sy + cr*cy );
			left[ 2 ] = sr*cp;
		}
		if( up )
		{
			up[ 0 ] = ( cr*sp*cy + -sr*-sy );
			up[ 1 ] = ( cr*sp*sy + -sr*cy );
			up[ 2 ] = cr*cp;
		}
	}
}

/*
** assumes "src" is normalized
*/
void PerpendicularVector( vec3_t dst, const vec3_t src )
{
	int	pos;
	int i;
	float minelem = 1.0F;
	vec3_t tempvec;

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for ( pos = 0, i = 0; i < 3; i++ )
	{
		if ( fabs( src[i] ) < minelem )
		{
			pos = i;
			minelem = fabs( src[i] );
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

	/*
	** project the point onto the plane defined by src
	*/
	ProjectPointOnPlane( dst, tempvec, src );

	/*
	** normalize the result
	*/
	VectorNormalize( dst );
}

/*
================
Q_isnan

Don't pass doubles to this
================
*/
int Q_isnan( float x )
{
	union
	{
		float f;
		unsigned int i;
	} t;

	t.f = x;
	t.i &= 0x7FFFFFFF;
	t.i = 0x7F800000 - t.i;

	return (int)( (unsigned int)t.i >> 31 );
}



/*
=================
GetPerpendicularViewVector

Used to find an "up" vector for drawing a sprite so that it always faces the view as best as possible
=================
*/
void GetPerpendicularViewVector(const vec3_t point, const vec3_t p1, const vec3_t p2, vec3_t up)
{
	vec3_t          v1, v2;

	VectorSubtract(point, p1, v1);
	VectorNormalize(v1);

	VectorSubtract(point, p2, v2);
	VectorNormalize(v2);

	CrossProduct(v1, v2, up);
	VectorNormalize(up);
}

/*
================
ProjectPointOntoVector
================
*/
void ProjectPointOntoVector(vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj)
{
	vec3_t          pVec, vec;

	VectorSubtract(point, vStart, pVec);
	VectorSubtract(vEnd, vStart, vec);
	VectorNormalize(vec);
	// project onto the directional vector for this segment
	VectorMA(vStart, DotProduct(pVec, vec), vec, vProj);
}

/*
================
VectorMaxComponent

Return the biggest component of some vector
================
*/
float VectorMaxComponent(vec3_t v)
{
	float           biggest = v[0];

	if(v[1] > biggest)
		biggest = v[1];

	if(v[2] > biggest)
		biggest = v[2];

	return biggest;
}

/*
================
VectorMinComponent

Return the smallest component of some vector
================
*/
float VectorMinComponent(vec3_t v)
{
	float           smallest = v[0];

	if(v[1] < smallest)
		smallest = v[1];

	if(v[2] < smallest)
		smallest = v[2];

	return smallest;
}


#define LINE_DISTANCE_EPSILON 1e-05f

/*
================
DistanceBetweenLineSegmentsSquared

Return the smallest distance between two line segments, squared
================
*/
vec_t DistanceBetweenLineSegmentsSquared(const vec3_t sP0, const vec3_t sP1,
										 const vec3_t tP0, const vec3_t tP1, float *s, float *t)
{
	vec3_t          sMag, tMag, diff;
	float           a, b, c, d, e;
	float           D;
	float           sN, sD;
	float           tN, tD;
	vec3_t          separation;

	VectorSubtract(sP1, sP0, sMag);
	VectorSubtract(tP1, tP0, tMag);
	VectorSubtract(sP0, tP0, diff);
	a = DotProduct(sMag, sMag);
	b = DotProduct(sMag, tMag);
	c = DotProduct(tMag, tMag);
	d = DotProduct(sMag, diff);
	e = DotProduct(tMag, diff);
	sD = tD = D = a * c - b * b;

	if(D < LINE_DISTANCE_EPSILON)
	{
		// the lines are almost parallel
		sN = 0.0;				// force using point P0 on segment S1
		sD = 1.0;				// to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else
	{
		// get the closest points on the infinite lines
		sN = (b * e - c * d);
		tN = (a * e - b * d);

		if(sN < 0.0)
		{
			// sN < 0 => the s=0 edge is visible
			sN = 0.0;
			tN = e;
			tD = c;
		}
		else if(sN > sD)
		{
			// sN > sD => the s=1 edge is visible
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if(tN < 0.0)
	{
		// tN < 0 => the t=0 edge is visible
		tN = 0.0;

		// recompute sN for this edge
		if(-d < 0.0)
			sN = 0.0;
		else if(-d > a)
			sN = sD;
		else
		{
			sN = -d;
			sD = a;
		}
	}
	else if(tN > tD)
	{
		// tN > tD => the t=1 edge is visible
		tN = tD;

		// recompute sN for this edge
		if((-d + b) < 0.0)
			sN = 0;
		else if((-d + b) > a)
			sN = sD;
		else
		{
			sN = (-d + b);
			sD = a;
		}
	}

	// finally do the division to get *s and *t
	*s = (fabs(sN) < LINE_DISTANCE_EPSILON ? 0.0 : sN / sD);
	*t = (fabs(tN) < LINE_DISTANCE_EPSILON ? 0.0 : tN / tD);

	// get the difference of the two closest points
	VectorScale(sMag, *s, sMag);
	VectorScale(tMag, *t, tMag);
	VectorAdd(diff, sMag, separation);
	VectorSubtract(separation, tMag, separation);

	return VectorLengthSquared(separation);
}

/*
================
DistanceBetweenLineSegments

Return the smallest distance between two line segments
================
*/
vec_t DistanceBetweenLineSegments(const vec3_t sP0, const vec3_t sP1, const vec3_t tP0, const vec3_t tP1, float *s, float *t)
{
	return (vec_t) sqrt(DistanceBetweenLineSegmentsSquared(sP0, sP1, tP0, tP1, s, t));
}

void VectorMatrixInverse( void* DstMatrix, const void* SrcMatrix )
{
	typedef float Float4x4[ 4 ][ 4 ];
	const Float4x4 M;
	Float4x4 Result;
	float Det[ 4 ];
	Float4x4 Tmp;

	memcpy( ( void * )M, SrcMatrix, sizeof( Float4x4 ) );

	Tmp[ 0 ][ 0 ] = M[ 2 ][ 2 ] * M[ 3 ][ 3 ] - M[ 2 ][ 3 ] * M[ 3 ][ 2 ];
	Tmp[ 0 ][ 1 ] = M[ 1 ][ 2 ] * M[ 3 ][ 3 ] - M[ 1 ][ 3 ] * M[ 3 ][ 2 ];
	Tmp[ 0 ][ 2 ] = M[ 1 ][ 2 ] * M[ 2 ][ 3 ] - M[ 1 ][ 3 ] * M[ 2 ][ 2 ];

	Tmp[ 1 ][ 0 ] = M[ 2 ][ 2 ] * M[ 3 ][ 3 ] - M[ 2 ][ 3 ] * M[ 3 ][ 2 ];
	Tmp[ 1 ][ 1 ] = M[ 0 ][ 2 ] * M[ 3 ][ 3 ] - M[ 0 ][ 3 ] * M[ 3 ][ 2 ];
	Tmp[ 1 ][ 2 ] = M[ 0 ][ 2 ] * M[ 2 ][ 3 ] - M[ 0 ][ 3 ] * M[ 2 ][ 2 ];

	Tmp[ 2 ][ 0 ] = M[ 1 ][ 2 ] * M[ 3 ][ 3 ] - M[ 1 ][ 3 ] * M[ 3 ][ 2 ];
	Tmp[ 2 ][ 1 ] = M[ 0 ][ 2 ] * M[ 3 ][ 3 ] - M[ 0 ][ 3 ] * M[ 3 ][ 2 ];
	Tmp[ 2 ][ 2 ] = M[ 0 ][ 2 ] * M[ 1 ][ 3 ] - M[ 0 ][ 3 ] * M[ 1 ][ 2 ];

	Tmp[ 3 ][ 0 ] = M[ 1 ][ 2 ] * M[ 2 ][ 3 ] - M[ 1 ][ 3 ] * M[ 2 ][ 2 ];
	Tmp[ 3 ][ 1 ] = M[ 0 ][ 2 ] * M[ 2 ][ 3 ] - M[ 0 ][ 3 ] * M[ 2 ][ 2 ];
	Tmp[ 3 ][ 2 ] = M[ 0 ][ 2 ] * M[ 1 ][ 3 ] - M[ 0 ][ 3 ] * M[ 1 ][ 2 ];

	Det[ 0 ] = M[ 1 ][ 1 ] * Tmp[ 0 ][ 0 ] - M[ 2 ][ 1 ] * Tmp[ 0 ][ 1 ] + M[ 3 ][ 1 ] * Tmp[ 0 ][ 2 ];
	Det[ 1 ] = M[ 0 ][ 1 ] * Tmp[ 1 ][ 0 ] - M[ 2 ][ 1 ] * Tmp[ 1 ][ 1 ] + M[ 3 ][ 1 ] * Tmp[ 1 ][ 2 ];
	Det[ 2 ] = M[ 0 ][ 1 ] * Tmp[ 2 ][ 0 ] - M[ 1 ][ 1 ] * Tmp[ 2 ][ 1 ] + M[ 3 ][ 1 ] * Tmp[ 2 ][ 2 ];
	Det[ 3 ] = M[ 0 ][ 1 ] * Tmp[ 3 ][ 0 ] - M[ 1 ][ 1 ] * Tmp[ 3 ][ 1 ] + M[ 2 ][ 1 ] * Tmp[ 3 ][ 2 ];

	float Determinant = M[ 0 ][ 0 ] * Det[ 0 ] - M[ 1 ][ 0 ] * Det[ 1 ] + M[ 2 ][ 0 ] * Det[ 2 ] - M[ 3 ][ 0 ] * Det[ 3 ];
	const float	RDet = 1.0f / Determinant;

	Result[ 0 ][ 0 ] = RDet * Det[ 0 ];
	Result[ 0 ][ 1 ] = -RDet * Det[ 1 ];
	Result[ 0 ][ 2 ] = RDet * Det[ 2 ];
	Result[ 0 ][ 3 ] = -RDet * Det[ 3 ];
	Result[ 1 ][ 0 ] = -RDet * ( M[ 1 ][ 0 ] * Tmp[ 0 ][ 0 ] - M[ 2 ][ 0 ] * Tmp[ 0 ][ 1 ] + M[ 3 ][ 0 ] * Tmp[ 0 ][ 2 ] );
	Result[ 1 ][ 1 ] = RDet * ( M[ 0 ][ 0 ] * Tmp[ 1 ][ 0 ] - M[ 2 ][ 0 ] * Tmp[ 1 ][ 1 ] + M[ 3 ][ 0 ] * Tmp[ 1 ][ 2 ] );
	Result[ 1 ][ 2 ] = -RDet * ( M[ 0 ][ 0 ] * Tmp[ 2 ][ 0 ] - M[ 1 ][ 0 ] * Tmp[ 2 ][ 1 ] + M[ 3 ][ 0 ] * Tmp[ 2 ][ 2 ] );
	Result[ 1 ][ 3 ] = RDet * ( M[ 0 ][ 0 ] * Tmp[ 3 ][ 0 ] - M[ 1 ][ 0 ] * Tmp[ 3 ][ 1 ] + M[ 2 ][ 0 ] * Tmp[ 3 ][ 2 ] );
	Result[ 2 ][ 0 ] = RDet * (
		M[ 1 ][ 0 ] * ( M[ 2 ][ 1 ] * M[ 3 ][ 3 ] - M[ 2 ][ 3 ] * M[ 3 ][ 1 ] ) -
		M[ 2 ][ 0 ] * ( M[ 1 ][ 1 ] * M[ 3 ][ 3 ] - M[ 1 ][ 3 ] * M[ 3 ][ 1 ] ) +
		M[ 3 ][ 0 ] * ( M[ 1 ][ 1 ] * M[ 2 ][ 3 ] - M[ 1 ][ 3 ] * M[ 2 ][ 1 ] )
		);
	Result[ 2 ][ 1 ] = -RDet * (
		M[ 0 ][ 0 ] * ( M[ 2 ][ 1 ] * M[ 3 ][ 3 ] - M[ 2 ][ 3 ] * M[ 3 ][ 1 ] ) -
		M[ 2 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 3 ][ 3 ] - M[ 0 ][ 3 ] * M[ 3 ][ 1 ] ) +
		M[ 3 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 2 ][ 3 ] - M[ 0 ][ 3 ] * M[ 2 ][ 1 ] )
		);
	Result[ 2 ][ 2 ] = RDet * (
		M[ 0 ][ 0 ] * ( M[ 1 ][ 1 ] * M[ 3 ][ 3 ] - M[ 1 ][ 3 ] * M[ 3 ][ 1 ] ) -
		M[ 1 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 3 ][ 3 ] - M[ 0 ][ 3 ] * M[ 3 ][ 1 ] ) +
		M[ 3 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 1 ][ 3 ] - M[ 0 ][ 3 ] * M[ 1 ][ 1 ] )
		);
	Result[ 2 ][ 3 ] = -RDet * (
		M[ 0 ][ 0 ] * ( M[ 1 ][ 1 ] * M[ 2 ][ 3 ] - M[ 1 ][ 3 ] * M[ 2 ][ 1 ] ) -
		M[ 1 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 2 ][ 3 ] - M[ 0 ][ 3 ] * M[ 2 ][ 1 ] ) +
		M[ 2 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 1 ][ 3 ] - M[ 0 ][ 3 ] * M[ 1 ][ 1 ] )
		);
	Result[ 3 ][ 0 ] = -RDet * (
		M[ 1 ][ 0 ] * ( M[ 2 ][ 1 ] * M[ 3 ][ 2 ] - M[ 2 ][ 2 ] * M[ 3 ][ 1 ] ) -
		M[ 2 ][ 0 ] * ( M[ 1 ][ 1 ] * M[ 3 ][ 2 ] - M[ 1 ][ 2 ] * M[ 3 ][ 1 ] ) +
		M[ 3 ][ 0 ] * ( M[ 1 ][ 1 ] * M[ 2 ][ 2 ] - M[ 1 ][ 2 ] * M[ 2 ][ 1 ] )
		);
	Result[ 3 ][ 1 ] = RDet * (
		M[ 0 ][ 0 ] * ( M[ 2 ][ 1 ] * M[ 3 ][ 2 ] - M[ 2 ][ 2 ] * M[ 3 ][ 1 ] ) -
		M[ 2 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 3 ][ 2 ] - M[ 0 ][ 2 ] * M[ 3 ][ 1 ] ) +
		M[ 3 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 2 ][ 2 ] - M[ 0 ][ 2 ] * M[ 2 ][ 1 ] )
		);
	Result[ 3 ][ 2 ] = -RDet * (
		M[ 0 ][ 0 ] * ( M[ 1 ][ 1 ] * M[ 3 ][ 2 ] - M[ 1 ][ 2 ] * M[ 3 ][ 1 ] ) -
		M[ 1 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 3 ][ 2 ] - M[ 0 ][ 2 ] * M[ 3 ][ 1 ] ) +
		M[ 3 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 1 ][ 2 ] - M[ 0 ][ 2 ] * M[ 1 ][ 1 ] )
		);
	Result[ 3 ][ 3 ] = RDet * (
		M[ 0 ][ 0 ] * ( M[ 1 ][ 1 ] * M[ 2 ][ 2 ] - M[ 1 ][ 2 ] * M[ 2 ][ 1 ] ) -
		M[ 1 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 2 ][ 2 ] - M[ 0 ][ 2 ] * M[ 2 ][ 1 ] ) +
		M[ 2 ][ 0 ] * ( M[ 0 ][ 1 ] * M[ 1 ][ 2 ] - M[ 0 ][ 2 ] * M[ 1 ][ 1 ] )
		);

	memcpy( DstMatrix, &Result, 16 * sizeof( float ) );
}

// *INDENT-OFF*
void MatrixIdentity(matrix_t m)
{
    m[ 0] = 1;      m[ 4] = 0;      m[ 8] = 0;      m[12] = 0;
	m[ 1] = 0;      m[ 5] = 1;      m[ 9] = 0;      m[13] = 0;
	m[ 2] = 0;      m[ 6] = 0;      m[10] = 1;      m[14] = 0;
	m[ 3] = 0;      m[ 7] = 0;      m[11] = 0;      m[15] = 1;
}

void MatrixClear(matrix_t m)
{
    m[ 0] = 0;      m[ 4] = 0;      m[ 8] = 0;      m[12] = 0;
	m[ 1] = 0;      m[ 5] = 0;      m[ 9] = 0;      m[13] = 0;
	m[ 2] = 0;      m[ 6] = 0;      m[10] = 0;      m[14] = 0;
	m[ 3] = 0;      m[ 7] = 0;      m[11] = 0;      m[15] = 0;
}


void MatrixCopy(const matrix_t in, matrix_t out)
{
#if id386_sse && defined __GNUC__ && 0
	asm volatile
	(
	"movups         (%%edx),        %%xmm0\n"
	"movups         0x10(%%edx),    %%xmm1\n"
	"movups         0x20(%%edx),    %%xmm2\n"
	"movups         0x30(%%edx),    %%xmm3\n"

	"movups         %%xmm0,         (%%eax)\n"
	"movups         %%xmm1,         0x10(%%eax)\n"
	"movups         %%xmm2,         0x20(%%eax)\n"
	"movups         %%xmm3,         0x30(%%eax)\n"
	:
	: "a"( out ), "d"( in )
	: "memory"
		);
#elif id386_3dnow && defined __GNUC__
	asm volatile
	(
	"femms\n"
	"movq           (%%edx),        %%mm0\n"
	"movq           8(%%edx),       %%mm1\n"
	"movq           16(%%edx),      %%mm2\n"
	"movq           24(%%edx),      %%mm3\n"
	"movq           32(%%edx),      %%mm4\n"
	"movq           40(%%edx),      %%mm5\n"
	"movq           48(%%edx),      %%mm6\n"
	"movq           56(%%edx),      %%mm7\n"

	"movq           %%mm0,          (%%eax)\n"
	"movq           %%mm1,          8(%%eax)\n"
	"movq           %%mm2,          16(%%eax)\n"
	"movq           %%mm3,          24(%%eax)\n"
	"movq           %%mm4,          32(%%eax)\n"
	"movq           %%mm5,          40(%%eax)\n"
	"movq           %%mm6,          48(%%eax)\n"
	"movq           %%mm7,          56(%%eax)\n"
	"femms\n"
	:
	: "a"(out), "d"(in)
	: "memory"
	);
#else
        out[ 0] = in[ 0];       out[ 4] = in[ 4];       out[ 8] = in[ 8];       out[12] = in[12];
        out[ 1] = in[ 1];       out[ 5] = in[ 5];       out[ 9] = in[ 9];       out[13] = in[13];
		out[ 2] = in[ 2];       out[ 6] = in[ 6];       out[10] = in[10];       out[14] = in[14];
		out[ 3] = in[ 3];       out[ 7] = in[ 7];       out[11] = in[11];       out[15] = in[15];
#endif
}

qboolean MatrixCompare(const matrix_t a, const matrix_t b)
{
	return (a[ 0] == b[ 0] && a[ 4] == b[ 4] && a[ 8] == b[ 8] && a[12] == b[12] &&
            a[ 1] == b[ 1] && a[ 5] == b[ 5] && a[ 9] == b[ 9] && a[13] == b[13] &&
		    a[ 2] == b[ 2] && a[ 6] == b[ 6] && a[10] == b[10] && a[14] == b[14] &&
		    a[ 3] == b[ 3] && a[ 7] == b[ 7] && a[11] == b[11] && a[15] == b[15]);
}

void MatrixTransposeIntoXMM(const matrix_t m)
{
#if id386_sse && defined __GNUC__ && 0
        asm volatile
        (                                                                               // reg[0]                       | reg[1]                | reg[2]                | reg[3]
        // load transpose into XMM registers
        "movlps         (%%eax),        %%xmm4\n"               // m[0][0]                      | m[0][1]               | -                     | -
        "movhps         16(%%eax),      %%xmm4\n"               // m[0][0]                      | m[0][1]               | m[1][0]               | m[1][1]

        "movlps         32(%%eax),      %%xmm3\n"               // m[2][0]                      | m[2][1]               | -                     | -
        "movhps         48(%%eax),      %%xmm3\n"               // m[2][0]                      | m[2][1]               | m[3][0]               | m[3][1]

        "movups         %%xmm4,         %%xmm5\n"               // m[0][0]                      | m[0][1]               | m[1][0]               | m[1][1]

        // 0x88 = 10 00 | 10 00 <-> 00 10 | 00 10          xmm4[00]                       xmm4[10]                xmm3[00]                xmm3[10]
        "shufps         $0x88, %%xmm3,  %%xmm4\n"       // m[0][0]                      | m[1][0]               | m[2][0]               | m[3][0]

        // 0xDD = 11 01 | 11 01 <-> 01 11 | 01 11          xmm5[01]                       xmm5[11]                xmm3[01]                xmm3[11]
        "shufps         $0xDD, %%xmm3,  %%xmm5\n"       // m[0][1]                      | m[1][1]               | m[2][1]               | m[3][1]

        "movlps         8(%%eax),       %%xmm6\n"               // m[0][2]                      | m[0][3]               | -                     | -
        "movhps         24(%%eax),      %%xmm6\n"               // m[0][2]                      | m[0][3]               | m[1][2]               | m[1][3]

        "movlps         40(%%eax),      %%xmm3\n"               // m[2][2]                      | m[2][3]               | -                     | -
        "movhps         56(%%eax),      %%xmm3\n"               // m[2][2]                      | m[2][3]               | m[3][2]               | m[3][3]

        "movups         %%xmm6,         %%xmm7\n"               // m[0][2]                      | m[0][3]               | m[1][2]               | m[1][3]

        // 0x88 = 10 00 | 10 00 <-> 00 10 | 00 10          xmm6[00]                       xmm6[10]                xmm3[00]                xmm3[10]
        "shufps         $0x88, %%xmm3,  %%xmm6\n"       // m[0][2]                      | m[1][2]               | m[2][2]               | m[3][2]

        // 0xDD = 11 01 | 11 01 <-> 01 11 | 01 11          xmm7[01]                       xmm7[11]                xmm3[01]                xmm3[11]
        "shufps         $0xDD, %%xmm3,  %%xmm7\n"       // m[0][3]                      | m[1][3]               | m[2][3]               | m[3][3]
	:
	: "a"( m )
	: "memory"
		);
#endif
}

void MatrixTranspose(const matrix_t in, matrix_t out)
{
#if id386_sse && defined __GNUC__ && 0
        // transpose the matrix into the xmm4-7
        MatrixTransposeIntoXMM(in);

        asm volatile
				(
				"movups         %%xmm4,         (%%eax)\n"
				"movups         %%xmm5,         0x10(%%eax)\n"
				"movups         %%xmm6,         0x20(%%eax)\n"
				"movups         %%xmm7,         0x30(%%eax)\n"
	:
	: "a"( out )
	: "memory"
				);
#else
	out[ 0] = in[ 0];       out[ 1] = in[ 4];       out[ 2] = in[ 8];       out[ 3] = in[12];
	out[ 4] = in[ 1];       out[ 5] = in[ 5];       out[ 6] = in[ 9];       out[ 7] = in[13];
	out[ 8] = in[ 2];       out[ 9] = in[ 6];       out[10] = in[10];       out[11] = in[14];
	out[12] = in[ 3];       out[13] = in[ 7];       out[14] = in[11];       out[15] = in[15];
#endif
}


// helper functions for MatrixInverse from GtkRadiant C mathlib
static float m3_det( matrix3x3_t mat )
{
  float det;

  det = mat[0] * ( mat[4]*mat[8] - mat[7]*mat[5] )
    - mat[1] * ( mat[3]*mat[8] - mat[6]*mat[5] )
    + mat[2] * ( mat[3]*mat[7] - mat[6]*mat[4] );

  return( det );
}

/*static int m3_inverse( matrix3x3_t mr, matrix3x3_t ma )
{
  float det = m3_det( ma );

  if (det == 0 )
  {
    return 1;
  }


  mr[0] =    ma[4]*ma[8] - ma[5]*ma[7]   / det;
  mr[1] = -( ma[1]*ma[8] - ma[7]*ma[2] ) / det;
  mr[2] =    ma[1]*ma[5] - ma[4]*ma[2]   / det;

  mr[3] = -( ma[3]*ma[8] - ma[5]*ma[6] ) / det;
  mr[4] =    ma[0]*ma[8] - ma[6]*ma[2]   / det;
  mr[5] = -( ma[0]*ma[5] - ma[3]*ma[2] ) / det;

  mr[6] =    ma[3]*ma[7] - ma[6]*ma[4]   / det;
  mr[7] = -( ma[0]*ma[7] - ma[6]*ma[1] ) / det;
  mr[8] =    ma[0]*ma[4] - ma[1]*ma[3]   / det;

  return 0;
}*/

static void m4_submat( matrix_t mr, matrix3x3_t mb, int i, int j )
{
  int ti, tj, idst = 0, jdst = 0;

  for ( ti = 0; ti < 4; ti++ )
  {
    if ( ti < i )
      idst = ti;
    else
      if ( ti > i )
        idst = ti-1;

      for ( tj = 0; tj < 4; tj++ )
      {
        if ( tj < j )
          jdst = tj;
        else
          if ( tj > j )
            jdst = tj-1;

          if ( ti != i && tj != j )
            mb[idst*3 + jdst] = mr[ti*4 + tj ];
      }
  }
}

static float m4_det( matrix_t mr )
{
  float  det, result = 0, i = 1;
  matrix3x3_t msub3;
  int     n;

  for ( n = 0; n < 4; n++, i *= -1 )
  {
    m4_submat( mr, msub3, 0, n );

    det     = m3_det( msub3 );
    result += mr[n] * det * i;
  }

  return result;
}

qboolean MatrixInverse(matrix_t matrix)
{
  float  mdet = m4_det(matrix);
  matrix3x3_t mtemp;
  int     i, j, sign;
  matrix_t m4x4_temp;

#if 0
  if ( fabs( mdet ) < 0.0000000001 )
    return qtrue;
#endif

  MatrixCopy(matrix, m4x4_temp);

  for ( i = 0; i < 4; i++ )
    for ( j = 0; j < 4; j++ )
    {
      sign = 1 - ( (i +j) % 2 ) * 2;

      m4_submat( m4x4_temp, mtemp, i, j );

	  // FIXME: try using * inverse det and see if speed/accuracy are good enough
      matrix[i+j*4] = ( m3_det( mtemp ) * sign ) / mdet;
    }

  return qfalse;
}

void MatrixSetupXRotation(matrix_t m, vec_t degrees)
{
	vec_t a = DEG2RAD(degrees);

	m[ 0] = 1;      m[ 4] = 0;              m[ 8] = 0;              m[12] = 0;
	m[ 1] = 0;      m[ 5] = cos(a);         m[ 9] =-sin(a);         m[13] = 0;
	m[ 2] = 0;      m[ 6] = sin(a);         m[10] = cos(a);         m[14] = 0;
	m[ 3] = 0;      m[ 7] = 0;              m[11] = 0;              m[15] = 1;
}

void MatrixSetupYRotation(matrix_t m, vec_t degrees)
{
	vec_t a = DEG2RAD(degrees);

	m[ 0] = cos(a);         m[ 4] = 0;      m[ 8] = sin(a);         m[12] = 0;
	m[ 1] = 0;              m[ 5] = 1;      m[ 9] = 0;              m[13] = 0;
	m[ 2] =-sin(a);         m[ 6] = 0;      m[10] = cos(a);         m[14] = 0;
	m[ 3] = 0;              m[ 7] = 0;      m[11] = 0;              m[15] = 1;
}

void MatrixSetupZRotation(matrix_t m, vec_t degrees)
{
	vec_t a = DEG2RAD(degrees);

	m[ 0] = cos(a);         m[ 4] =-sin(a);         m[ 8] = 0;      m[12] = 0;
	m[ 1] = sin(a);         m[ 5] = cos(a);         m[ 9] = 0;      m[13] = 0;
	m[ 2] = 0;              m[ 6] = 0;              m[10] = 1;      m[14] = 0;
	m[ 3] = 0;              m[ 7] = 0;              m[11] = 0;      m[15] = 1;
}

void MatrixSetupTranslation(matrix_t m, vec_t x, vec_t y, vec_t z)
{
	m[ 0] = 1;      m[ 4] = 0;      m[ 8] = 0;      m[12] = x;
	m[ 1] = 0;      m[ 5] = 1;      m[ 9] = 0;      m[13] = y;
	m[ 2] = 0;      m[ 6] = 0;      m[10] = 1;      m[14] = z;
	m[ 3] = 0;      m[ 7] = 0;      m[11] = 0;      m[15] = 1;
}

void MatrixSetupScale(matrix_t m, vec_t x, vec_t y, vec_t z)
{
	m[ 0] = x;      m[ 4] = 0;      m[ 8] = 0;      m[12] = 0;
	m[ 1] = 0;      m[ 5] = y;      m[ 9] = 0;      m[13] = 0;
	m[ 2] = 0;      m[ 6] = 0;      m[10] = z;      m[14] = 0;
	m[ 3] = 0;      m[ 7] = 0;      m[11] = 0;      m[15] = 1;
}

void MatrixSetupShear(matrix_t m, vec_t x, vec_t y)
{
	m[ 0] = 1;      m[ 4] = x;      m[ 8] = 0;      m[12] = 0;
	m[ 1] = y;      m[ 5] = 1;      m[ 9] = 0;      m[13] = 0;
	m[ 2] = 0;      m[ 6] = 0;      m[10] = 1;      m[14] = 0;
	m[ 3] = 0;      m[ 7] = 0;      m[11] = 0;      m[15] = 1;
}

void Matrix4x4Multiply(const matrix_t a, const matrix_t b, matrix_t out)
{
#if id386_sse
//#error Matrix3x3Multiply
	int				i;
	__m128			_t0, _t1, _t2, _t3, _t4, _t5, _t6, _t7;

	_t4 = _mm_loadu_ps(&a[0]);
	_t5 = _mm_loadu_ps(&a[4]);
	_t6 = _mm_loadu_ps(&a[8]);
	_t7 = _mm_loadu_ps(&a[12]);

	for(i = 0; i < 4; i++)
	{
		_t0 = _mm_load1_ps(&b[i * 4 + 0]);
		_t0 = _mm_mul_ps(_t4, _t0);

		_t1 = _mm_load1_ps(&b[i * 4 + 1]);
		_t1 = _mm_mul_ps(_t5, _t1);

		_t2 = _mm_load1_ps(&b[i * 4 + 2]);
		_t2 = _mm_mul_ps(_t6, _t2);

		_t3 = _mm_load1_ps(&b[i * 4 + 3]);
		_t3 = _mm_mul_ps(_t7, _t3);

		_t1 = _mm_add_ps(_t0, _t1);
		_t2 = _mm_add_ps(_t1, _t2);
		_t3 = _mm_add_ps(_t2, _t3);

		_mm_storeu_ps(&out[i * 4], _t3);
	}

#else
        out[ 0] = b[ 0]*a[ 0] + b[ 1]*a[ 4] + b[ 2]*a[ 8] + b[ 3]*a[12];
        out[ 1] = b[ 0]*a[ 1] + b[ 1]*a[ 5] + b[ 2]*a[ 9] + b[ 3]*a[13];
		out[ 2] = b[ 0]*a[ 2] + b[ 1]*a[ 6] + b[ 2]*a[10] + b[ 3]*a[14];
		out[ 3] = b[ 0]*a[ 3] + b[ 1]*a[ 7] + b[ 2]*a[11] + b[ 3]*a[15];

		out[ 4] = b[ 4]*a[ 0] + b[ 5]*a[ 4] + b[ 6]*a[ 8] + b[ 7]*a[12];
		out[ 5] = b[ 4]*a[ 1] + b[ 5]*a[ 5] + b[ 6]*a[ 9] + b[ 7]*a[13];
		out[ 6] = b[ 4]*a[ 2] + b[ 5]*a[ 6] + b[ 6]*a[10] + b[ 7]*a[14];
		out[ 7] = b[ 4]*a[ 3] + b[ 5]*a[ 7] + b[ 6]*a[11] + b[ 7]*a[15];

		out[ 8] = b[ 8]*a[ 0] + b[ 9]*a[ 4] + b[10]*a[ 8] + b[11]*a[12];
		out[ 9] = b[ 8]*a[ 1] + b[ 9]*a[ 5] + b[10]*a[ 9] + b[11]*a[13];
		out[10] = b[ 8]*a[ 2] + b[ 9]*a[ 6] + b[10]*a[10] + b[11]*a[14];
		out[11] = b[ 8]*a[ 3] + b[ 9]*a[ 7] + b[10]*a[11] + b[11]*a[15];

		out[12] = b[12]*a[ 0] + b[13]*a[ 4] + b[14]*a[ 8] + b[15]*a[12];
		out[13] = b[12]*a[ 1] + b[13]*a[ 5] + b[14]*a[ 9] + b[15]*a[13];
		out[14] = b[12]*a[ 2] + b[13]*a[ 6] + b[14]*a[10] + b[15]*a[14];
		out[15] = b[12]*a[ 3] + b[13]*a[ 7] + b[14]*a[11] + b[15]*a[15];
#endif
}

/*
================
MatrixMultiply
================
*/
void MatrixMultiply( const float in1[ 3 ][ 3 ], const float in2[ 3 ][ 3 ], float out[ 3 ][ 3 ] ) {
	out[ 0 ][ 0 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 0 ][ 1 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 0 ][ 2 ] = in1[ 0 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 0 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 0 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 1 ][ 0 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 1 ][ 1 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 1 ][ 2 ] = in1[ 1 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 1 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 1 ][ 2 ] * in2[ 2 ][ 2 ];
	out[ 2 ][ 0 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 0 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 0 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 0 ];
	out[ 2 ][ 1 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 1 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 1 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 1 ];
	out[ 2 ][ 2 ] = in1[ 2 ][ 0 ] * in2[ 0 ][ 2 ] + in1[ 2 ][ 1 ] * in2[ 1 ][ 2 ] +
		in1[ 2 ][ 2 ] * in2[ 2 ][ 2 ];
}

void MatrixMultiply2(matrix_t m, const matrix_t m2)
{
	matrix_t        tmp;

	MatrixCopy(m, tmp);
	Matrix4x4Multiply(tmp, m2, m);
}

void MatrixMultiplyRotation(matrix_t m, vec_t pitch, vec_t yaw, vec_t roll)
{
	matrix_t        tmp, rot;

	MatrixCopy(m, tmp);
	MatrixFromAngles(rot, pitch, yaw, roll);

	Matrix4x4Multiply(tmp, rot, m);
}

void MatrixMultiplyZRotation(matrix_t m, vec_t degrees)
{
	matrix_t        tmp, rot;

	MatrixCopy(m, tmp);
	MatrixSetupZRotation(rot, degrees);

	Matrix4x4Multiply(tmp, rot, m);
}

void MatrixMultiplyTranslation(matrix_t m, vec_t x, vec_t y, vec_t z)
{
#if 1
	matrix_t        tmp, trans;

	MatrixCopy(m, tmp);
	MatrixSetupTranslation(trans, x, y, z);
	Matrix4x4Multiply(tmp, trans, m);
#else
	m[12] += m[ 0] * x + m[ 4] * y + m[ 8] * z;
	m[13] += m[ 1] * x + m[ 5] * y + m[ 9] * z;
	m[14] += m[ 2] * x + m[ 6] * y + m[10] * z;
	m[15] += m[ 3] * x + m[ 7] * y + m[11] * z;
#endif
}

void MatrixMultiplyScale(matrix_t m, vec_t x, vec_t y, vec_t z)
{
#if 0
	matrix_t        tmp, scale;

	MatrixCopy(m, tmp);
	MatrixSetupScale(scale, x, y, z);
	Matrix4x4Multiply(tmp, scale, m);
#else
	m[ 0] *= x;     m[ 4] *= y;        m[ 8] *= z;
	m[ 1] *= x;     m[ 5] *= y;        m[ 9] *= z;
	m[ 2] *= x;     m[ 6] *= y;        m[10] *= z;
	m[ 3] *= x;     m[ 7] *= y;        m[11] *= z;
#endif
}

void MatrixMultiplyShear(matrix_t m, vec_t x, vec_t y)
{
	matrix_t        tmp, shear;

	MatrixCopy(m, tmp);
	MatrixSetupShear(shear, x, y);
	Matrix4x4Multiply(tmp, shear, m);
}

#ifndef FLT_EPSILON
#define FLT_EPSILON 1.19209290E-07F
#endif
void MatrixToAngles(const matrix_t m, vec3_t angles)
{
#if 1
	double          theta;
	double          cp;
	double          sp;

	sp = m[2];

	// cap off our sin value so that we don't get any NANs
	if(sp > 1.0)
	{
		sp = 1.0;
	}
	else if(sp < -1.0)
	{
		sp = -1.0;
	}

	theta = -asin(sp);
	cp = cos(theta);

	if(cp > 8192 * FLT_EPSILON)
	{
		angles[PITCH] = RAD2DEG(theta);
		angles[YAW] = RAD2DEG(atan2(m[1], m[0]));
		angles[ROLL] = RAD2DEG(atan2(m[6], m[10]));
	}
	else
	{
		angles[PITCH] = RAD2DEG(theta);
		angles[YAW] = RAD2DEG(-atan2(m[4], m[5]));
		angles[ROLL] = 0;
	}
#else
	double          a;
	double          ca;

	a = asin(-m[2]);
	ca = cos(a);

	if(fabs(ca) > 0.005)		// Gimbal lock?
	{
		angles[PITCH] = RAD2DEG(atan2(m[6] / ca, m[10] / ca));
		angles[YAW] = RAD2DEG(a);
		angles[ROLL] = RAD2DEG(atan2(m[1] / ca, m[0] / ca));
	}
	else
	{
		// Gimbal lock has occurred
		angles[PITCH] = RAD2DEG(atan2(-m[9], m[5]));
		angles[YAW] = RAD2DEG(a);
		angles[ROLL] = 0;
	}
#endif
}


void MatrixFromAngles(matrix_t m, vec_t pitch, vec_t yaw, vec_t roll)
{
	static float    sr, sp, sy, cr, cp, cy;

    // static to help MS compiler fp bugs
	sp = sin(DEG2RAD(pitch));
	cp = cos(DEG2RAD(pitch));

	sy = sin(DEG2RAD(yaw));
	cy = cos(DEG2RAD(yaw));

	sr = sin(DEG2RAD(roll));
	cr = cos(DEG2RAD(roll));

	m[ 0] = cp*cy;  m[ 4] = (sr*sp*cy+cr*-sy);      m[ 8] = (cr*sp*cy+-sr*-sy);     m[12] = 0;
	m[ 1] = cp*sy;  m[ 5] = (sr*sp*sy+cr*cy);       m[ 9] = (cr*sp*sy+-sr*cy);      m[13] = 0;
	m[ 2] = -sp;    m[ 6] = sr*cp;                  m[10] = cr*cp;                  m[14] = 0;
	m[ 3] = 0;      m[ 7] = 0;                      m[11] = 0;                      m[15] = 1;
}

void MatrixFromVectorsFLU(matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up)
{
    m[ 0] = forward[0];     m[ 4] = left[0];        m[ 8] = up[0];  m[12] = 0;
	m[ 1] = forward[1];     m[ 5] = left[1];        m[ 9] = up[1];  m[13] = 0;
	m[ 2] = forward[2];     m[ 6] = left[2];        m[10] = up[2];  m[14] = 0;
	m[ 3] = 0;              m[ 7] = 0;              m[11] = 0;      m[15] = 1;
}

void MatrixFromVectorsFRU(matrix_t m, const vec3_t forward, const vec3_t right, const vec3_t up)
{
    m[ 0] = forward[0];     m[ 4] =-right[0];       m[ 8] = up[0];  m[12] = 0;
	m[ 1] = forward[1];     m[ 5] =-right[1];       m[ 9] = up[1];  m[13] = 0;
	m[ 2] = forward[2];     m[ 6] =-right[2];       m[10] = up[2];  m[14] = 0;
	m[ 3] = 0;              m[ 7] = 0;              m[11] = 0;      m[15] = 1;
}

void MatrixFromQuat(matrix_t m, const quat_t q)
{
#if 1
	/*
	From Quaternion to Matrix and Back
	February 27th 2005
	J.M.P. van Waveren

	http://www.intel.com/cd/ids/developer/asmo-na/eng/293748.htm
	*/
	float			x2, y2, z2, w2;
	float			yy2, xy2;
	float			xz2, yz2, zz2;
	float			wz2, wy2, wx2, xx2;

	x2 = q[0] + q[0];
	y2 = q[1] + q[1];
	z2 = q[2] + q[2];
	w2 = q[3] + q[3];

	yy2 = q[1] * y2;
	xy2 = q[0] * y2;

	xz2 = q[0] * z2;
	yz2 = q[1] * z2;
	zz2 = q[2] * z2;

	wz2 = q[3] * z2;
	wy2 = q[3] * y2;
	wx2 = q[3] * x2;
	xx2 = q[0] * x2;

	m[ 0] = - yy2 - zz2 + 1.0f;
	m[ 1] =   xy2 + wz2;
	m[ 2] =   xz2 - wy2;

	m[ 4] =   xy2 - wz2;
	m[ 5] = - xx2 - zz2 + 1.0f;
	m[ 6] =   yz2 + wx2;

	m[ 8] =   xz2 + wy2;
	m[ 9] =   yz2 - wx2;
	m[10] = - xx2 - yy2 + 1.0f;

	m[ 3] = m[ 7] = m[11] = m[12] = m[13] = m[14] = 0;
    m[15] = 1;

#else
	/*
	http://www.gamedev.net/reference/articles/article1691.asp#Q54
	Q54. How do I convert a quaternion to a rotation matrix?

	Assuming that a quaternion has been created in the form:

    Q = |X Y Z W|

    Then the quaternion can then be converted into a 4x4 rotation
    matrix using the following expression (Warning: you might have to
    transpose this matrix if you (do not) follow the OpenGL order!):

         ?        2     2                                      ?
         ? 1 - (2Y  + 2Z )   2XY - 2ZW         2XZ + 2YW       ?
         ?                                                     ?
         ?                          2     2                    ?
     M = ? 2XY + 2ZW         1 - (2X  + 2Z )   2YZ - 2XW       ?
         ?                                                     ?
         ?                                            2     2  ?
         ? 2XZ - 2YW         2YZ + 2XW         1 - (2X  + 2Y ) ?
         ?                                                     ?
	*/

	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm

	float			xx, xy, xz, xw, yy, yz, yw, zz, zw;

	xx = q[0] * q[0];
    xy = q[0] * q[1];
    xz = q[0] * q[2];
    xw = q[0] * q[3];
    yy = q[1] * q[1];
    yz = q[1] * q[2];
    yw = q[1] * q[3];
    zz = q[2] * q[2];
    zw = q[2] * q[3];

    m[ 0] = 1 - 2 * ( yy + zz );
    m[ 1] =     2 * ( xy + zw );
    m[ 2] =     2 * ( xz - yw );
    m[ 4] =     2 * ( xy - zw );
    m[ 5] = 1 - 2 * ( xx + zz );
    m[ 6] =     2 * ( yz + xw );
    m[ 8] =     2 * ( xz + yw );
    m[ 9] =     2 * ( yz - xw );
    m[10] = 1 - 2 * ( xx + yy );

    m[ 3] = m[ 7] = m[11] = m[12] = m[13] = m[14] = 0;
    m[15] = 1;
#endif
}

void MatrixFromPlanes(matrix_t m, const vec4_t left, const vec4_t right, const vec4_t bottom, const vec4_t top, const vec4_t near, const vec4_t far)
{
	m[ 0] = (right[0] - left[0]) / 2;
	m[ 1] = (top[0] - bottom[0]) / 2;
	m[ 2] = (far[0] - near[0]) / 2;
	m[ 3] = right[0] - (right[0] - left[0]) / 2;

	m[ 4] = (right[1] - left[1]) / 2;
	m[ 5] = (top[1] - bottom[1]) / 2;
	m[ 6] = (far[1] - near[1]) / 2;
	m[ 7] = right[1] - (right[1] - left[1]) / 2;

	m[ 8] = (right[2] - left[2]) / 2;
	m[ 9] = (top[2] - bottom[2]) / 2;
	m[10] = (far[2] - near[2]) / 2;
	m[11] = right[2] - (right[2] - left[2]) / 2;

#if 0
	m[12] = (right[3] - left[3]) / 2;
	m[13] = (top[3] - bottom[3]) / 2;
	m[14] = (far[3] - near[3]) / 2;
	m[15] = right[3] - (right[3] - left[3]) / 2;
#else
	m[12] = (-right[3] - -left[3]) / 2;
	m[13] = (-top[3] - -bottom[3]) / 2;
	m[14] = (-far[3] - -near[3]) / 2;
	m[15] = -right[3] - (-right[3] - -left[3]) / 2;
#endif
}

void MatrixToVectorsFLU(const matrix_t m, vec3_t forward, vec3_t left, vec3_t up)
{
	if(forward)
	{
		forward[0] = m[ 0];     // cp*cy;
		forward[1] = m[ 1];     // cp*sy;
		forward[2] = m[ 2];     //-sp;
	}

    if(left)
    {
		left[0] = m[ 4];        // sr*sp*cy+cr*-sy;
		left[1] = m[ 5];        // sr*sp*sy+cr*cy;
		left[2] = m[ 6];        // sr*cp;
    }

	if(up)
	{
		up[0] = m[ 8];  // cr*sp*cy+-sr*-sy;
		up[1] = m[ 9];  // cr*sp*sy+-sr*cy;
		up[2] = m[10];  // cr*cp;
	}
}

void MatrixToVectorsFRU(const matrix_t m, vec3_t forward, vec3_t right, vec3_t up)
{
	if(forward)
	{
		forward[0] = m[ 0];
		forward[1] = m[ 1];
		forward[2] = m[ 2];
	}

	if(right)
	{
		right[0] =-m[ 4];
		right[1] =-m[ 5];
		right[2] =-m[ 6];
	}

	if(up)
	{
		up[0] = m[ 8];
		up[1] = m[ 9];
		up[2] = m[10];
	}
}

void MatrixSetupTransformFromVectorsFLU(matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up, const vec3_t origin)
{
	m[ 0] = forward[0];     m[ 4] = left[0];        m[ 8] = up[0];  m[12] = origin[0];
	m[ 1] = forward[1];     m[ 5] = left[1];        m[ 9] = up[1];  m[13] = origin[1];
	m[ 2] = forward[2];     m[ 6] = left[2];        m[10] = up[2];  m[14] = origin[2];
	m[ 3] = 0;              m[ 7] = 0;              m[11] = 0;      m[15] = 1;
}

void MatrixSetupTransformFromVectorsFRU(matrix_t m, const vec3_t forward, const vec3_t right, const vec3_t up, const vec3_t origin)
{
	m[ 0] = forward[0];     m[ 4] = -right[0];        m[ 8] = up[0];  m[12] = origin[0];
	m[ 1] = forward[1];     m[ 5] = -right[1];        m[ 9] = up[1];  m[13] = origin[1];
	m[ 2] = forward[2];     m[ 6] = -right[2];        m[10] = up[2];  m[14] = origin[2];
	m[ 3] = 0;              m[ 7] = 0;             	  m[11] = 0;      m[15] = 1;
}

void MatrixSetupTransformFromRotation(matrix_t m, const matrix_t rot, const vec3_t origin)
{
	m[ 0] = rot[ 0];     m[ 4] = rot[ 4];        m[ 8] = rot[ 8];  m[12] = origin[0];
	m[ 1] = rot[ 1];     m[ 5] = rot[ 5];        m[ 9] = rot[ 9];  m[13] = origin[1];
	m[ 2] = rot[ 2];     m[ 6] = rot[ 6];        m[10] = rot[10];  m[14] = origin[2];
	m[ 3] = 0;           m[ 7] = 0;              m[11] = 0;        m[15] = 1;
}

void MatrixSetupTransformFromQuat(matrix_t m, const quat_t quat, const vec3_t origin)
{
	matrix_t        rot;

	MatrixFromQuat(rot, quat);

	m[ 0] = rot[ 0];     m[ 4] = rot[ 4];        m[ 8] = rot[ 8];  m[12] = origin[0];
	m[ 1] = rot[ 1];     m[ 5] = rot[ 5];        m[ 9] = rot[ 9];  m[13] = origin[1];
	m[ 2] = rot[ 2];     m[ 6] = rot[ 6];        m[10] = rot[10];  m[14] = origin[2];
	m[ 3] = 0;           m[ 7] = 0;              m[11] = 0;        m[15] = 1;
}

void MatrixAffineInverse(const matrix_t in, matrix_t out)
{
#if 0
		MatrixCopy(in, out);
		MatrixInverse(out);
#else
        // Tr3B - cleaned up
        out[ 0] = in[ 0];       out[ 4] = in[ 1];       out[ 8] = in[ 2];
        out[ 1] = in[ 4];       out[ 5] = in[ 5];       out[ 9] = in[ 6];
		out[ 2] = in[ 8];       out[ 6] = in[ 9];       out[10] = in[10];
		out[ 3] = 0;            out[ 7] = 0;            out[11] = 0;            out[15] = 1;

		out[12] = -( in[12] * out[ 0] + in[13] * out[ 4] + in[14] * out[ 8] );
		out[13] = -( in[12] * out[ 1] + in[13] * out[ 5] + in[14] * out[ 9] );
		out[14] = -( in[12] * out[ 2] + in[13] * out[ 6] + in[14] * out[10] );
#endif
}

void MatrixTransformNormal(const matrix_t m, const vec3_t in, vec3_t out)
{
	out[ 0] = m[ 0] * in[ 0] + m[ 4] * in[ 1] + m[ 8] * in[ 2];
	out[ 1] = m[ 1] * in[ 0] + m[ 5] * in[ 1] + m[ 9] * in[ 2];
	out[ 2] = m[ 2] * in[ 0] + m[ 6] * in[ 1] + m[10] * in[ 2];
}

void MatrixTransformNormal2(const matrix_t m, vec3_t inout)
{
	vec3_t          tmp;

	tmp[ 0] = m[ 0] * inout[ 0] + m[ 4] * inout[ 1] + m[ 8] * inout[ 2];
	tmp[ 1] = m[ 1] * inout[ 0] + m[ 5] * inout[ 1] + m[ 9] * inout[ 2];
	tmp[ 2] = m[ 2] * inout[ 0] + m[ 6] * inout[ 1] + m[10] * inout[ 2];

	VectorCopy(tmp, inout);
}

void MatrixTransformPoint(const matrix_t m, const vec3_t in, vec3_t out)
{
	out[ 0] = m[ 0] * in[ 0] + m[ 4] * in[ 1] + m[ 8] * in[ 2] + m[12];
	out[ 1] = m[ 1] * in[ 0] + m[ 5] * in[ 1] + m[ 9] * in[ 2] + m[13];
	out[ 2] = m[ 2] * in[ 0] + m[ 6] * in[ 1] + m[10] * in[ 2] + m[14];
}

void MatrixTransformPoint2(const matrix_t m, vec3_t inout)
{
	vec3_t          tmp;

	tmp[ 0] = m[ 0] * inout[ 0] + m[ 4] * inout[ 1] + m[ 8] * inout[ 2] + m[12];
	tmp[ 1] = m[ 1] * inout[ 0] + m[ 5] * inout[ 1] + m[ 9] * inout[ 2] + m[13];
	tmp[ 2] = m[ 2] * inout[ 0] + m[ 6] * inout[ 1] + m[10] * inout[ 2] + m[14];

	VectorCopy(tmp, inout);
}

void MatrixTransform4(const matrix_t m, const vec4_t in, vec4_t out)
{
#if id386_sse
//#error MatrixTransform4

	__m128			_t0, _t1, _t2, _x, _y, _z, _w, _m0, _m1, _m2, _m3;

	_m0 = _mm_loadu_ps(&m[0]);
	_m1 = _mm_loadu_ps(&m[4]);
	_m2 = _mm_loadu_ps(&m[8]);
	_m3 = _mm_loadu_ps(&m[12]);

	_t0 = _mm_loadu_ps(in);
	_x = _mm_shuffle_ps(_t0, _t0, _MM_SHUFFLE(0, 0, 0, 0));
	_y = _mm_shuffle_ps(_t0, _t0, _MM_SHUFFLE(1, 1, 1, 1));
	_z = _mm_shuffle_ps(_t0, _t0, _MM_SHUFFLE(2, 2, 2, 2));
	_w = _mm_shuffle_ps(_t0, _t0, _MM_SHUFFLE(3, 3, 3, 3));

	_t0 = _mm_mul_ps(_m3, _w);
	_t1 = _mm_mul_ps(_m2, _z);
	_t0 = _mm_add_ps(_t0, _t1);

	_t1 = _mm_mul_ps(_m1, _y);
	_t2 = _mm_mul_ps(_m0, _x);
	_t1 = _mm_add_ps(_t1, _t2);

	_t0 = _mm_add_ps(_t0, _t1);

	_mm_storeu_ps(out, _t0);
#else
	out[ 0] = m[ 0] * in[ 0] + m[ 4] * in[ 1] + m[ 8] * in[ 2] + m[12] * in[ 3];
	out[ 1] = m[ 1] * in[ 0] + m[ 5] * in[ 1] + m[ 9] * in[ 2] + m[13] * in[ 3];
	out[ 2] = m[ 2] * in[ 0] + m[ 6] * in[ 1] + m[10] * in[ 2] + m[14] * in[ 3];
	out[ 3] = m[ 3] * in[ 0] + m[ 7] * in[ 1] + m[11] * in[ 2] + m[15] * in[ 3];
#endif
}


void MatrixTransformPlane(const matrix_t m, const vec4_t in, vec4_t out)
{
	vec3_t			translation;
	vec3_t          planePos;

	// rotate the plane normal
	MatrixTransformNormal(m, in, out);

	// add new position to current plane position
	VectorSet(translation,  m[12], m[13], m[14]);
	VectorMA(translation, in[3], out, planePos);

	out[3] = DotProduct(out, planePos);
}

void MatrixTransformPlane2(const matrix_t m, vec4_t inout)
{
	vec4_t			tmp;

	MatrixTransformPlane(m, inout, tmp);
	VectorCopy4(tmp, inout);
}


/*
replacement for glFrustum
see glspec30.pdf chapter 2.12 Coordinate Transformations
*/
void MatrixPerspectiveProjection(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far)
{
	m[0] = (2 * near) / (right - left);	m[4] = 0;							m[8] = (right + left) / (right - left);		m[12] = 0;
	m[1] = 0;							m[5] = (2 * near) / (top - bottom);	m[9] = (top + bottom) / (top - bottom);		m[13] = 0;
	m[2] = 0;							m[6] = 0;							m[10] = -(far + near) / (far - near);		m[14] = -(2 * far * near) / (far - near);
	m[3] = 0;							m[7] = 0;							m[11] = -1;									m[15] = 0;
}

/*
same as D3DXMatrixPerspectiveOffCenterLH

http://msdn.microsoft.com/en-us/library/bb205353(VS.85).aspx
*/
void MatrixPerspectiveProjectionLH(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far)
{
	m[0] = (2 * near) / (right - left);	m[4] = 0;							m[8] = (left + right) / (left - right);		m[12] = 0;
	m[1] = 0;							m[5] = (2 * near) / (top - bottom);	m[9] = (top + bottom) / (bottom - top);		m[13] = 0;
	m[2] = 0;							m[6] = 0;							m[10] = far / (far - near);					m[14] = (near * far) / (near - far);
	m[3] = 0;							m[7] = 0;							m[11] = 1;									m[15] = 0;
}

/*
same as D3DXMatrixPerspectiveOffCenterRH

http://msdn.microsoft.com/en-us/library/bb205354(VS.85).aspx
*/
void MatrixPerspectiveProjectionRH(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far)
{
	m[0] = (2 * near) / (right - left);	m[4] = 0;							m[8] = (left + right) / (right - left);		m[12] = 0;
	m[1] = 0;							m[5] = (2 * near) / (top - bottom);	m[9] = (top + bottom) / (top - bottom);		m[13] = 0;
	m[2] = 0;							m[6] = 0;							m[10] = far / (near - far);					m[14] = (near * far) / (near - far);
	m[3] = 0;							m[7] = 0;							m[11] = -1;									m[15] = 0;
}

/*
same as D3DXMatrixPerspectiveFovLH

http://msdn.microsoft.com/en-us/library/bb205350(VS.85).aspx
*/
void MatrixPerspectiveProjectionFovYAspectLH(matrix_t m, vec_t fov, vec_t aspect, vec_t near, vec_t far)
{
	vec_t width, height;

	width = tanf(DEG2RAD(fov * 0.5f));
	height = width / aspect;

	m[0] = 1 / width;	m[4] = 0;			m[8] = 0;						m[12] = 0;
	m[1] = 0;			m[5] = 1 / height;	m[9] = 0;						m[13] = 0;
	m[2] = 0;			m[6] = 0;			m[10] = far / (far - near);		m[14] = -(near * far) / (far - near);
	m[3] = 0;			m[7] = 0;			m[11] = 1;						m[15] = 0;
}

void MatrixPerspectiveProjectionFovXYLH(matrix_t m, vec_t fovX, vec_t fovY, vec_t near, vec_t far)
{
	vec_t width, height;

	width = tanf(DEG2RAD(fovX * 0.5f));
	height = tanf(DEG2RAD(fovY * 0.5f));

	m[0] = 1 / width;	m[4] = 0;			m[8] = 0;						m[12] = 0;
	m[1] = 0;			m[5] = 1 / height;	m[9] = 0;						m[13] = 0;
	m[2] = 0;			m[6] = 0;			m[10] = far / (far - near);		m[14] = -(near * far) / (far - near);
	m[3] = 0;			m[7] = 0;			m[11] = 1;						m[15] = 0;
}

void MatrixPerspectiveProjectionFovXYRH(matrix_t m, vec_t fovX, vec_t fovY, vec_t near, vec_t far)
{
	vec_t width, height;

	width = tanf(DEG2RAD(fovX * 0.5f));
	height = tanf(DEG2RAD(fovY * 0.5f));

	m[0] = 1 / width;	m[4] = 0;			m[8] = 0;						m[12] = 0;
	m[1] = 0;			m[5] = 1 / height;	m[9] = 0;						m[13] = 0;
	m[2] = 0;			m[6] = 0;			m[10] = far / (near - far);		m[14] = (near * far) / (near - far);
	m[3] = 0;			m[7] = 0;			m[11] = -1;						m[15] = 0;
}

// Tr3B: far plane at infinity, see RobustShadowVolumes.pdf by Nvidia
void MatrixPerspectiveProjectionFovXYInfiniteRH(matrix_t m, vec_t fovX, vec_t fovY, vec_t near)
{
	vec_t width, height;

	width = tanf(DEG2RAD(fovX * 0.5f));
	height = tanf(DEG2RAD(fovY * 0.5f));

	m[0] = 1 / width;	m[4] = 0;			m[8] = 0;						m[12] = 0;
	m[1] = 0;			m[5] = 1 / height;	m[9] = 0;						m[13] = 0;
	m[2] = 0;			m[6] = 0;			m[10] = -1;						m[14] = -2 * near;
	m[3] = 0;			m[7] = 0;			m[11] = -1;						m[15] = 0;
}

/*
replacement for glOrtho
see glspec30.pdf chapter 2.12 Coordinate Transformations
*/
void MatrixOrthogonalProjection(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far)
{
	m[0] = 2 / (right - left);	m[4] = 0;					m[8] = 0;					m[12] = -(right + left) / (right - left);
	m[1] = 0;					m[5] = 2 / (top - bottom);	m[9] = 0;					m[13] = -(top + bottom) / (top - bottom);
	m[2] = 0;					m[6] = 0;					m[10] = -2 / (far - near);	m[14] = -(far + near) / (far - near);
	m[3] = 0;					m[7] = 0;					m[11] = 0;					m[15] = 1;
}

/*
same as D3DXMatrixOrthoOffCenterLH

http://msdn.microsoft.com/en-us/library/bb205347(VS.85).aspx
*/
void MatrixOrthogonalProjectionLH(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far)
{
	m[0] = 2 / (right - left);	m[4] = 0;					m[8] = 0;					m[12] = (left + right) / (left - right);
	m[1] = 0;					m[5] = 2 / (top - bottom);	m[9] = 0;					m[13] = (top + bottom) / (bottom - top);
	m[2] = 0;					m[6] = 0;					m[10] = 1 / (far - near);	m[14] = near / (near - far);
	m[3] = 0;					m[7] = 0;					m[11] = 0;					m[15] = 1;
}

/*
same as D3DXMatrixOrthoOffCenterRH

http://msdn.microsoft.com/en-us/library/bb205348(VS.85).aspx
*/
void MatrixOrthogonalProjectionRH(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far)
{
	m[0] = 2 / (right - left);	m[4] = 0;					m[8] = 0;					m[12] = (left + right) / (left - right);
	m[1] = 0;					m[5] = 2 / (top - bottom);	m[9] = 0;					m[13] = (top + bottom) / (bottom - top);
	m[2] = 0;					m[6] = 0;					m[10] = 1 / (near - far);	m[14] = near / (near - far);
	m[3] = 0;					m[7] = 0;					m[11] = 0;					m[15] = 1;
}

/*
same as D3DXMatrixReflect

http://msdn.microsoft.com/en-us/library/bb205356%28v=VS.85%29.aspx
*/
void MatrixPlaneReflection(matrix_t m, const vec4_t plane)
{
	vec4_t P;
	VectorCopy4(plane, P);

	PlaneNormalize(P);

	/*
	-2 * P.a * P.a + 1  -2 * P.b * P.a      -2 * P.c * P.a        0
	-2 * P.a * P.b      -2 * P.b * P.b + 1  -2 * P.c * P.b        0
	-2 * P.a * P.c      -2 * P.b * P.c      -2 * P.c * P.c + 1    0
	-2 * P.a * P.d      -2 * P.b * P.d      -2 * P.c * P.d        1
	 */

	// Quake uses a different plane equation
	m[0] = -2 * P[0] * P[0] + 1;	m[4] = -2 * P[0] * P[1];			m[8]  = -2 * P[0] * P[2];		m[12] = 2 * P[0] * P[3];
	m[1] = -2 * P[1] * P[0];		m[5] = -2 * P[1] * P[1] + 1;		m[9]  = -2 * P[1] * P[2];		m[13] = 2 * P[1] * P[3];
	m[2] = -2 * P[2] * P[0];		m[6] = -2 * P[2] * P[1];			m[10] = -2 * P[2] * P[2] + 1;	m[14] = 2 * P[2] * P[3];
	m[3] = 0;						m[7] = 0;							m[11] = 0;						m[15] = 1;

#if 0
	matrix_t m2;
	MatrixCopy(m, m2);
	MatrixTranspose(m2, m);
#endif
}

void MatrixLookAtLH(matrix_t m, const vec3_t eye, const vec3_t dir, const vec3_t up)
{
	vec3_t dirN;
	vec3_t upN;
	vec3_t sideN;

#if 1
	CrossProduct(up, dir, sideN);
	VectorNormalize(sideN);

	CrossProduct(dir, sideN, upN);
	VectorNormalize(upN);
#else
	CrossProduct(dir, up, sideN);
	VectorNormalize(sideN);

	CrossProduct(sideN, dir, upN);
	VectorNormalize(upN);
#endif

	VectorNormalize2(dir, dirN);

	m[ 0] = sideN[0];	m[ 4] = sideN[1];		m[ 8] = sideN[2];		m[12] = -DotProduct(sideN, eye);
	m[ 1] = upN[0];		m[ 5] = upN[1];			m[ 9] = upN[2];			m[13] = -DotProduct(upN, eye);
	m[ 2] = dirN[0];	m[ 6] = dirN[1];		m[10] = dirN[2];		m[14] = -DotProduct(dirN, eye);
	m[ 3] = 0;			m[ 7] = 0;				m[11] = 0;				m[15] = 1;
}

void MatrixLookAtRH(matrix_t m, const vec3_t eye, const vec3_t dir, const vec3_t up)
{
	vec3_t dirN;
	vec3_t upN;
	vec3_t sideN;

	CrossProduct(dir, up, sideN);
	VectorNormalize(sideN);

	CrossProduct(sideN, dir, upN);
	VectorNormalize(upN);

	VectorNormalize2(dir, dirN);

	m[ 0] = sideN[0];	m[ 4] = sideN[1];		m[ 8] = sideN[2];		m[12] = -DotProduct(sideN, eye);
	m[ 1] = upN[0];		m[ 5] = upN[1];			m[ 9] = upN[2];			m[13] = -DotProduct(upN, eye);
	m[ 2] = -dirN[0];	m[ 6] = -dirN[1];		m[10] = -dirN[2];		m[14] = DotProduct(dirN, eye);
	m[ 3] = 0;			m[ 7] = 0;				m[11] = 0;				m[15] = 1;
}

void MatrixScaleTranslateToUnitCube(matrix_t m, const vec3_t mins, const vec3_t maxs)
{
	m[ 0] = 2/(maxs[0]-mins[0]);
	m[ 4] = 0;
	m[ 8] = 0;
	m[12] = -(maxs[0]+mins[0])/(maxs[0]-mins[0]);

	m[ 1] = 0;
	m[ 5] = 2/(maxs[1]-mins[1]);
	m[ 9] = 0;
	m[13] = -(maxs[1]+mins[1])/(maxs[1]-mins[1]);

	m[ 2] = 0;
	m[ 6] = 0;
	m[10] = 2/(maxs[2]-mins[2]);
	m[14] = -(maxs[2]+mins[2])/(maxs[2]-mins[2]);

	m[ 3] = 0;
	m[ 7] = 0;
	m[11] = 0;
	m[15] = 1;
}

void MatrixCrop(matrix_t m, const vec3_t mins, const vec3_t maxs)
{
	float			scaleX, scaleY, scaleZ;
	float			offsetX, offsetY, offsetZ;

	scaleX = 2.0f / (maxs[0] - mins[0]);
	scaleY = 2.0f / (maxs[1] - mins[1]);

	offsetX = -0.5f * (maxs[0] + mins[0]) * scaleX;
	offsetY = -0.5f * (maxs[1] + mins[1]) * scaleY;

	scaleZ = 1.0f / (maxs[2] - mins[2]);
	offsetZ = -mins[2] * scaleZ;

	m[ 0] = scaleX;		m[ 4] = 0;			m[ 8] = 0;      	m[12] = offsetX;
	m[ 1] = 0;			m[ 5] = scaleY;		m[ 9] = 0;      	m[13] = offsetY;
	m[ 2] = 0;			m[ 6] = 0;      	m[10] = scaleZ;		m[14] = offsetZ;
	m[ 3] = 0;			m[ 7] = 0;			m[11] = 0;			m[15] = 1;
}


// *INDENT-ON*

float ClampAxis( float ang )
{
	// returns Angle in the range (-360,360)
	ang = fmod( ang, 360.f );

	if( ang < 0.f )
	{
		// shift to [0,360) range
		ang += 360.f;
	}

	return ang;
}


float NormalizeAxis( float ang )
{
	// returns Angle in the range [0,360)
	ang = ClampAxis( ang );

	if( ang > 180.f )
	{
		// shift to (-180,180]
		ang -= 360.f;
	}

	return ang;
}

vec_t QuatNormalize(quat_t q)
{
	float           length, ilength;

	length = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
	length = sqrt(length);

	if(length)
	{
		ilength = 1 / length;
		q[0] *= ilength;
		q[1] *= ilength;
		q[2] *= ilength;
		q[3] *= ilength;
	}

	return length;
}

void QuatFromAngles(quat_t q, vec_t pitch, vec_t yaw, vec_t roll)
{
#if 1
	matrix_t        tmp;

	MatrixFromAngles(tmp, pitch, yaw, roll);
	QuatFromMatrix(q, tmp);
#else
	static float    sr, sp, sy, cr, cp, cy;

	// static to help MS compiler fp bugs
	sp = sin(DEG2RAD(pitch));
	cp = cos(DEG2RAD(pitch));

	sy = sin(DEG2RAD(yaw));
	cy = cos(DEG2RAD(yaw));

	sr = sin(DEG2RAD(roll));
	cr = cos(DEG2RAD(roll));

	q[0] = sr * cp * cy - cr * sp * sy;	// x
	q[1] = cr * sp * cy + sr * cp * sy;	// y
	q[2] = cr * cp * sy - sr * sp * cy;	// z
	q[3] = cr * cp * cy + sr * sp * sy;	// w
#endif
}

void QuatFromMatrix(quat_t q, const matrix_t m)
{
#if 1
	/*
	   From Quaternion to Matrix and Back
	   February 27th 2005
	   J.M.P. van Waveren

	   http://www.intel.com/cd/ids/developer/asmo-na/eng/293748.htm
	 */
	float           t, s;

	if(m[0] + m[5] + m[10] > 0.0f)
	{
		t = m[0] + m[5] + m[10] + 1.0f;
		s = (1.0f / sqrtf(t)) * 0.5f;

		q[3] = s * t;
		q[2] = (m[1] - m[4]) * s;
		q[1] = (m[8] - m[2]) * s;
		q[0] = (m[6] - m[9]) * s;
	}
	else if(m[0] > m[5] && m[0] > m[10])
	{
		t = m[0] - m[5] - m[10] + 1.0f;
		s = (1.0f / sqrtf(t)) * 0.5f;

		q[0] = s * t;
		q[1] = (m[1] + m[4]) * s;
		q[2] = (m[8] + m[2]) * s;
		q[3] = (m[6] - m[9]) * s;
	}
	else if(m[5] > m[10])
	{
		t = -m[0] + m[5] - m[10] + 1.0f;
		s = (1.0f / sqrtf(t)) * 0.5f;

		q[1] = s * t;
		q[0] = (m[1] + m[4]) * s;
		q[3] = (m[8] - m[2]) * s;
		q[2] = (m[6] + m[9]) * s;
	}
	else
	{
		t = -m[0] - m[5] + m[10] + 1.0f;
		s = (1.0f / sqrtf(t)) * 0.5f;

		q[2] = s * t;
		q[3] = (m[1] - m[4]) * s;
		q[0] = (m[8] + m[2]) * s;
		q[1] = (m[6] + m[9]) * s;
	}

#else
	float           trace;

	// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm

	trace = 1.0f + m[0] + m[5] + m[10];

	if(trace > 0.0f)
	{
		vec_t           s = 0.5f / sqrt(trace);

		q[0] = (m[6] - m[9]) * s;
		q[1] = (m[8] - m[2]) * s;
		q[2] = (m[1] - m[4]) * s;
		q[3] = 0.25f / s;
	}
	else
	{
		if(m[0] > m[5] && m[0] > m[10])
		{
			// column 0
			float           s = sqrt(1.0f + m[0] - m[5] - m[10]) * 2.0f;

			q[0] = 0.25f * s;
			q[1] = (m[4] + m[1]) / s;
			q[2] = (m[8] + m[2]) / s;
			q[3] = (m[9] - m[6]) / s;
		}
		else if(m[5] > m[10])
		{
			// column 1
			float           s = sqrt(1.0f + m[5] - m[0] - m[10]) * 2.0f;

			q[0] = (m[4] + m[1]) / s;
			q[1] = 0.25f * s;
			q[2] = (m[9] + m[6]) / s;
			q[3] = (m[8] - m[2]) / s;
		}
		else
		{
			// column 2
			float           s = sqrt(1.0f + m[10] - m[0] - m[5]) * 2.0f;

			q[0] = (m[8] + m[2]) / s;
			q[1] = (m[9] + m[6]) / s;
			q[2] = 0.25f * s;
			q[3] = (m[4] - m[1]) / s;
		}
	}

	QuatNormalize(q);
#endif
}

void QuatToVectorsFLU(const quat_t q, vec3_t forward, vec3_t left, vec3_t up)
{
	matrix_t        tmp;

	MatrixFromQuat(tmp, q);
	MatrixToVectorsFRU(tmp, forward, left, up);
}

void QuatToVectorsFRU(const quat_t q, vec3_t forward, vec3_t right, vec3_t up)
{
	matrix_t        tmp;

	MatrixFromQuat(tmp, q);
	MatrixToVectorsFRU(tmp, forward, right, up);
}

void QuatToAxis(const quat_t q, vec3_t axis[3])
{
	matrix_t        tmp;

	MatrixFromQuat(tmp, q);
	MatrixToVectorsFLU(tmp, axis[0], axis[1], axis[2]);
}

// wombat: pretty straightforward
void QuatToRotAngle( const quat_t q, vec_t *angle ) {
	*angle = 360.0f / M_PI * atan( VectorLength(q)/q[3] );
}

void QuatToRotAngleAxis( const quat_t q, vec_t *angle, vec3_t axis ) {

	*angle = atan( VectorLength(q)/q[3] );
	axis[0] = q[0] / sin(*angle);
	axis[1] = q[1] / sin(*angle);
	axis[2] = q[2] / sin(*angle);

	*angle *= 360.0f / M_PI;
}

void QuatFromRotAngleAxis( quat_t q, vec_t angle, const vec3_t axis ) {

	q[0] = axis[0] * sin( angle*M_PI/360 );
	q[1] = axis[1] * sin( angle*M_PI/360 );
	q[2] = axis[2] * sin( angle*M_PI/360 );
	q[3] = cos( angle*M_PI/360 );
}

void QuatToAngles( const quat_t q, vec3_t angles )
{
	quat_t          q2;

	q2[ 0 ] = q[ 0 ] * q[ 0 ];
	q2[ 1 ] = q[ 1 ] * q[ 1 ];
	q2[ 2 ] = q[ 2 ] * q[ 2 ];
	q2[ 3 ] = q[ 3 ] * q[ 3 ];

	angles[ PITCH ] = RAD2DEG( asin( -2 * ( q[ 2 ] * q[ 0 ] - q[ 3 ] * q[ 1 ] ) ) );
	angles[ YAW ] = RAD2DEG( atan2( 2 * ( q[ 2 ] * q[ 3 ] + q[ 0 ] * q[ 1 ] ), ( q2[ 2 ] - q2[ 3 ] - q2[ 0 ] + q2[ 1 ] ) ) );
	angles[ ROLL ] = RAD2DEG( atan2( 2 * ( q[ 3 ] * q[ 0 ] + q[ 2 ] * q[ 1 ] ), ( -q2[ 2 ] - q2[ 3 ] + q2[ 0 ] + q2[ 1 ] ) ) );
}

/*void QuatToAngles(const quat_t q, vec3_t angles)
{
	const float SingularityTest = q[Z]*q[X] - q[W]*q[Y];
	const float YawY = 2.f*( q[W]*q[Z] + q[X]*q[Y] );
	const float YawX = ( 1.f - 2.f*( Square( q[Y] ) + Square( q[Z] ) ) );
	const float SINGULARITY_THRESHOLD = 0.4999995f;
	const float RAD_TO_DEG = ( 180.f ) / M_PI;

	if( SingularityTest < -SINGULARITY_THRESHOLD )
	{
		angles[ PITCH ] = -90.0f;
		angles[ YAW ] = atan2( YawY, YawX ) * RAD_TO_DEG;
		angles[ ROLL ] = NormalizeAxis( -angles[ YAW ] - ( 2.f * atan2( q[ X ], q[ W ] ) * RAD_TO_DEG ) );
	}
	else if( SingularityTest > SINGULARITY_THRESHOLD )
	{
		angles[ PITCH ] = 90.0f;
		angles[ YAW ] = atan2( YawY, YawX ) * RAD_TO_DEG;
		angles[ ROLL ] = NormalizeAxis( angles[ YAW ] - ( 2.f * atan2( q[X], q[W] ) * RAD_TO_DEG ) );
	}
	else
	{
		angles[ PITCH ] = asin( 2.f*( SingularityTest ) ) * RAD_TO_DEG;
		angles[ YAW ] = atan2( YawY, YawX ) * RAD_TO_DEG;
		angles[ ROLL ] = atan2( -2.f*( q[W]*q[X] + q[Y]*q[Z] ), ( 1.f - 2.f*( Square( q[X] ) + Square( q[Y] ) ) ) ) * RAD_TO_DEG;
	}
}*/

void QuaternionMultiply( quat_t output, quat_t first, quat_t second )
{
	output[3] = (second[3] * first[3]) - (second[0] * first[0]) - (second[1] * first[1]) - (second[2] * first[2]);
	output[0] = (second[3] * first[0]) + (second[0] * first[3]) + (second[1] * first[2]) - (second[2] * first[1]);
	output[1] = (second[3] * first[1]) + (second[1] * first[3]) + (second[2] * first[0]) - (second[0] * first[2]);
	output[2] = (second[3] * first[2]) + (second[2] * first[3]) + (second[0] * first[1]) - (second[1] * first[0]);
}

void QuatMultiply0(quat_t qa, const quat_t qb)
{
	quat_t          tmp;

	QuatCopy(qa, tmp);
	QuatMultiply1(tmp, qb, qa);
}

void QuatMultiply1(const quat_t qa, const quat_t qb, quat_t qc)
{
	/*
	   from matrix and quaternion faq
	   x = w1x2 + x1w2 + y1z2 - z1y2
	   y = w1y2 + y1w2 + z1x2 - x1z2
	   z = w1z2 + z1w2 + x1y2 - y1x2

	   w = w1w2 - x1x2 - y1y2 - z1z2
	 */

	qc[0] = qa[3] * qb[0] + qa[0] * qb[3] + qa[1] * qb[2] - qa[2] * qb[1];
	qc[1] = qa[3] * qb[1] + qa[1] * qb[3] + qa[2] * qb[0] - qa[0] * qb[2];
	qc[2] = qa[3] * qb[2] + qa[2] * qb[3] + qa[0] * qb[1] - qa[1] * qb[0];
	qc[3] = qa[3] * qb[3] - qa[0] * qb[0] - qa[1] * qb[1] - qa[2] * qb[2];
}

void QuatMultiply2(const quat_t qa, const quat_t qb, quat_t qc)
{
	qc[0] = qa[3] * qb[0] + qa[0] * qb[3] + qa[1] * qb[2] + qa[2] * qb[1];
	qc[1] = qa[3] * qb[1] - qa[1] * qb[3] - qa[2] * qb[0] + qa[0] * qb[2];
	qc[2] = qa[3] * qb[2] - qa[2] * qb[3] - qa[0] * qb[1] + qa[1] * qb[0];
	qc[3] = qa[3] * qb[3] - qa[0] * qb[0] - qa[1] * qb[1] + qa[2] * qb[2];
}

void QuatMultiply3(const quat_t qa, const quat_t qb, quat_t qc)
{
	qc[0] = qa[3] * qb[0] + qa[0] * qb[3] + qa[1] * qb[2] + qa[2] * qb[1];
	qc[1] = -qa[3] * qb[1] + qa[1] * qb[3] - qa[2] * qb[0] + qa[0] * qb[2];
	qc[2] = -qa[3] * qb[2] + qa[2] * qb[3] - qa[0] * qb[1] + qa[1] * qb[0];
	qc[3] = -qa[3] * qb[3] + qa[0] * qb[0] - qa[1] * qb[1] + qa[2] * qb[2];
}

void QuatMultiply4(const quat_t qa, const quat_t qb, quat_t qc)
{
	qc[0] = qa[3] * qb[0] - qa[0] * qb[3] - qa[1] * qb[2] - qa[2] * qb[1];
	qc[1] = -qa[3] * qb[1] - qa[1] * qb[3] + qa[2] * qb[0] - qa[0] * qb[2];
	qc[2] = -qa[3] * qb[2] - qa[2] * qb[3] + qa[0] * qb[1] - qa[1] * qb[0];
	qc[3] = -qa[3] * qb[3] - qa[0] * qb[0] + qa[1] * qb[1] - qa[2] * qb[2];
}

void QuatSlerp(const quat_t from, const quat_t to, float frac, quat_t out)
{
#if 0
	quat_t          to1;
	double          omega, cosom, sinom, scale0, scale1;

	cosom = from[0] * to[0] + from[1] * to[1] + from[2] * to[2] + from[3] * to[3];

	if(cosom < 0.0)
	{
		cosom = -cosom;

		QuatCopy(to, to1);
		QuatAntipodal(to1);
	}
	else
	{
		QuatCopy(to, to1);
	}

	if((1.0 - cosom) > 0)
	{
		omega = acos(cosom);
		sinom = sin(omega);
		scale0 = sin((1.0 - frac) * omega) / sinom;
		scale1 = sin(frac * omega) / sinom;
	}
	else
	{
		scale0 = 1.0 - frac;
		scale1 = frac;
	}

	out[0] = scale0 * from[0] + scale1 * to1[0];
	out[1] = scale0 * from[1] + scale1 * to1[1];
	out[2] = scale0 * from[2] + scale1 * to1[2];
	out[3] = scale0 * from[3] + scale1 * to1[3];
#else
	/*
	   Slerping Clock Cycles
	   February 27th 2005
	   J.M.P. van Waveren

	   http://www.intel.com/cd/ids/developer/asmo-na/eng/293747.htm
	 */
	float           cosom, absCosom, sinom, sinSqr, omega, scale0, scale1;

	if(frac <= 0.0f)
	{
		QuatCopy(from, out);
		return;
	}

	if(frac >= 1.0f)
	{
		QuatCopy(to, out);
		return;
	}

	if(QuatCompare(from, to))
	{
		QuatCopy(from, out);
		return;
	}

	cosom = from[0] * to[0] + from[1] * to[1] + from[2] * to[2] + from[3] * to[3];
	absCosom = fabs(cosom);

	if((1.0f - absCosom) > 1e-6f)
	{
		sinSqr = 1.0f - absCosom * absCosom;
		sinom = 1.0f / sqrt(sinSqr);
		omega = atan2(sinSqr * sinom, absCosom);

		scale0 = sin((1.0f - frac) * omega) * sinom;
		scale1 = sin(frac * omega) * sinom;
	}
	else
	{
		scale0 = 1.0f - frac;
		scale1 = frac;
	}

	scale1 = (cosom >= 0.0f) ? scale1 : -scale1;

	out[0] = scale0 * from[0] + scale1 * to[0];
	out[1] = scale0 * from[1] + scale1 * to[1];
	out[2] = scale0 * from[2] + scale1 * to[2];
	out[3] = scale0 * from[3] + scale1 * to[3];
#endif
}

void QuatTransformVector(const quat_t q, const vec3_t in, vec3_t out)
{
	matrix_t        m;

	MatrixFromQuat(m, q);
	MatrixTransformNormal(m, in, out);
}

void MatrixToEulerAngles
	(
	const float mat[ 3 ][ 3 ],
	vec3_t ang
	)

{
	double theta;
	double cp;
	double sp;

	sp = mat[ 0 ][ 2 ];

	// cap off our sin value so that we don't get any NANs
	if( sp > 1.0 )
	{
		sp = 1.0;
	}
	if( sp < -1.0 )
	{
		sp = -1.0;
	}

	theta = -asin( sp );
	cp = cos( theta );

	if( cp > 8192 * FLT_EPSILON )
	{
		ang[ 0 ] = theta * 180 / M_PI;
		ang[ 1 ] = atan2( mat[ 0 ][ 1 ], mat[ 0 ][ 0 ] ) * 180 / M_PI;
		ang[ 2 ] = atan2( mat[ 1 ][ 2 ], mat[ 2 ][ 2 ] ) * 180 / M_PI;
	}
	else
	{
		ang[ 0 ] = theta * 180 / M_PI;
		ang[ 1 ] = -atan2( mat[ 1 ][ 0 ], mat[ 1 ][ 1 ] ) * 180 / M_PI;
		ang[ 2 ] = 0;
	}
}

void TransposeMatrix( float in[ 3 ][ 3 ], float out[ 3 ][ 3 ] )
{
	out[ 0 ][ 0 ] = in[ 0 ][ 0 ];
	out[ 0 ][ 1 ] = in[ 1 ][ 0 ];
	out[ 0 ][ 2 ] = in[ 2 ][ 0 ];
	out[ 1 ][ 0 ] = in[ 0 ][ 1 ];
	out[ 1 ][ 1 ] = in[ 1 ][ 1 ];
	out[ 1 ][ 2 ] = in[ 2 ][ 1 ];
	out[ 2 ][ 0 ] = in[ 0 ][ 2 ];
	out[ 2 ][ 1 ] = in[ 1 ][ 2 ];
	out[ 2 ][ 2 ] = in[ 2 ][ 2 ];
}
