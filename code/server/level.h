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

// level.h : TU Level.

#ifndef __LEVEL_H__
#define __LEVEL_H__

#include <listener.h>

class SimpleArchivedEntity;

class Level : public Listener {
public:
	bool		m_LoopDrop;
	bool		m_LoopProtection;

	str	m_mapscript;
	str	current_map;

	// Level time
	int framenum;
	int inttime;
	int intframetime;

	float time;
	float frametime;

	int		spawnflags;

	// Server time
	int		svsTime;
	float	svsFloatTime;
	int		svsStartTime;
	int		svsEndTime;

	bool	m_bScriptSpawn;
	bool	m_bRejectSpawn;

	Container< SimpleArchivedEntity * > m_SimpleArchivedEntities;

public:
	CLASS_PROTOTYPE( Level );

	void			setTime( int _svsTime_ );
	void			setFrametime( int frameTime );
};

extern Level level;

#endif /* __LEVEL_H__*/