/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// vector.h: C++ vector class.

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <math.h>
#include <stdio.h>

//#define X 0
//#define Y 1
//#define Z 2
//#define W 3

#ifdef __Q_FABS__
#define VECTOR_FABS  Q_fabs
#else
#define VECTOR_FABS  fabs
#endif

static float vrsqrt( float number )
{
	union {
		float f;
		int i;
	} t;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	t.f = number;
	t.i = 0x5f3759df - ( t.i >> 1 ); // what the fuck?
	y = t.f;
	y = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration
	return y;
}

class Vector
{
public:
	float		x;
	float		y;
	float		z;

	Vector();
	Vector( const vec3_t src );
	Vector( const float x, const float y, const float z );
	explicit	Vector( const char *text );

	operator float * ();
	operator float const * () const;

	float					pitch( void ) const;
	float					yaw( void ) const;
	float					roll( void ) const;
	float					operator[]( const int index ) const;
	float &					operator[]( const int index );
	void 					copyTo( vec3_t vec ) const;
	void					setPitch( const float x );
	void 					setYaw( const float y );
	void 					setRoll( const float z );
	void 					setXYZ( const float x, const float y, const float z );
	const Vector &			operator=( const Vector &a );
	const Vector &			operator=( vec3_t a );
	const Vector &			operator=( const char *a );
	friend Vector			operator+( const Vector &a, const Vector &b );
	friend Vector			operator+( vec3_t a, const Vector &b );
	friend Vector			operator+( const Vector &a, vec3_t b );
	const Vector &			operator+=( const Vector &a );
	const Vector &			operator+=( vec3_t a );
	friend Vector			operator-( const Vector &a, const Vector &b );
	friend Vector			operator-( vec3_t a, const Vector &b );
	friend Vector			operator-( const Vector &a, vec3_t b );
	const Vector &			operator-=( const Vector &a );
	const Vector &			operator-=( vec3_t a );
	friend Vector			operator*( const Vector &a, const float b );
	friend Vector			operator*( const float a, const Vector &b );
	friend float			operator*( const Vector &a, const Vector &b );
	friend float			operator*( vec3_t a, const Vector &b );
	friend float			operator*( const Vector &a, vec3_t b );
	const Vector &			operator*=( const float a );
	friend Vector			operator/( const Vector &a, const float b );
	friend Vector			operator/( const float a, const Vector &b );
	friend float			operator/( const Vector &a, const Vector &b );
	friend float			operator/( vec3_t a, const Vector &b );
	friend float			operator/( const Vector &a, vec3_t b );
	const Vector &			operator/=( const float a );
	friend int				operator==(	const Vector &a, const Vector &b );
	friend int				operator==(	vec3_t a, const Vector &b );
	friend int				operator==(	const Vector &a, vec3_t b );
	friend int				operator!=(	const Vector &a, const Vector &b );
	friend int				operator!=(	vec3_t a, const Vector &b );
	friend int				operator!=(	const Vector &a, vec3_t b );
	int						FuzzyEqual( const Vector &b, const float epsilon ) const;
	int						FuzzyEqual( vec3_t b, const float epsilon ) const;
	const Vector &			CrossProduct( const Vector &a, const Vector &b	);
	const Vector &			CrossProduct( vec3_t a, const Vector &b	);
	const Vector &			CrossProduct( const Vector &a, vec3_t b	);
	float					length( void ) const;
	float					lengthfast( void ) const;
	float					lengthSquared( void ) const;
	float					lengthXY( void ) const;
	float					normalize( void );
	void					normalizefast( void );
	void					EulerNormalize( void );
	void					EulerNormalize360( void );
	static Vector			Clamp( Vector &value, const Vector &min, const Vector &max );
	static Vector			Cross( const Vector &vector1, const Vector &vector2 );
	static float			Dot( const Vector &vector1, const Vector &vector2 );
	static float			Dot( vec3_t a, const Vector &b );
	static float			Dot( const Vector &a, vec3_t b );
	static float			Distance( const Vector &vector1, const Vector &vector2 );
	static float			DistanceSquared( const Vector &vector1, const Vector &vector2 );
	static float			DistanceXY( const Vector &vector1, const Vector &vector2 );
	static Vector			AnglesBetween( const Vector &vector1, const Vector &vector2 );
	static float			AngleBetween( const Vector &vector1, const Vector &vector2 );
	static bool				CloseEnough( const Vector &vector1, const Vector &vector2, const float epsilon = Vector::Epsilon()) ;
	static bool				SmallEnough( const Vector &vector, const float epsilon = Vector::Epsilon() );
	static float			Epsilon( void );
	static Vector &			Identity( void );
	Vector					operator-( void ) const;
	friend Vector			fabs( const Vector &a );
	float					toYaw( void	) const;
	float					toPitch( void ) const;
	Vector					toAngles( void ) const;
	Vector					AnglesMod( void ) const;
	void					AngleVectors( Vector *forward, Vector *right = NULL, Vector *up = NULL )  const;
	void					AngleVectorsLeft( Vector *forward, Vector *right = NULL, Vector *up = NULL )  const;
	friend Vector			LerpVector( const Vector &w1, const Vector &w2, const float t );
	friend float			MaxValue( const Vector &a );
	Vector					GetRotatedX( float angle ) const;
	void					RotateX( double angle );
	Vector					GetRotatedY( float angle ) const;
	void					RotateY( double angle );
	Vector					GetRotatedZ( float angle ) const;
	void					RotateZ( float angle );
	void					PackTo01();
	Vector					GetPackedTo01() const;
};

