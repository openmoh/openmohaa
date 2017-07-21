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

// SkelMat4.h : Skeletor

#ifndef __SKELMAT4_H__
#define __SKELMAT4_H__

#ifdef __cplusplus

class SkelQuat;

class SkelMat4 {
public:
	float val[ 4 ][ 3 ];

protected:
	void copy( const SkelMat4& );

public:
	void		MakeIdentity();
	void		MakeTranslate( float, float, float );
	void		MakeXRotation( float );
	void		MakeYRotation( float );
	void		MakeZRotation( float );

	SkelMat4( const float mat[ 3 ][ 3 ] );
	SkelMat4();

	SkelVec3	*XAxis();
	SkelVec3	*YAxis();
	SkelVec3	*ZAxis();
	SkelVec3	*XAxis() const;
	SkelVec3	*YAxis() const;
	SkelVec3	*ZAxis() const;

	operator float *( );
	operator float *( ) const;

	bool		IsOrthonormal() const;
	bool		IsValid() const;

	float		*operator[]( int index );
	float		*operator[]( int index ) const;

	int			CompareExact( const SkelMat4& skel ) const;

	void		Sum( const SkelMat4& m1, const SkelMat4& m2 );
	void		Difference( const SkelMat4& m1, const SkelMat4& m2 );
	void		Multiply( const SkelMat4& m1, const SkelMat4& m2 );
	void		InvertAxis( int );
	void		RotateBy( const SkelMat3& m );
	void		RotateByInverse( const SkelMat3& m );
	void		RotateXAxis( float x );
	void		RotateYAxis( float y );
	void		RotateZAxis( float z );
	void		RotateXAxis( float, float );
	void		RotateYAxis( float, float );
	void		RotateZAxis( float, float );
	void		MoveOnXAxis( float x );
	void		MoveOnYAxis( float y );
	void		MoveOnZAxis( float z );
	void		TransformVector( float * ) const;
	void		TransposeRot();
	void		TransposeOf( const SkelMat4& m );
	void		TransposeRotOf( const SkelMat4& m );
	void		InverseRotOf( const SkelMat4& m );
	float		Determinant( void );
	void		Inverse( void );
	void		GetRotationMatrix( float( *)[ 3 ] ) const;
	void		GetRotationMatrix( float( *)[ 4 ] ) const;
	void		ReplacePos( const float * );
	void		ReplaceRot( const SkelMat3& m );
	void		ReplaceRot( const SkelMat4& m );
	void		GetPos( float *pos ) const;
	void		GetScale( float *scale ) const;
	void		DeltaPos( const SkelMat4& m, float *delta ) const;
	void		RotateYaw( float, float );

	void		GetQuat( SkelQuat& quat );
};

inline
SkelMat4::SkelMat4( const float mat[ 3 ][ 3 ] )
{
	memcpy( &val, mat, sizeof( mat ) );
	val[ 3 ][ 0 ] = 0.0f;
	val[ 3 ][ 1 ] = 0.0f;
	val[ 3 ][ 2 ] = 0.0f;
}

inline
SkelMat4::SkelMat4()
{
	MakeIdentity();
}

inline
void SkelMat4::copy( const SkelMat4& m )
{
	MatrixCopy( *m.val, *val );
}

inline
void SkelMat4::MakeIdentity()
{
	memset( val, 0, sizeof( val ) );
	val[ 0 ][ 0 ] = 1.0f;
	val[ 1 ][ 1 ] = 1.0f;
	val[ 2 ][ 2 ] = 1.0f;
}

inline
SkelVec3 *SkelMat4::XAxis()
{
	return ( SkelVec3 * )&val[ 0 ];
}

inline
SkelVec3 *SkelMat4::YAxis()
{
	return ( SkelVec3 * )&val[ 1 ];
}

inline
SkelVec3 *SkelMat4::ZAxis()
{
	return ( SkelVec3 * )&val[ 2 ];
}

inline
SkelVec3 *SkelMat4::XAxis() const
{
	return ( SkelVec3 * )&val[ 0 ];
}

inline
SkelVec3 *SkelMat4::YAxis() const
{
	return ( SkelVec3 * )&val[ 1 ];
}

