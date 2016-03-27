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

// game.h : TU Game.

#ifndef __GAME_H__
#define __GAME_H__

#include <listener.h>

class Game : public Listener {
	CLASS_PROTOTYPE( Game );
};

extern Game game;

SimpleEntity *G_FindTarget( SimpleEntity *next, const char *classname );
Vector G_GetMovedir( float angle );
float G_Random( float value );

#endif /* __GAME_H__*/