static Vector vec_origin = Vector( 0, 0, 0 );
static Vector vec_zero = Vector( 0, 0, 0 );
static Vector g_vEyeDir = Vector( 0, 0, 0 );

inline float Vector::pitch( void ) const		{ return x; }
inline float Vector::yaw( void ) const			{ return y; }
inline float Vector::roll( void ) const			{ return z; }
inline void  Vector::setPitch( float pitch )	{ x = pitch; }
inline void  Vector::setYaw( float yaw )		{ y = yaw; }
inline void  Vector::setRoll( float roll )		{ z = roll; }

inline void Vector::copyTo(	vec3_t vec ) const
{
	vec[ 0 ] = x;
	vec[ 1 ] = y;
	vec[ 2 ] = z;
}

inline float Vector::operator[]( const int index ) const
{
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &x )[ index ];
}

inline float& Vector::operator[]( const int index )
{
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &x )[ index ];
}

inline void  Vector::setXYZ( const float new_x, const float new_y,const float new_z	)
{
	x = new_x;
	y = new_y;
	z = new_z;
}

inline Vector::Vector(): x( 0 ), y( 0 ), z( 0 )
{
}

inline Vector::Vector( const vec3_t src ): x( src[0] ),	y( src[1] ), z( src[2] )
{
}

inline Vector::Vector( const float init_x, const float init_y, const float init_z ): x( init_x ), y( init_y ), z( init_z )
{
}

inline Vector::Vector( const char *text	): x( 0 ), y( 0 ), z( 0 )

{
	if ( text )
	{
		if ( text[0] == '"' )
			sscanf( text, "\"%f %f %f\"", &x, &y, &z );
		else
			sscanf( text, "%f %f %f", &x, &y, &z );
	}
}

inline Vector::operator float * ( void )
{
	return &x;
}

inline Vector::operator float const * ( void ) const
{
	return &x;
}

inline const Vector & Vector::operator=( const Vector &a )
{
	x = a.x;
	y = a.y;
	z = a.z;

	return *this;
}

inline const Vector & Vector::operator=( vec3_t a )
{
	x = a[ 0 ];
	y = a[ 1 ];
	z = a[ 2 ];

	return *this;
}

inline const Vector & Vector::operator=( const char *a )
{
	if( a )
	{
		if( a[ 0 ] == '"' )
			sscanf( a, "\"%f %f %f\"", &x, &y, &z );
		else
			sscanf( a, "%f %f %f", &x, &y, &z );
	}

	return *this;
}

inline Vector operator+( const Vector &a, const Vector &b )
{
	return Vector( a.x + b.x, a.y + b.y, a.z + b.z );
}

inline Vector operator+( vec3_t a, const Vector &b )
{
	return Vector( a[ 0 ] + b.x, a[ 1 ] + b.y, a[ 2 ] + b.z );
}

inline Vector operator+( const Vector &a, vec3_t b )
{
	return Vector( a.x + b[ 0 ], a.y + b[ 1 ], a.z + b[ 2 ] );
}

