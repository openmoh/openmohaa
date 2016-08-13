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

// con_set.cpp: C++ map/set classes

#include "glb_local.h"

// Basic Hash functions

template<>
int HashCode< const char * >( const char * const& key )
{
	const char *p;
	int hash = 0;

	for( p = key; *p != NULL; p++ )
	{
		hash = *p + 31 * hash;
	}

	return hash;
}

template<>
int HashCode< void * >( void * const& key )
{
	return 0;
}

template<>
int HashCode< const void * >( const void * const& key )
{
	return 0;
}

template<>
int HashCode< char >( const char& key )
{
	return key;
}

template<>
int HashCode< unsigned char >( const unsigned char& key )
{
	return key;
}

template<>
int HashCode< unsigned char * >( unsigned char * const& key )
{
	return ( int )key;
}

template<>
int HashCode< short >( const short& key )
{
	return key;
}

template<>
int HashCode< unsigned short >( const unsigned short& key )
{
	return key;
}

template<>
int HashCode< short3 >( const short3& key )
{
	return key;
}

template<>
int HashCode< unsigned_short3 >( const unsigned_short3& key )
{
	return key;
}

template<>
int HashCode< int >( const int& key )
{
	return key;
}

template<>
int HashCode< unsigned int >( const unsigned int& key )
{
	return key;
}

template<>
int HashCode< float >( const float& key )
{
	return *( int * )&key;
}

template<>
int HashCode< double >( const double& key )
{
	return *( int * )&key;
}

template<>
int HashCode< str >( const str& key )
{
	return HashCode< const char * >( key.c_str() );
}

template<>
int HashCode< Vector >( const Vector& key )
{
	return ( int )( ( key[ 0 ] + key[ 1 ] + key[ 2 ] ) / 3 );
}
