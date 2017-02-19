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

// tiki_utility.cpp : TIKI Utility

#include "q_shared.h"
#include "qcommon.h"
#include "tiki_shared.h"
#include "dbgheap.h"

/*
===============
TIKI_Name
===============
*/
const char *TIKI_Name( dtiki_t *pmdl )
{
	if( pmdl )
	{
		return pmdl->a->name;
	}
	else
	{
		return 0;
	}
}

/*
===============
TIKI_NumAnims
===============
*/
int TIKI_NumAnims( dtiki_t *pmdl )
{
	if( pmdl )
	{
		return pmdl->a->num_anims;
	}
	else
	{
		return 0;
	}
}

/*
===============
TIKI_NumSurfaces
===============
*/
int TIKI_NumSurfaces( dtiki_t *pmdl )
{
	if( pmdl )
	{
		return pmdl->num_surfaces;
	}
	else
	{
		return 0;
	}
}

/*
===============
TIKI_NumTags
===============
*/
int TIKI_NumTags( dtiki_t *pmdl )
{
	return pmdl->m_boneList.NumChannels();
}

/*
===============
TIKI_CalculateBounds
===============
*/
void TIKI_CalculateBounds( dtiki_t *pmdl, float scale, vec3_t mins, vec3_t maxs )
{
	int j;

	for( j = 0; j < 3; j++ )
	{
		mins[ j ] = pmdl->a->mins[ j ] * pmdl->load_scale * scale;
		maxs[ j ] = pmdl->a->maxs[ j ] * pmdl->load_scale * scale;
	}
}

/*
===============
TIKI_GlobalRadius
===============
*/
float TIKI_GlobalRadius( dtiki_t *pmdl )
{
	return pmdl->radius;
}

/*
===============
TIKI_NumHeadModels
===============
*/
int TIKI_NumHeadModels( dtikianim_t *tiki )
{
	int numheadmodels = 0;
	const char *s = tiki->headmodels;

	while( 1 )
	{
		s = strstr( s, "\n" );
		if( !s ) {
			break;
		}
		numheadmodels++;
		s++;
	}

	return numheadmodels;
}

/*
===============
TIKI_GetHeadModel
===============
*/
void TIKI_GetHeadModel( dtikianim_t *tiki, int num, char *name )
{
	const char *s, *t;

	if( tiki->headmodels == ( char * )1 )
	{
		*name = 0;
		return;
	}

	s = tiki->headmodels;
	for( num = num - 1; num >= 0; num-- )
	{
		s = strstr( s, "\n" );
		if( !s )
		{
			*name = 0;
			return;
		}
		s++;
	}

	t = strstr( s, "\n" );
	if( t )
	{
		strncpy( name, s, t - s );
		name[ t - s ] = 0;
	}
	else
	{
		*name = 0;
	}
}

/*
===============
TIKI_NumHeadSkins
===============
*/
int TIKI_NumHeadSkins( dtikianim_t *tiki )
{
	int numheadskins = 0;
	const char *s = tiki->headskins;

	while( 1 )
	{
		s = strstr( s, "\n" );
		if( !s ) {
			break;
		}
		numheadskins++;
		s++;
	}

	return numheadskins;
}

/*
===============
TIKI_GetHeadSkin
===============
*/
void TIKI_GetHeadSkin( dtikianim_t *tiki, int num, char *name )
{
	const char *s, *t;

	if( tiki->headskins == ( char * )1 )
	{
		*name = 0;
		return;
	}

	s = tiki->headmodels;
	for( num = num - 1; num >= 0; num-- )
	{
		s = strstr( s, "\n" );
		if( !s )
		{
			*name = 0;
			return;
		}
		s++;
	}

	t = strstr( s, "\n" );
	if( t )
	{
		strncpy( name, s, t - s );
		name[ t - s ] = 0;
	}
	else
	{
		*name = 0;
	}
}
