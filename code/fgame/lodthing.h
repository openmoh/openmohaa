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

// lodthing.cpp : Level of detail manager

#include "animate.h"

class LODMaster : public Listener {
public:
	CLASS_PROTOTYPE( LODMaster );

	SafePtr< Entity > m_current;

	void			Init( void );
	void			Spawn( Event *ev );
	void			PassEvent( Event *ev );

	void Archive( Archiver& arc ) override;
};

inline void LODMaster::Archive
	(
	Archiver& arc
	)
{
	Listener::Archive( arc );

	arc.ArchiveSafePointer( &m_current );
}

class LODSlave : public Animate {
public:
	CLASS_PROTOTYPE( LODSlave );

	Vector	m_baseorigin;
	float	m_scale;

	LODSlave();

	void			UpdateCvars( qboolean quiet, qboolean updateFrame );
	void			ThinkEvent( Event *ev );
	void			SetModelEvent( Event *ev );
	void			Delete( Event *ev );

	void Archive( Archiver& arc ) override;
};

inline void LODSlave::Archive
	(
	Archiver& arc
	)
{
	Animate::Archive( arc );

	arc.ArchiveVector( &m_baseorigin );
	arc.ArchiveFloat( &m_scale );
}

extern LODMaster LODModel;
