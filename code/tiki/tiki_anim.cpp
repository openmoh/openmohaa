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

// tiki_anim.cpp : TIKI Anim

#include "q_shared.h"
#include "qcommon.h"
#include "../skeletor/skeletor.h"
#include <mem_blockalloc.h>
#include <con_set.h>
#include "tiki_files.h"
#include "dbgheap.h"

/*
===============
AnimCompareFunc
===============
*/
static int AnimCompareFunc( const void *a, const void *b )
{
	return stricmp( loaddef.loadanims[ *( int * )a ]->alias, loaddef.loadanims[ *( int * )b ]->alias );
}

/*
===============
TIKI_GetAnimOrder
===============
*/
void TIKI_GetAnimOrder( dloaddef_t *ld, int *order )
{
	int i;

	for( i = 0; i < ld->numanims; i++ )
		order[ i ] = i;
	qsort( order, ld->numanims, sizeof( int ), AnimCompareFunc );
}

/*
===============
TIKI_Anim_NameForNum
===============
*/
const char *TIKI_Anim_NameForNum( dtiki_t *pmdl, int animnum )
{ 
	dtikianimdef_t *panimdef = pmdl->a->animdefs[ animnum ];
	return panimdef->alias;
}

/*
===============
TIKI_Anim_NumForName
===============
*/
int TIKI_Anim_NumForName( dtiki_t *pmdl, const char *name )
{
	int iTop;
	int iBottom;
	int iMiddle;
	int iComp;
	dtikianimdef_t *panimdef;
	float fAnimWeights[ MAX_FRAMEINFOS ];
	float fWeight;
	float fTotalWeight;
	int iAnimCount;
	int i, k;

	if( !pmdl )
	{
		return -1;
	}

	iBottom = 0;
	iTop = pmdl->a->num_anims - 1;

	while( iBottom <= iTop )
	{
		iMiddle = ( iBottom + iTop ) / 2;

		panimdef = pmdl->a->animdefs[ iMiddle ];
		if( !panimdef )
		{
			iComp = -1;
		}
		else
		{
			iComp = stricmp( panimdef->alias, name );
		}

		if( !iComp )
		{
			if( !( panimdef->flags & TAF_RANDOM ) )
			{
				return iMiddle;
			}

			for( i = iMiddle; i > 0; i-- )
			{
				if( !pmdl->a->animdefs[ i - 1 ] || stricmp( panimdef->alias, pmdl->a->animdefs[ i - 1 ]->alias ) ) {
					break;
				}
			}

			k = i;

			for( iMiddle++; iMiddle < pmdl->a->num_anims; iMiddle++ )
			{
				if( !pmdl->a->animdefs[ iMiddle ] || stricmp( panimdef->alias, pmdl->a->animdefs[ iMiddle ]->alias ) ) {
					break;
				}
			}

			fTotalWeight = 0.0f;
			iAnimCount = 0;

			for( ; i < iMiddle; i++ )
			{
				panimdef = pmdl->a->animdefs[ i ];
				if( !panimdef )
				{
					continue;
				}

				if( panimdef->flags & TAF_AUTOSTEPS_DOG )
				{
					fAnimWeights[ iAnimCount ] = 0.0f;
					panimdef->flags &= ~TAF_AUTOSTEPS_DOG;
				}
				else
				{
					fAnimWeights[ iAnimCount ] = panimdef->weight;
					fTotalWeight += panimdef->weight;
				}

				iAnimCount++;
			}

			fWeight = randweight() * fTotalWeight;
			for( i = 0; i < iAnimCount; i++ )
			{
				if( fWeight < fAnimWeights[ i ] ) {
					break;
				}

				fWeight -= fAnimWeights[ i ];
			}


			iMiddle = i + k;
			panimdef = pmdl->a->animdefs[ iMiddle ];
			if( panimdef && panimdef->flags & TAF_NOREPEAT )
			{
				panimdef->flags |= TAF_AUTOSTEPS_DOG;
			}

			return iMiddle;
		}

		if( iComp > 0 )
		{
			iTop = iMiddle - 1;
		}
		else
		{
			iBottom = iMiddle + 1;
		}
	}

	return -1;
}

/*
===============
TIKI_Anim_Random
===============
*/
int TIKI_Anim_Random( dtiki_t *pmdl, const char *name )
{
	dtikianimdef_t *panimdef;
	int i;
	float totalweight;
	float weights[ MAX_FRAMEINFOS ];
	int anim[ MAX_FRAMEINFOS ];
	int num;
	size_t len;
	int diff;
	float weight;

	len = strlen( name );
	if( !len || !pmdl )
	{
		return -1;
	}

	num = 0;
	totalweight = 0.0f;
	for( i = 0; i < pmdl->a->num_anims; i++ )
	{
		panimdef = pmdl->a->animdefs[ i ];
		diff = strnicmp( panimdef->alias, name, len );
		if( diff || panimdef->alias[ len ] == '_' )
		{
			if( diff > 0 )
				break;
		}
		else
		{
			if( num >= MAX_FRAMEINFOS )
				break;

			totalweight += panimdef->weight;
			anim[ num ] = i;
			weights[ num ] = panimdef->weight;

			num++;
		}
	}

	// animation name not found
	if( !num )
	{
		return -1;
	}

	// find a random animation based on the weight
	weight = randweight() * totalweight;
	for( i = 0; i < num; i++ )
	{
		if( weight < weights[ i ] ) {
			break;
		}

		weight -= weights[ i ];
	}

	return anim[ i ];
}

