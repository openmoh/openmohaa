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

// crateobject.h : Crates

#ifndef __CRATEOBJECT_H__
#define __CRATEOBJECT_H__

#include "entity.h"

#define CRATE_INDESTRUCTABLE		1
#define CRATE_NOTSTACKEDON			2

class CrateObject : public Entity {
	float m_fMoveTime;
	int m_iDebrisType;
	Vector m_vJitterAngles;
	Vector m_vStartAngles;
	float m_fJitterScale;

private:
	void TellNeighborsToFall( void );
	void TellNeighborsToJitter( Vector vJitterAdd );

public:
	CLASS_PROTOTYPE( CrateObject );

	CrateObject();

	void			CrateSetup( Event *ev );
	void			CrateDebrisType( Event *ev );
	void			StartFalling( Event *ev );
	void			CrateFalling( Event *ev );
	void			CrateDamaged( Event *ev );
	void			CrateKilled( Event *ev );
	void			CrateThink( Event *ev );
	void Archive( Archiver& arc ) override;
};

inline void CrateObject::Archive
	(
	Archiver& arc
	)

{
	Entity::Archive( arc );

	arc.ArchiveFloat( &m_fMoveTime );
	arc.ArchiveInteger( &m_iDebrisType );

	arc.ArchiveVector( &m_vJitterAngles );
	arc.ArchiveVector( &m_vStartAngles );
	arc.ArchiveFloat( &m_fJitterScale );
}

#endif // __CRATEOBJECT_H__