inline
SkelVec3 *SkelMat4::ZAxis() const
{
	return ( SkelVec3 * )&val[ 2 ];
}

inline
SkelMat4::operator float *( )
{
	return &val[ 0 ][ 0 ];
}

inline
SkelMat4::operator float *( ) const
{
	return ( float * )&val[ 0 ][ 0 ];
}

inline
float *SkelMat4::operator[]( int index )
{
	return val[ index ];
}

inline
float *SkelMat4::operator[]( int index ) const
{
	return ( float * )val[ index ];
}

inline
bool SkelMat4::IsOrthonormal() const
{
	// FIXME: stub
	return false;
}

inline
bool SkelMat4::IsValid() const
{
	// FIXME: stub
	return false;
}

inline
void SkelMat4::Sum( const SkelMat4& m1, const SkelMat4& m2 )
{
	VectorAdd( m1.val[ 0 ], m2.val[ 0 ], val[ 0 ] );
	VectorAdd( m1.val[ 1 ], m2.val[ 1 ], val[ 1 ] );
	VectorAdd( m1.val[ 2 ], m2.val[ 2 ], val[ 2 ] );
}

inline
void SkelMat4::Difference( const SkelMat4& m1, const SkelMat4& m2 )
{
	VectorSubtract( m1.val[ 0 ], m2.val[ 0 ], val[ 0 ] );
	VectorSubtract( m1.val[ 1 ], m2.val[ 1 ], val[ 1 ] );
	VectorSubtract( m1.val[ 2 ], m2.val[ 2 ], val[ 2 ] );
}

inline
void SkelMat4::Multiply( const SkelMat4& m1, const SkelMat4& m2 )
{
	val[ 0 ][ 0 ] = m1[ 0 ][ 0 ] * m2[ 0 ][ 0 ] + m1[ 0 ][ 1 ] * m2[ 1 ][ 0 ] + m1[ 0 ][ 2 ] * m2[ 2 ][ 0 ];
	val[ 1 ][ 0 ] = m1[ 1 ][ 0 ] * m2[ 0 ][ 0 ] + m1[ 1 ][ 1 ] * m2[ 1 ][ 0 ] + m1[ 1 ][ 2 ] * m2[ 2 ][ 0 ];
	val[ 2 ][ 0 ] = m1[ 2 ][ 0 ] * m2[ 0 ][ 0 ] + m1[ 2 ][ 1 ] * m2[ 1 ][ 0 ] + m1[ 2 ][ 2 ] * m2[ 2 ][ 0 ];
	val[ 3 ][ 0 ] = m1[ 3 ][ 0 ] * m2[ 0 ][ 0 ] + m1[ 3 ][ 1 ] * m2[ 1 ][ 0 ] + m1[ 3 ][ 2 ] * m2[ 2 ][ 0 ] + m2[ 3 ][ 0 ];

	val[ 0 ][ 1 ] = m1[ 0 ][ 0 ] * m2[ 0 ][ 1 ] + m1[ 0 ][ 1 ] * m2[ 1 ][ 1 ] + m1[ 0 ][ 2 ] * m2[ 2 ][ 1 ];
	val[ 1 ][ 1 ] = m1[ 1 ][ 0 ] * m2[ 0 ][ 1 ] + m1[ 1 ][ 1 ] * m2[ 1 ][ 1 ] + m1[ 1 ][ 2 ] * m2[ 2 ][ 1 ];
	val[ 2 ][ 1 ] = m1[ 2 ][ 0 ] * m2[ 0 ][ 1 ] + m1[ 2 ][ 1 ] * m2[ 1 ][ 1 ] + m1[ 2 ][ 2 ] * m2[ 2 ][ 1 ];
	val[ 3 ][ 1 ] = m1[ 3 ][ 0 ] * m2[ 0 ][ 1 ] + m1[ 3 ][ 1 ] * m2[ 1 ][ 1 ] + m1[ 3 ][ 2 ] * m2[ 2 ][ 1 ] + m2[ 3 ][ 1 ];

	val[ 0 ][ 2 ] = m1[ 0 ][ 0 ] * m2[ 0 ][ 2 ] + m1[ 0 ][ 1 ] * m2[ 1 ][ 2 ] + m1[ 0 ][ 2 ] * m2[ 2 ][ 2 ];
	val[ 1 ][ 2 ] = m1[ 1 ][ 0 ] * m2[ 0 ][ 2 ] + m1[ 1 ][ 1 ] * m2[ 1 ][ 2 ] + m1[ 1 ][ 2 ] * m2[ 2 ][ 2 ];
	val[ 2 ][ 2 ] = m1[ 2 ][ 0 ] * m2[ 0 ][ 2 ] + m1[ 2 ][ 1 ] * m2[ 1 ][ 2 ] + m1[ 2 ][ 2 ] * m2[ 2 ][ 2 ];
	val[ 3 ][ 2 ] = m1[ 3 ][ 0 ] * m2[ 0 ][ 2 ] + m1[ 3 ][ 1 ] * m2[ 1 ][ 2 ] + m1[ 3 ][ 2 ] * m2[ 2 ][ 2 ] + m2[ 3 ][ 2 ];
}

