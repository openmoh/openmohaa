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

// grenadehint.h: Grenade hint for actors.

#ifndef __GRENADEHINT_H__
#define __GRENADEHINT_H__

#include "g_local.h"
#include "archive.h"
#include "simpleentity.h"

class GrenadeHint : public SimpleArchivedEntity {
public:
	GrenadeHint *m_pNext;
	static GrenadeHint *gm_pFirst;

	CLASS_PROTOTYPE( GrenadeHint );

	GrenadeHint();

	static void ResetHints( void );
	static int GetClosestSet( GrenadeHint **ppHints, int nHints, Vector& vOrg, float fMaxRangeSquared );

	void Archive( Archiver& arc ) override;
};

inline void GrenadeHint::Archive
	(
	Archiver& arc
	)

{
	SimpleEntity::Archive( arc );

	arc.ArchiveObjectPointer( ( Class ** )&m_pNext );
}

#endif