inline const Vector & Vector::operator+=( const Vector &a )
{
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}

inline const Vector & Vector::operator+=( vec3_t a	)
{
	x += a[ 0 ];
	y += a[ 1 ];
	z += a[ 2 ];

	return *this;
}

inline Vector operator-( const Vector &a, const Vector &b )
{
	return Vector( a.x - b.x, a.y - b.y, a.z - b.z );
}

inline Vector operator-( vec3_t a, const Vector &b )
{
	return Vector( a[ 0 ] - b.x, a[ 1 ] - b.y, a[ 2 ] - b.z );
}

inline Vector operator-( const Vector &a, vec3_t b )
{
	return Vector( a.x - b[ 0 ], a.y - b[ 1 ], a.z - b[ 2 ] );
}

inline const Vector & Vector::operator-=( const Vector &a )
{
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

inline const Vector & Vector::operator-=( vec3_t a )
{
	x -= a[ 0 ];
	y -= a[ 1 ];
	z -= a[ 2 ];

	return *this;
}

inline Vector operator*( const Vector &a, const float b )
{
	return Vector( a.x * b, a.y * b, a.z * b );
}

inline Vector operator*( const float a,	const Vector &b	)
{
	return b * a;
}

inline float operator*(	const Vector &a, const Vector &b )
{
	return ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );
}

inline float operator*( vec3_t a, const Vector &b )
{
	return ( a[ 0 ] * b.x ) + ( a[ 1 ] * b.y ) + ( a[ 2 ] * b.z );
}

inline float operator*(	const Vector &a, vec3_t b )
{
	return ( a.x * b[ 0 ] ) + ( a.y * b[ 1 ] ) + ( a.z * b[ 2 ] );
}

inline const Vector& Vector::operator*=( const float a )
{
	x *= a;
	y *= a;
	z *= a;

	return *this;
}

inline Vector operator/( const Vector &a, const float b )
{
	return Vector (a.x/b, a.y/b, a.z/b);
}

inline Vector operator/( const float a,	const Vector &b	)
{
	return b / a;
}

inline float operator/(	const Vector &a, const Vector &b )
{
	return ( a.x / b.x ) + ( a.y / b.y ) + ( a.z / b.z );
}

inline float operator/( vec3_t a, const Vector &b )
{
	return ( a[ 0 ] / b.x ) + ( a[ 1 ] / b.y ) + ( a[ 2 ] / b.z );
}

inline float operator/(	const Vector &a, vec3_t b )
{
	return ( a.x / b[ 0 ] ) + ( a.y / b[ 1 ] ) + ( a.z / b[ 2 ] );
}

inline const Vector &	Vector::operator/=( const float a )
{
	*this=*this/a;
	return *this;
}

inline int Vector::FuzzyEqual( const Vector &b, const float epsilon ) const
{
	return
		(
		( VECTOR_FABS( x - b.x ) < epsilon ) &&
		( VECTOR_FABS( y - b.y ) < epsilon ) &&
		( VECTOR_FABS( z - b.z ) < epsilon )
		);
}

inline int Vector::FuzzyEqual( vec3_t b, const float epsilon ) const
{
	return
		(
		( VECTOR_FABS( x - b[ 0 ] ) < epsilon ) &&
		( VECTOR_FABS( y - b[ 1 ] ) < epsilon ) &&
		( VECTOR_FABS( z - b[ 2 ] ) < epsilon )
		);
}

inline int operator==( const Vector &a, const Vector &b	)

{
	return ( ( a.x == b.x ) && ( a.y == b.y ) && ( a.z == b.z ) );
}

inline int operator==( vec3_t a, const Vector &b )
{
	return ( ( a[ 0 ] == b.x ) && ( a[ 1 ] == b.y ) && ( a[ 2 ] == b.z ) );
}

inline int operator==( const Vector &a, vec3_t b )
{
	return ( ( a.x == b[ 0 ] ) && ( a.y == b[ 1 ] ) && ( a.z == b[ 2 ] ) );
}

inline int operator!=( const Vector &a,	const Vector &b	)
{
	return ( ( a.x != b.x ) || ( a.y != b.y ) || ( a.z != b.z ) );
}

