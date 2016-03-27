/*
===========================================================================
Copyright (C) 2008 the OpenMoHAA team

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

// object.h: Object (used by common TIKIs)

#ifndef __OBJECT_H__
#define __OBJECT_H__

#if defined ( GAME_DLL ) || defined ( CGAME_DLL )

#include "animate.h"

class Object : public Animate {
public:
	CLASS_PROTOTYPE( Object );
};

#else

#include "simpleentity.h"

class Object : public SimpleEntity {
public:
	CLASS_PROTOTYPE( Object );
};

#endif

#endif /* __OBJECT_H__ */
