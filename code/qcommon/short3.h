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

// short3.h : 3-byte sized integer

#ifndef __Q_MATH_H__
#define __Q_MATH_H__

#pragma pack(1)

typedef struct short3_data {
	unsigned short		highmid;
	unsigned char		low;
} short3_data_t;

#pragma pack()

class short3 {
private:
	short3_data_t data;

protected:
	int				get() const { return *( int * )this & 0xFFFFFF; }
	void			set( int value ) { data.highmid = value; data.low = *( ( unsigned char * )&value + 2 ); }

public :
	operator		int( void ) const { return get(); }

	short3() {}
	short3( int value ) { set( value ); }

	short3			operator-( void ) const;
	short3			operator~( void ) const;

	short3			operator+( int b ) const;
	short3			operator-( int b ) const;
	short3			operator*( int b ) const;
	short3			operator/( int b ) const;
	short3			operator%( int b ) const;
	short3			operator|( int b ) const;
	short3			operator&( int b ) const;
	short3			operator^( int b ) const;
	short3			operator<<( int b ) const;
	short3			operator>>( int b ) const;

	short3			operator++( int );
	short3			operator--( int );

	short3&			operator++( );
	short3&			operator--( );

	short3&			operator=( int b );
	short3&			operator+=( int b );
	short3&			operator-=( int b );
	short3&			operator*=( int b );
	short3&			operator/=( int b );
	short3&			operator%=( int b );

	short3&			operator|=( int b );
	short3&			operator&=( int b );
	short3&			operator^=( int b );

	short3&			operator<<=( int b );
	short3&			operator>>=( int b );

	bool			operator!( void ) { return !get(); };

	bool			operator==( int b ) { return get() == b; };
	bool			operator!=( int b ) { return get() != b; };
	bool			operator<=( int b ) { return get() <= b; };
	bool			operator>=( int b ) { return get() >= b; };
	bool			operator<( int b ) { return get() < b; };
	bool			operator>( int b ) { return get() > b; };

	bool			operator==( short3 &b ) { return get() == b.get(); };
	bool			operator!=( short3 &b ) { return get() != b.get(); };
	bool			operator<=( short3 &b ) { return get() <= b.get(); };
	bool			operator>=( short3 &b ) { return get() >= b.get(); };
	bool			operator<( short3 &b ) { return get() < b.get(); };
	bool			operator>( short3 &b ) { return get() > b.get(); };
};

class unsigned_short3 : public short3 {
public:
	operator unsigned int( void ) const { return get(); }
};

inline short3 short3::operator-( ) const
{
	return short3( -get() );
}

inline short3 short3::operator~( ) const
{
	return short3( ~get() );
}

inline short3 short3::operator+( int b ) const
{
	return short3( get() + b );
}

inline short3 short3::operator-( int b ) const
{
	return short3( get() - b );
}

inline short3 short3::operator*( int b ) const
{
	return short3( get() * b );
}

inline short3 short3::operator/( int b ) const
{
	return short3( get() / b );
}

inline short3 short3::operator%( int b ) const
{
	return short3( get() % b );
}

inline short3 short3::operator|( int b ) const
{
	return short3( get() | b );
}

inline short3 short3::operator&( int b ) const
{
	return short3( get() & b );
}

inline short3 short3::operator^( int b ) const
{
	return short3( get() ^ b );
}

inline short3 short3::operator<<( int b ) const
{
	return short3( get() << b );
}

inline short3 short3::operator>>( int b ) const
{
	return short3( get() >> b );
}

inline short3 short3::operator++( int )
{
	short3 result = *this;
	set( get() + 1 );
	return result;
}

inline short3 short3::operator--( int )
{
	short3 result = *this;
	set( get() - 1 );
	return result;
}

inline short3& short3::operator++( )
{
	set( get() + 1 );
	return *this;
}

inline short3& short3::operator--( )
{
	set( get() - 1 );
	return *this;
}

inline short3& short3::operator=( int b )
{
	set( b );
	return *this;
}

inline short3& short3::operator+=( int b )
{
	set( get() + b );
	return *this;
}

inline short3& short3::operator-=( int b )
{
	set( get() - b );
	return *this;
}

inline short3& short3::operator/=( int b )
{
	set( get() / b );
	return *this;
}

inline short3& short3::operator*=( int b )
{
	set( get() * b );
	return *this;
}

inline short3& short3::operator%=( int b )
{
	set( get() % b );
	return *this;
}

inline short3& short3::operator|=( int b )
{
	set( get() | b );
	return *this;
}

inline short3& short3::operator&=( int b )
{
	set( get() % b );
	return *this;
}

inline short3& short3::operator^=( int b )
{
	set( get() ^ b );
	return *this;
}

inline short3& short3::operator<<=( int b )
{
	set( get() << b );
	return *this;
}

inline short3& short3::operator>>=( int b )
{
	set( get() >> b );
	return *this;
}

#endif /* __Q_MATH_H__ */