inline int operator!=( vec3_t a, const Vector &b )
{
	return ( ( a[ 0 ] != b.x ) || ( a[ 1 ] != b.y ) || ( a[ 2 ] != b.z ) );
}

inline int operator!=( const Vector &a, vec3_t b )
{
	return ( ( a.x != b[ 0 ] ) || ( a.y != b[ 1 ] ) || ( a.z != b[ 2 ] ) );
}

inline const Vector & Vector::CrossProduct( const Vector &a, const Vector &b )
{
	x = ( a.y * b.z ) - ( a.z * b.y );
	y = ( a.z * b.x ) - ( a.x * b.z );
	z = ( a.x * b.y ) - ( a.y * b.x );

	return *this;
}

inline const Vector & Vector::CrossProduct( vec3_t a, const Vector &b )
{
	x = ( a[ 1 ] * b.z ) - ( a[ 2 ] * b.y );
	y = ( a[ 2 ] * b.x ) - ( a[ 0 ] * b.z );
	z = ( a[ 0 ] * b.y ) - ( a[ 1 ] * b.x );

	return *this;
}

inline const Vector & Vector::CrossProduct( const Vector &a, vec3_t b )
{
	x = ( a.y * b[ 2 ] ) - ( a.z * b[ 1 ] );
	y = ( a.z * b[ 0 ] ) - ( a.x * b[ 2 ] );
	z = ( a.x * b[ 1 ] ) - ( a.y * b[ 0 ] );

	return *this;
}

inline Vector Vector::Clamp( Vector &value, const Vector &minimum, const Vector &maximum )
{
	Vector clamped(value);
	for (int i=0; i<3; i++)
	{
		const float min = minimum[i];
		const float max = maximum[i];
		assert( min <= max );

		if (clamped[i] < min)
		{
			clamped[i] = min;
		}
		else if (clamped[i] > max)
		{
			clamped[i] = max;
		}
	}
	return clamped;
}
inline Vector Vector::Cross( const Vector &vector1, const Vector &vector2 )
{
	const Vector result (
	( vector1.y * vector2.z ) - ( vector1.z * vector2.y ),
	( vector1.z * vector2.x ) - ( vector1.x * vector2.z ),
	( vector1.x * vector2.y ) - ( vector1.y * vector2.x )
	);

	return result;
}


inline float Vector::Dot( const Vector &vector1, const Vector &vector2 )
{
	return vector1 * vector2;
}

inline float Vector::Dot( vec3_t vector1, const Vector &vector2 )
{
	return vector1 * vector2;
}

inline float Vector::Dot( const Vector &vector1, vec3_t vector2 )
{
	return vector1 * vector2;
}

//----------------------------------------------------------------
// Name:				lengthSquared
// Class:			Vector
//
// Description:	Returns squared length of the vector
//
// Parameters:		None
//
// Returns:			float - squared length
//----------------------------------------------------------------
inline float Vector::lengthSquared( void ) const
{
	return ( x * x ) + ( y * y ) + ( z * z );
}

inline float Vector::length( void ) const
{
	return sqrt( lengthSquared() );
}

inline float Vector::lengthfast( void ) const
{
	return vrsqrt( lengthSquared() );
}

//----------------------------------------------------------------
// Name:				lengthXY
// Class:			Vector
//
// Description:	Returns length of the vector (using only the x
//						and y components
//
// Parameters:		None
//
// Returns:			float - length of the vector in the xy plane
//----------------------------------------------------------------
inline float Vector::lengthXY( void ) const
{
	return sqrt(( x * x ) + ( y * y ));
}

//----------------------------------------------------------------
// Name:				normalize
// Class:			Vector
//
// Description:	unitizes the vector
//
// Parameters:		None
//
// Returns:			float - length of the vector before the function
//----------------------------------------------------------------
inline float Vector::normalize( void )
{
	float	length, ilength;

	length = this->length();
	if ( length )
	{
		ilength = 1.0f / length;
		x *= ilength;
		y *= ilength;
		z *= ilength;
	}

	return length;
}

//----------------------------------------------------------------
// Name:				normalizefast
// Class:			Vector
//
// Description:	fast version of normalize
//
// Parameters:		None
//
// Returns:			float - length of the vector before the function
//----------------------------------------------------------------
inline void Vector::normalizefast( void )
{
	float ilength;

	ilength = this->lengthfast();

	x *= ilength;
	y *= ilength;
	z *= ilength;
}