inline
void SkelMat4::InvertAxis( int a )
{
	val[ a ][ 0 ] = -val[ a ][ 0 ];	val[ a ][ 1 ] = -val[ a ][ 1 ];	val[ a ][ 2 ] = -val[ a ][ 2 ];
}

inline
float SkelMat4::Determinant( void )
{
	float mat4x4[ 4 ][ 4 ];

	mat4x4[ 0 ][ 0 ] = val[ 0 ][ 0 ];
	mat4x4[ 0 ][ 1 ] = val[ 0 ][ 1 ];
	mat4x4[ 0 ][ 2 ] = val[ 0 ][ 2 ];
	mat4x4[ 0 ][ 3 ] = 0;
	mat4x4[ 1 ][ 0 ] = val[ 1 ][ 0 ];
	mat4x4[ 1 ][ 1 ] = val[ 1 ][ 1 ];
	mat4x4[ 1 ][ 2 ] = val[ 1 ][ 2 ];
	mat4x4[ 1 ][ 3 ] = 0;
	mat4x4[ 2 ][ 0 ] = val[ 2 ][ 0 ];
	mat4x4[ 2 ][ 1 ] = val[ 2 ][ 1 ];
	mat4x4[ 2 ][ 2 ] = val[ 2 ][ 2 ];
	mat4x4[ 2 ][ 3 ] = 0;
	mat4x4[ 3 ][ 0 ] = val[ 3 ][ 0 ];
	mat4x4[ 3 ][ 1 ] = val[ 3 ][ 1 ];
	mat4x4[ 3 ][ 2 ] = val[ 3 ][ 2 ];
	mat4x4[ 3 ][ 3 ] = 1;

	return	mat4x4[ 0 ][ 0 ] * (
		mat4x4[ 1 ][ 1 ] * ( mat4x4[ 2 ][ 2 ] * mat4x4[ 3 ][ 3 ] - mat4x4[ 2 ][ 3 ] * mat4x4[ 3 ][ 2 ] ) -
		mat4x4[ 2 ][ 1 ] * ( mat4x4[ 1 ][ 2 ] * mat4x4[ 3 ][ 3 ] - mat4x4[ 1 ][ 3 ] * mat4x4[ 3 ][ 2 ] ) +
		mat4x4[ 3 ][ 1 ] * ( mat4x4[ 1 ][ 2 ] * mat4x4[ 2 ][ 3 ] - mat4x4[ 1 ][ 3 ] * mat4x4[ 2 ][ 2 ] )
		) -
		mat4x4[ 1 ][ 0 ] * (
			mat4x4[ 0 ][ 1 ] * ( mat4x4[ 2 ][ 2 ] * mat4x4[ 3 ][ 3 ] - mat4x4[ 2 ][ 3 ] * mat4x4[ 3 ][ 2 ] ) -
			mat4x4[ 2 ][ 1 ] * ( mat4x4[ 0 ][ 2 ] * mat4x4[ 3 ][ 3 ] - mat4x4[ 0 ][ 3 ] * mat4x4[ 3 ][ 2 ] ) +
			mat4x4[ 3 ][ 1 ] * ( mat4x4[ 0 ][ 2 ] * mat4x4[ 2 ][ 3 ] - mat4x4[ 0 ][ 3 ] * mat4x4[ 2 ][ 2 ] )
			) +
		mat4x4[ 2 ][ 0 ] * (
			mat4x4[ 0 ][ 1 ] * ( mat4x4[ 1 ][ 2 ] * mat4x4[ 3 ][ 3 ] - mat4x4[ 1 ][ 3 ] * mat4x4[ 3 ][ 2 ] ) -
			mat4x4[ 1 ][ 1 ] * ( mat4x4[ 0 ][ 2 ] * mat4x4[ 3 ][ 3 ] - mat4x4[ 0 ][ 3 ] * mat4x4[ 3 ][ 2 ] ) +
			mat4x4[ 3 ][ 1 ] * ( mat4x4[ 0 ][ 2 ] * mat4x4[ 1 ][ 3 ] - mat4x4[ 0 ][ 3 ] * mat4x4[ 1 ][ 2 ] )
			) -
		mat4x4[ 3 ][ 0 ] * (
			mat4x4[ 0 ][ 1 ] * ( mat4x4[ 1 ][ 2 ] * mat4x4[ 2 ][ 3 ] - mat4x4[ 1 ][ 3 ] * mat4x4[ 2 ][ 2 ] ) -
			mat4x4[ 1 ][ 1 ] * ( mat4x4[ 0 ][ 2 ] * mat4x4[ 2 ][ 3 ] - mat4x4[ 0 ][ 3 ] * mat4x4[ 2 ][ 2 ] ) +
			mat4x4[ 2 ][ 1 ] * ( mat4x4[ 0 ][ 2 ] * mat4x4[ 1 ][ 3 ] - mat4x4[ 0 ][ 3 ] * mat4x4[ 1 ][ 2 ] )
			);
}