/*
===============
TIKI_Anim_NumFrames
===============
*/
int TIKI_Anim_NumFrames( dtiki_t *pmdl, int animnum )
{
	skelAnimDataGameHeader_t *animData = SkeletorCacheGetData( pmdl->a->m_aliases[ animnum ] );
	return animData->numFrames;
}

/*
===============
TIKI_Anim_Time
===============
*/
float TIKI_Anim_Time( dtiki_t *pmdl, int animnum )
{
	skelAnimDataGameHeader_t *animData = SkeletorCacheGetData( pmdl->a->m_aliases[ animnum ] );

	return animData->flags & TAF_DELTADRIVEN
		? animData->frameTime * animData->numFrames
		: animData->frameTime * ( animData->numFrames - 1 );
}

/*
===============
TIKI_Anim_Frametime
===============
*/
float TIKI_Anim_Frametime( dtiki_t *pmdl, int animnum )
{
	skelAnimDataGameHeader_t *animData = SkeletorCacheGetData( pmdl->a->m_aliases[ animnum ] );
	return animData->frameTime;
}

/*
===============
TIKI_Anim_Delta
===============
*/
void TIKI_Anim_Delta( dtiki_t *pmdl, int animnum, float *delta )
{
	skelAnimDataGameHeader_t *animData = SkeletorCacheGetData( pmdl->a->m_aliases[ animnum ] );
	VectorScale( animData->totalDelta, pmdl->load_scale, delta );
}

/*
===============
TIKI_Anim_HasDelta
===============
*/
qboolean TIKI_Anim_HasDelta( dtiki_t *pmdl, int animnum )
{
	skelAnimDataGameHeader_t *animData = SkeletorCacheGetData( pmdl->a->m_aliases[ animnum ] );
	return animData->bHasDelta;
}

/*
===============
TIKI_Anim_DeltaOverTime
===============
*/
void TIKI_Anim_DeltaOverTime( dtiki_t *pTiki, int iAnimnum, float fTime1, float fTime2, float *vDelta )
{
	int realAnimIndex;
	skelAnimDataGameHeader_t *animData;
	SkelVec3 absDelta;

	realAnimIndex = pTiki->a->m_aliases[ iAnimnum ];
	if( realAnimIndex != -1 )
	{
		animData = SkeletorCacheGetData( realAnimIndex );
		absDelta = animData->GetDeltaOverTime( fTime1, fTime2 );
		VectorScale( absDelta, pTiki->load_scale, vDelta );
	}
	else
	{
		TIKI_Error( "Skeletor GetDeltaOverTime: Couldn't find animation with index %i\n", iAnimnum );
		VectorClear( vDelta );
	}
}

/*
===============
TIKI_Anim_Flags
===============
*/
int TIKI_Anim_Flags( dtiki_t *pmdl, int animnum )
{
	dtikianimdef_t *panimdef = pmdl->a->animdefs[ animnum ];
	return panimdef->flags;
}

/*
===============
TIKI_Anim_FlagsSkel
===============
*/
int TIKI_Anim_FlagsSkel( dtiki_t *pmdl, int animnum )
{
	skelAnimDataGameHeader_t *animData;
	int flags;

	animData = SkeletorCacheGetData( pmdl->a->m_aliases[ animnum ] );
	flags = animData->flags;

	if( animData->bHasDelta )
		flags |= TAF_HASDELTA;

	if( animData->bHasMorph )
		flags |= TAF_HASMORPH;

	return flags;
}

/*
===============
TIKI_Anim_HasServerCommands
===============
*/
qboolean TIKI_Anim_HasServerCommands( dtiki_t *pmdl, int animnum )
{
	dtikianimdef_t *panimdef = pmdl->a->animdefs[ animnum ];
	return panimdef->num_server_cmds > 0;
}

/*
===============
TIKI_Anim_HasClientCommands
===============
*/
qboolean TIKI_Anim_HasClientCommands( dtiki_t *pmdl, int animnum )
{
	dtikianimdef_t *panimdef = pmdl->a->animdefs[ animnum ];
	return panimdef->num_client_cmds > 0;
}

/*
===============
TIKI_Anim_CrossblendTime
===============
*/
float TIKI_Anim_CrossblendTime( dtiki_t *pmdl, int animnum )
{
	dtikianimdef_t *panimdef = pmdl->a->animdefs[ animnum ];
	return panimdef->blendtime;
}