//----------------------------------------------------------------
// Name:				EulerNormalize
// Class:			Vector
//
// Description:	forces each component of the vector into the
//						range (-180, +180) by adding or subtracting 360
//						This is useful when the Vector is being used as
//						EulerAngles to represent a rotational offset
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
inline void Vector::EulerNormalize( void )
{
	x = AngleNormalize180( x );
	y = AngleNormalize180( y );
	z = AngleNormalize180( z );
}

//----------------------------------------------------------------
// Name:				EulerNormalize360
// Class:			Vector
//
// Description:	forces each component of the vector into the
//						range (0, +360) by adding or subtracting 360
//						This is useful when the Vector is being used as
//						EulerAngles to represent a rotational direction
//
// Parameters:		None
//
// Returns:			None
//----------------------------------------------------------------
inline void Vector::EulerNormalize360( void )
{
	x = AngleNormalize360( x );
	y = AngleNormalize360( y );
	z = AngleNormalize360( z );
}

//----------------------------------------------------------------
// Name:				Epsilon
// Class:			Vector
//
// Description:	returns a standard 'small' value for the class
//
// Parameters:		None
//
// Returns:			float - the epsilon constant for the class
//----------------------------------------------------------------
inline float Vector::Epsilon( void )
{
	return 0.000000001f;
}

//----------------------------------------------------------------
// Name:				Identity
// Class:			Vector
//
// Description:	returns the additive identity for the class
//
// Parameters:		None
//
// Returns:			Vector - the identity for the class
//----------------------------------------------------------------
inline Vector & Vector::Identity(void)
{
	return vec_zero;
}

//----------------------------------------------------------------
// Name:				Distance
// Class:			Vector
//
// Description:	returns the distance between two vectors
//
// Parameters:
//						Vector - first vector
//						Vector - second vector
//
// Returns:			float - distance between the two vectors
//----------------------------------------------------------------
inline float Vector::Distance(const Vector &vector1, const Vector &vector2)
{
	return (vector1 - vector2).length();
}

//----------------------------------------------------------------
// Name:				DistanceSquared
// Class:			Vector
//
// Description:	returns the squared distance between two vectors
//
// Parameters:
//						Vector - first vector
//						Vector - second vector
//
// Returns:			float - distance between the two vectors squared
//----------------------------------------------------------------
inline float Vector::DistanceSquared(const Vector &vector1, const Vector &vector2)
{
	return (vector1 - vector2).lengthSquared();
}

//----------------------------------------------------------------
// Name:				DistanceXY
// Class:			Vector
//
// Description:	returns the distance between two vectors in the
//						xy plane
//
// Parameters:
//						Vector - first vector
//						Vector - second vector
//
// Returns:			float - distance between the two vectors in the
//						xy plane
//----------------------------------------------------------------
inline float Vector::DistanceXY(const Vector &vector1, const Vector &vector2)
{
	return (vector1 - vector2).lengthXY();
}

inline Vector Vector::toAngles( void ) const
{
	float	forward;
	float	yaw, pitch;

	if ( ( x == 0.0f ) && ( y == 0.0f ) )
	{
		yaw = 0.0f;
		if ( z > 0.0f )
		{
			pitch = 90.0f;
		}
		else
		{
			pitch = 270.0f;
		}
	}
	else
	{
		yaw = atan2( y, x ) * 180.0f / M_PI;
		if ( yaw < 0.0f )
		{
			yaw += 360.0f;
		}

		forward = ( float )sqrt( x * x + y * y );
		pitch = atan2( z, forward ) * 180.0f / M_PI;
		if ( pitch < 0.0f )
		{
			pitch += 360.0f;
		}
	}

	return Vector( -pitch, yaw, 0.0f );
}

//----------------------------------------------------------------
// Name:				AnglesBetween
// Class:			Vector
//
// Description:	returns the smaller of the angles formed by the
//						two vectors
//
// Parameters:
//						Vector - first vector
//						Vector - second vector
//
// Returns:			Vector - angles between the vectors
//----------------------------------------------------------------
inline Vector Vector::AnglesBetween(const Vector &vector1, const Vector &vector2)
{
	Vector unitVector1(vector1);
	unitVector1.normalize();
	Vector unitVector2(vector2);
	unitVector2.normalize();
	Vector angles(unitVector1.toAngles() - unitVector2.toAngles());
	angles.EulerNormalize();

	return angles;
}