inline
void SkelMat4::Inverse( void )
{
	float mat4x4[ 4 ][ 4 ];
	float outmat4x4[ 4 ][ 4 ];
	const float	Det = Determinant();

	mat4x4[ 0 ][ 0 ] = val[ 0 ][ 0 ];
	mat4x4[ 0 ][ 1 ] = val[ 0 ][ 1 ];
	mat4x4[ 0 ][ 2 ] = val[ 0 ][ 2 ];
	mat4x4[ 0 ][ 3 ] = 0;
	mat4x4[ 1 ][ 0 ] = val[ 1 ][ 0 ];
	mat4x4[ 1 ][ 1 ] = val[ 1 ][ 1 ];
	mat4x4[ 1 ][ 2 ] = val[ 1 ][ 2 ];
	mat4x4[ 1 ][ 3 ] = 0;
	mat4x4[ 2 ][ 0 ] = val[ 2 ][ 0 ];
	mat4x4[ 2 ][ 1 ] = val[ 2 ][ 1 ];
	mat4x4[ 2 ][ 2 ] = val[ 2 ][ 2 ];
	mat4x4[ 2 ][ 3 ] = 0;
	mat4x4[ 3 ][ 0 ] = val[ 3 ][ 0 ];
	mat4x4[ 3 ][ 1 ] = val[ 3 ][ 1 ];
	mat4x4[ 3 ][ 2 ] = val[ 3 ][ 2 ];
	mat4x4[ 3 ][ 3 ] = 1;

	if( Det == 0.0f )
	{
		MakeIdentity();
	}
	else
	{
		VectorMatrixInverse( outmat4x4, mat4x4 );

		val[ 0 ][ 0 ] = outmat4x4[ 0 ][ 0 ];
		val[ 0 ][ 1 ] = outmat4x4[ 0 ][ 1 ];
		val[ 0 ][ 2 ] = outmat4x4[ 0 ][ 2 ];
		val[ 1 ][ 0 ] = outmat4x4[ 1 ][ 0 ];
		val[ 1 ][ 1 ] = outmat4x4[ 1 ][ 1 ];
		val[ 1 ][ 2 ] = outmat4x4[ 1 ][ 2 ];
		val[ 2 ][ 0 ] = outmat4x4[ 2 ][ 0 ];
		val[ 2 ][ 1 ] = outmat4x4[ 2 ][ 1 ];
		val[ 2 ][ 2 ] = outmat4x4[ 2 ][ 2 ];
		val[ 3 ][ 0 ] = outmat4x4[ 3 ][ 0 ];
		val[ 3 ][ 1 ] = outmat4x4[ 3 ][ 1 ];
		val[ 3 ][ 2 ] = outmat4x4[ 3 ][ 2 ];
	}
}

