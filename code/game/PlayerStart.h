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

// PlayerStart.h: Player start location entity declarations
// 

#ifndef __PLAYERSTART_H__
#define __PLAYERSTART_H__

#include "g_local.h"
#include "simpleentity.h"
#include "camera.h"
#include "navigate.h"

class PlayerStart : public SimpleArchivedEntity
{
public:
	bool		m_bForbidSpawns;
	bool		m_bDeleteOnSpawn;

public:
	CLASS_PROTOTYPE( PlayerStart );

	PlayerStart();

	virtual void	Archive( Archiver &arc );

	void			SetAngle( Event *ev );
	void			EventEnableSpawn( Event *ev );
	void			EventDisableSpawn( Event *ev );
	void			EventDeleteOnSpawn( Event *ev );
	void			EventKeepOnSpawn( Event *ev );
};

inline void PlayerStart::Archive( Archiver &arc )
{
	SimpleArchivedEntity::Archive( arc );

	arc.ArchiveBool( &m_bForbidSpawns );
	arc.ArchiveBool( &m_bDeleteOnSpawn );
}

class TestPlayerStart : public PlayerStart
	{
	public:
		CLASS_PROTOTYPE( TestPlayerStart );
	};

class PlayerDeathmatchStart : public PlayerStart
	{
	public:
		CLASS_PROTOTYPE( PlayerDeathmatchStart );
	};

class PlayerAlliedDeathmatchStart : public PlayerDeathmatchStart
{
public:
	CLASS_PROTOTYPE( PlayerAlliedDeathmatchStart );
};

class PlayerAxisDeathmatchStart : public PlayerDeathmatchStart
{
public:
	CLASS_PROTOTYPE( PlayerAxisDeathmatchStart );
};

class PlayerIntermission : public Camera
	{
	public:
		CLASS_PROTOTYPE( PlayerIntermission );
      PlayerIntermission();
	};

#endif /* PlayerStart.h */