//----------------------------------------------------------------
// Name:				AngleBetween
// Class:			Vector
//
// Description:	returns the smaller of the angles formed by the
//						two vectors
//
// Parameters:
//						Vector - first vector
//						Vector - second vector
//
// Returns:			float - angle between the vectors
//----------------------------------------------------------------
inline float Vector::AngleBetween(const Vector &vector1, const Vector &vector2)
{
	Vector unitVector1(vector1);
	unitVector1.normalize();
	Vector unitVector2(vector2);
	unitVector2.normalize();

	return acos( Vector::Dot( unitVector1, unitVector2 ) );
}

//----------------------------------------------------------------
// Name:				CloseEnough
// Class:			Vector
//
// Description:	tests to see if the two vectors are within
//						'epsilon' of each other
//
// Parameters:
//						Vector - first vector
//						Vector - second vector
//						float  - amount that each component of the
//						vectors can be apart
//
// Returns:			bool	 - the result of the test for closeness
//----------------------------------------------------------------
inline bool Vector::CloseEnough(const Vector &vector1, const Vector &vector2, const float epsilon)
{
	return Distance(vector1, vector2) < epsilon;
}

//----------------------------------------------------------------
// Name:				SmallEnough
// Class:			Vector
//
// Description:	tests to see if the vectors are within
//						'epsilon' of the origin
//
// Parameters:
//						Vector - vector
//						float  - amount that each component of the
//						vectors can be from the origin
//
// Returns:			bool	 - the result of the test for smallness
//----------------------------------------------------------------
inline bool Vector::SmallEnough(const Vector &vector, const float epsilon)
{
	return CloseEnough(vector, Vector::Identity(), epsilon);
}

inline Vector Vector::operator-() const
{
	return Vector( -x, -y, -z );
}

inline Vector fabs( const Vector &a )
{
	return Vector( VECTOR_FABS( a.x ), VECTOR_FABS( a.y ), VECTOR_FABS( a.z ) );
}

inline float MaxValue( const Vector &a )
{
	float maxy;
	float maxz;
	float max;

	max = VECTOR_FABS( a.x );
	maxy = VECTOR_FABS( a.y );
	maxz = VECTOR_FABS( a.z );

	if ( maxy > max )
	{
		max = maxy;
	}
	if ( maxz > max )
	{
		max = maxz;
	}
	return max;
}

inline float Vector::toYaw( void ) const
{
	float yaw;

	if ( ( y == 0.0f ) && ( x == 0.0f ) )
	{
		yaw = 0.0f;
	}
	else
	{
		yaw = ( float )( ( int )( atan2( y, x ) * 180.0f / M_PI ) );
		if ( yaw < 0.0f )
		{
			yaw += 360.0f;
		}
	}

	return yaw;
}

inline float Vector::toPitch( void ) const
{
	float	forward;
	float	pitch;

	if ( ( x == 0.0f ) && ( y == 0.0f ) )
	{
		if ( z > 0.0f )
		{
			pitch = 90.0f;
		}
		else
		{
			pitch = 270.0f;
		}
	}
	else
	{
		forward = ( float )sqrt( ( x * x ) + ( y * y ) );
		pitch = ( float )( ( int )( atan2( z, forward ) * 180.0f / M_PI ) );
		if ( pitch < 0.0f )
		{
			pitch += 360.0f;
		}
	}

	return pitch;
}

inline Vector Vector::AnglesMod( void ) const
{
	return Vector( AngleMod( x ), AngleMod( y ), AngleMod( z ) );
}

inline void Vector::AngleVectors( Vector *forward, Vector *right, Vector *up ) const
{
	float				angle;
	static float	sr, sp, sy, cr, cp, cy; // static to help MS compiler fp bugs

	angle = yaw() * ( M_PI * 2.0f / 360.0f );
	sy = sin( angle );
	cy = cos( angle );

	angle = pitch() * ( M_PI * 2.0f / 360.0f );
	sp = sin( angle );
	cp = cos( angle );

	angle = roll() * ( M_PI * 2.0f / 360.0f );
	sr = sin( angle );
	cr = cos( angle );

	if( forward )
	{
		forward->setXYZ( cp * cy, cp * sy, -sp );
	}

	if( right )
	{
		right->setXYZ( ( -1 * sr * sp * cy ) + ( -1 * cr * -sy ), ( -1 * sr * sp * sy ) + ( -1 * cr * cy ), -1 * sr * cp );
	}

	if( up )
	{
		up->setXYZ( ( cr * sp * cy ) + ( -sr * -sy ), ( cr * sp * sy ) + ( -sr * cy ), cr * cp );
	}
}