inline
void SkelMat4::RotateBy( const SkelMat3& m )
{
	SkelMat4 temp = *this;

	val[ 0 ][ 0 ] = temp[ 0 ][ 0 ] * m[ 0 ][ 0 ] + temp[ 0 ][ 1 ] * m[ 1 ][ 0 ] + temp[ 0 ][ 2 ] * m[ 2 ][ 0 ];
	val[ 0 ][ 1 ] = temp[ 0 ][ 0 ] * m[ 0 ][ 1 ] + temp[ 0 ][ 1 ] * m[ 1 ][ 1 ] + temp[ 0 ][ 2 ] * m[ 2 ][ 1 ];
	val[ 0 ][ 2 ] = temp[ 0 ][ 0 ] * m[ 0 ][ 2 ] + temp[ 0 ][ 1 ] * m[ 1 ][ 2 ] + temp[ 0 ][ 2 ] * m[ 2 ][ 2 ];

	val[ 1 ][ 0 ] = temp[ 1 ][ 0 ] * m[ 0 ][ 0 ] + temp[ 1 ][ 1 ] * m[ 1 ][ 0 ] + temp[ 1 ][ 2 ] * m[ 2 ][ 0 ];
	val[ 1 ][ 1 ] = temp[ 1 ][ 0 ] * m[ 0 ][ 1 ] + temp[ 1 ][ 1 ] * m[ 1 ][ 1 ] + temp[ 1 ][ 2 ] * m[ 2 ][ 1 ];
	val[ 1 ][ 2 ] = temp[ 1 ][ 0 ] * m[ 0 ][ 2 ] + temp[ 1 ][ 1 ] * m[ 1 ][ 2 ] + temp[ 1 ][ 2 ] * m[ 2 ][ 2 ];

	val[ 2 ][ 0 ] = temp[ 2 ][ 0 ] * m[ 0 ][ 0 ] + temp[ 2 ][ 1 ] * m[ 1 ][ 0 ] + temp[ 2 ][ 2 ] * m[ 2 ][ 0 ];
	val[ 2 ][ 1 ] = temp[ 2 ][ 0 ] * m[ 0 ][ 1 ] + temp[ 2 ][ 1 ] * m[ 1 ][ 1 ] + temp[ 2 ][ 2 ] * m[ 2 ][ 1 ];
	val[ 2 ][ 2 ] = temp[ 2 ][ 0 ] * m[ 0 ][ 2 ] + temp[ 2 ][ 1 ] * m[ 1 ][ 2 ] + temp[ 2 ][ 2 ] * m[ 2 ][ 2 ];
}

inline
void SkelMat4::RotateByInverse( const SkelMat3& m )
{
	// FIXME: stub
}

inline
void SkelMat4::TransposeRotOf( const SkelMat4& m )
{
	val[ 0 ][ 0 ] = m.val[ 0 ][ 0 ];
	val[ 0 ][ 1 ] = m.val[ 1 ][ 0 ];
	val[ 0 ][ 2 ] = m.val[ 2 ][ 0 ];
	val[ 1 ][ 0 ] = m.val[ 0 ][ 1 ];
	val[ 1 ][ 1 ] = m.val[ 1 ][ 1 ];
	val[ 1 ][ 2 ] = m.val[ 2 ][ 1 ];
	val[ 2 ][ 0 ] = m.val[ 0 ][ 2 ];
	val[ 2 ][ 1 ] = m.val[ 1 ][ 2 ];
	val[ 2 ][ 2 ] = m.val[ 2 ][ 2 ];
}

inline
void SkelMat4::GetQuat( SkelQuat& quat )
{
	MatToQuat( val, ( float * )&quat );
}

#else

typedef struct {
	float val[ 4 ][ 3 ];
} SkelMat4;

#endif

#endif // __SKELMAT4_H__
