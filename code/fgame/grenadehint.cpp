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

// grenadehint.cpp: Grenade hint for actors.

#include "grenadehint.h"

CLASS_DECLARATION( SimpleEntity, GrenadeHint, "info_grenadehint" )
{
	{ NULL, NULL }
};

GrenadeHint *GrenadeHint::gm_pFirst;

GrenadeHint::GrenadeHint()
{
	m_pNext = gm_pFirst;
	gm_pFirst = this;
}

void GrenadeHint::ResetHints
	(
	void
	)
{
	gm_pFirst = NULL;
}

int GrenadeHint::GetClosestSet
	(
	GrenadeHint **ppHints,
	int nHints,
	Vector& vOrg,
	float fMaxRangeSquared
	)
{
	int i;
	int nFound = 0;
	GrenadeHint *pHint;
	Vector vDelta;
	float afRangeSquared[ 256 ];
	float fRangeSquared;

	if( nHints > 256 )
		nHints = 256;

	// iterate through all grenade hints and get the closest
	for( pHint = gm_pFirst; pHint != NULL; pHint = pHint->m_pNext )
	{
		vDelta = pHint->origin - vOrg;

		fRangeSquared = VectorLengthSquared( vDelta );

		if( fRangeSquared > fMaxRangeSquared )
		{
			for( i = nFound; i > 0; i-- )
			{
				if( fRangeSquared > afRangeSquared[ i - 1 ]  )
					break;

				if( i < nFound || nFound < nHints )
				{
					afRangeSquared[ i ] = afRangeSquared[ i - 1 ];
					ppHints[ i ] = ppHints[ i - 1 ];
				}
			}

			if( i < nHints )
			{
				afRangeSquared[ i ] = fRangeSquared;
				ppHints[ i ] = pHint;

				if( nFound < nHints )
					nFound++;
			}
		}
	}

	return nFound;
}
