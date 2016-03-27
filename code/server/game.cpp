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

// game.cpp : TU Game.

#include "game.h"

Game game;

void CacheResource( const char *name )
{

}

SimpleEntity *G_FindTarget( SimpleEntity *next, const char *classname )
{
	return NULL;
}

Vector G_GetMovedir( float angle )
{
	if( angle == -1.0f )
	{
		return Vector( 0.0f, 0.0f, 1.0f );
	}
	else if( angle == -2.0f )
	{
		return Vector( 0.0f, 0.0f, -1.0f );
	}

	angle *= ( M_PI * 2.0f / 360.0f );
	return Vector( cos( angle ), sin( angle ), 0.0f );
}

float G_Random( float value )
{
	return fmod( rand(), value );
}

CLASS_DECLARATION( Listener, Game, NULL )
{
	{ NULL, NULL }
};
