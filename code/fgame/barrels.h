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

// barrels.h : Barrels

#ifndef __BARRELOBJECT_H__
#define __BARRELOBJECT_H__

#include "entity.h"

#define BARREL_INDESTRUCTABLE		1
#define MAX_BARREL_LEAKS			4

enum
{
	BARREL_EMPTY,
	BARREL_OIL,
	BARREL_WATER,
	BARREL_GAS
};

class BarrelObject : public Entity {
	int m_iBarrelType;
	float m_fFluidAmount;
	float m_fHeightFluid;
	qboolean m_bLeaksActive[ MAX_BARREL_LEAKS ];
	Vector m_vLeaks[ MAX_BARREL_LEAKS ];
	Vector m_vLeakNorms[ MAX_BARREL_LEAKS ];
	Vector m_vJitterAngles;
	Vector m_vStartAngles;
	float m_fJitterScale;
	float m_fLastEffectTime;
	float m_fDamageSoundTime;

public:
	CLASS_PROTOTYPE( BarrelObject );

	BarrelObject();

	int				PickBarrelLeak( void );

	void			BarrelSetup( Event *ev );
	void			BarrelSetType( Event *ev );
	void			BarrelThink( Event *ev );
	void			BarrelDamaged( Event *ev );
	void			BarrelKilled( Event *ev );

	void Archive( Archiver& arc ) override;
};

inline void BarrelObject::Archive
	(
	Archiver& arc
	)

{
	Entity::Archive( arc );

	arc.ArchiveInteger( &m_iBarrelType );
	arc.ArchiveFloat( &m_fFluidAmount );
	arc.ArchiveFloat( &m_fHeightFluid );
	arc.ArchiveVector( &m_vJitterAngles );
	arc.ArchiveVector( &m_vStartAngles );
	arc.ArchiveFloat( &m_fJitterScale );
	arc.ArchiveFloat( &m_fLastEffectTime );
	arc.ArchiveFloat( &m_fDamageSoundTime );

	for( int i = MAX_BARREL_LEAKS - 1; i >= 0; i-- )
	{
		arc.ArchiveBoolean( &m_bLeaksActive[ i ] );
		arc.ArchiveVector( &m_vLeaks[ i ] );
		arc.ArchiveVector( &m_vLeakNorms[ i ] );
	}
}

#endif // __BARRELOBJECT_H__
