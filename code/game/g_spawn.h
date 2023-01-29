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

// g_spawn.h : spawner for scripts.

#ifndef __G_SPAWN_H__
#define __G_SPAWN_H__

#include "glb_local.h"

// spawnflags
// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG_NOT_EASY			0x00000100
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200
#define	SPAWNFLAG_NOT_HARD			0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define	SPAWNFLAG_DEVELOPMENT		0x00002000
#define	SPAWNFLAG_DETAIL			0x00004000
#define	SPAWNFLAG_NOCONSOLE			0x00008000
#define	SPAWNFLAG_NOPC				0x00010000

class Listener;

class SpawnArgs : public Class
{
private:
	Container<str> keyList;
	Container<str> valueList;

public:
	CLASS_PROTOTYPE( SpawnArgs );

	SpawnArgs();
	SpawnArgs( SpawnArgs &arglist );

	void           Clear( void );

	char			*Parse( char *data, bool bAllowUtils = false );
	const char     *getArg( const char *key, const char *defaultValue = NULL );
	void           setArg( const char *key, const char *value );

	int            NumArgs( void );
	const char     *getKey( int index );
	const char     *getValue( int index );
	void           operator=( SpawnArgs &a );

	ClassDef       *getClassDef( qboolean *tikiWasStatic = NULL );
	Listener	   *Spawn( void );
	Listener	   *SpawnInternal( void );

	virtual void   Archive( Archiver &arc );
};

extern Container< SafePtr< Listener > > g_spawnlist;

ClassDef			*FindClass( const char *name, qboolean *isModel );

#ifdef GAME_DLL
void				G_InitClientPersistant( gclient_t *client );
#endif

#endif