inline void Vector::AngleVectorsLeft( Vector *forward, Vector *left, Vector *up ) const
{
	float				angle;
	static float	sr, sp, sy, cr, cp, cy; // static to help MS compiler fp bugs

	angle = yaw() * ( M_PI * 2.0f / 360.0f );
	sy = sin( angle );
	cy = cos( angle );

	angle = pitch() * ( M_PI * 2.0f / 360.0f );
	sp = sin( angle );
	cp = cos( angle );

	angle = roll() * ( M_PI * 2.0f / 360.0f );
	sr = sin( angle );
	cr = cos( angle );

	if ( forward )
	{
		forward->setXYZ( cp * cy, cp * sy, -sp );
	}

	if ( left )
	{
		left->setXYZ( ( sr * sp * cy ) + ( cr * -sy ), (sr * sp * sy ) + ( cr * cy ), sr * cp );
	}

	if ( up )
	{
		up->setXYZ( ( cr * sp * cy ) + ( -sr * -sy ), ( cr * sp * sy ) + ( -sr * cy ), cr * cp );
	}
}


#define LERP_DELTA 1e-6
inline Vector LerpVector( const Vector &vector1, const Vector &vector2, const float t )
{
	float	omega, cosom, sinom, scale0, scale1;

	Vector w1( vector1 );
	Vector w2( vector2 );

	w1.normalize();
	w2.normalize();

	cosom = w1 * w2;
	if ( ( 1.0f - cosom ) > LERP_DELTA )
	{
		omega = acos( cosom );
		sinom = sin( omega );
		scale0 = sin( ( 1.0f - t ) * omega ) / sinom;
		scale1 = sin( t * omega ) / sinom;
	}
	else
	{
		scale0 = 1.0f - t;
		scale1 = t;
	}

	return ( ( w1 * scale0 ) + ( w2 * scale1 ) );
}

class Quat
	{
	public:
		float			x;
		float			y;
		float			z;
		float			w;

						Quat();
						Quat( Vector angles );
						Quat( float scrMatrix[ 3 ][ 3 ] );
						Quat( const float x, const float y, const float z, const float w );

		float *			vec4( void );
		float			operator[]( const int index ) const;
		float &			operator[]( const int index );
		void 			set( const float x, const float y, const float z, const float w );
		const Quat &	operator=( const Quat &a );
		friend Quat		operator+( const Quat &a, const Quat &b );
		const Quat &	operator+=( const Quat &a );
		friend Quat		operator-( const Quat &a, const Quat &b );
		const Quat &	operator-=( const Quat &a );
		friend Quat		operator*( const Quat &a, const float b );
		friend Quat		operator*( const float a, const Quat &b );
		const Quat &	operator*=( const float a );
		friend int		operator==(	const Quat &a, const Quat &b );
		friend int		operator!=(	const Quat &a, const Quat &b );
		float			length( void ) const;
		float			lengthSquared( void ) const;
		const Quat &	normalize( void );
		Quat			operator-() const;
		Vector			toAngles( void );
	};

inline Quat::Quat(): x( 0 ), y( 0 ), z( 0 ), w( 0 )
{
}

inline Quat::Quat( Vector Angles )
{
	EulerToQuat( Angles, this->vec4() );
}

inline Quat::Quat( float srcMatrix[ 3 ][ 3 ] )
{
	MatToQuat( srcMatrix, this->vec4() );
}

inline Quat::Quat( const float init_x, const float init_y, const float init_z, const float init_w ): x( init_x ), y( init_y ), z( init_z ), w( init_w )
{
}

inline float Quat::operator[]( const int index ) const
{
	assert( ( index >= 0 ) && ( index < 4 ) );
	return ( &x )[ index ];
}

inline float & Quat::operator[]( const int index)
{
	assert( ( index >= 0 ) && ( index < 4 ) );
	return ( &x )[ index ];
}

inline float *Quat::vec4( void )
{
	return &x;
}

inline void  Quat::set(	const float new_x, const float new_y, const float new_z, const float new_w )
{
	x = new_x;
	y = new_y;
	z = new_z;
	w = new_w;
}


inline const Quat & Quat::operator=( const Quat &a )
{
	x = a.x;
	y = a.y;
	z = a.z;
	w = a.w;

	return *this;
}

inline Quat operator+( const Quat &a, const Quat &b )
{
	return Quat( a.x + b.x, a.y + b.y , a.z + b.z, a.w + b.w );
}

inline const Quat & Quat::operator+=( const Quat &a )
{
	*this = *this + a;

	return *this;
}

inline Quat operator-( const Quat &a, const Quat &b )
{
	return Quat( a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w );
}

inline const Quat & Quat::operator-=( const Quat &a )
{
	*this = *this - a;

	return *this;
}

inline Quat operator*( const Quat &a, const float b )
{
	return Quat( a.x * b, a.y * b, a.z * b, a.w * b );
}

inline Quat operator*( const float a, const Quat &b )
{
	return b * a;
}

inline const Quat & Quat::operator*=( const float a )
{
	*this = *this * a;

	return *this;
}

inline int operator==( const Quat &a, const Quat &b	)
{
	return ( ( a.x == b.x ) && ( a.y == b.y ) && ( a.z == b.z ) && ( a.w == b.w ) );
}

inline int operator!=( const Quat &a, const Quat &b )
{
	return ( ( a.x != b.x ) || ( a.y != b.y ) || ( a.z != b.z ) && ( a.w != b.w ) );
}

inline float Quat::length( void	) const
{
	float	length;

	length = ( x * x ) + ( y * y ) + ( z * z ) + ( w * w );
	return sqrt( length );
}

inline const Quat & Quat::normalize( void )
{
	float	length, ilength;

	length = this->length();
	if ( length )
	{
		ilength = 1.0f / length;
		*this *= ilength;
	}

	return *this;
}

inline Quat Quat::operator-() const
{
	return Quat( -x, -y, -z, -w );
}

inline Vector Quat::toAngles( void )
{
	float m[ 3 ][ 3 ];
	vec3_t angles;

	QuatToMat( this->vec4(), m );
	MatrixToEulerAngles( m, angles );
	return Vector( angles );
}

inline Vector Vector::GetRotatedX( float angle ) const
{
	if( angle == 0.0 )
		return ( *this );

	float sinAngle = ( float )sin( M_PI*angle / 180 );
	float cosAngle = ( float )cos( M_PI*angle / 180 );

	return Vector( x,
		y*cosAngle - z*sinAngle,
		y*sinAngle + z*cosAngle );
}

inline void Vector::RotateX( double angle )
{
	( *this ) = GetRotatedX( angle );
}

inline Vector Vector::GetRotatedY( float angle ) const
{
	if( angle == 0.0 )
		return ( *this );

	float sinAngle = ( float )sin( M_PI*angle / 180 );
	float cosAngle = ( float )cos( M_PI*angle / 180 );

	return Vector( x*cosAngle + z*sinAngle,
		y,
		-x*sinAngle + z*cosAngle );
}

inline void Vector::RotateY( double angle )
{
	( *this ) = GetRotatedY( angle );
}

inline Vector Vector::GetRotatedZ( float angle ) const
{
	if( angle == 0.0 )
		return ( *this );

	float sinAngle = ( float )sin( M_PI*angle / 180 );
	float cosAngle = ( float )cos( M_PI*angle / 180 );

	return Vector( x*cosAngle - y*sinAngle,
		x*sinAngle + y*cosAngle,
		z );
}

inline void Vector::RotateZ( float angle )
{
	( *this ) = GetRotatedZ( angle );
}

inline void Vector::PackTo01()
{
	( *this ) = GetPackedTo01();
}

inline Vector Vector::GetPackedTo01() const
{
	Vector temp( *this );

	temp.normalize();

	temp = temp*0.5f + Vector( 0.5f, 0.5f, 0.5f );

	return temp;
}

#undef VECTOR_FABS

#endif /* Vector.h */
