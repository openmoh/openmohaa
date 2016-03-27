/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

//===========================================================================
// ANSI, Area Navigational System Interface
// AAS,  Area Awareness System
//===========================================================================

#include "qbsp.h"
#include "l_mem.h"
#include "../botlib/aasfile.h"			//aas_bbox_t
#include "aas_store.h"		//AAS_MAX_BBOXES
#include "aas_cfg.h"
#include "aas_map.h"			//AAS_CreateMapBrushes


#ifdef ME

#define NODESTACKSIZE		1024

int nodestack[NODESTACKSIZE];
int *nodestackptr;
int nodestacksize = 0;
int brushmodelnumbers[MAX_MAPFILE_BRUSHES];
int dbrushleafnums[MAX_MAPFILE_BRUSHES];
int dplanes2mapplanes[MAX_MAPFILE_PLANES];

#endif //ME

//====================================================================

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Q2_CreateMapTexinfo(void)
{

} //end of the function Q2_CreateMapTexinfo

/*
===========
Q2_BrushContents
===========
*/
int	Q2_BrushContents (mapbrush_t *b)
{
	
	return 0;
}

#ifdef ME

#define BBOX_NORMAL_EPSILON			0.0001

//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void MakeAreaPortalBrush(mapbrush_t *brush)
{

} //end of the function MakeAreaPortalBrush
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void DPlanes2MapPlanes(void)
{
	
} //end of the function DPlanes2MapPlanes
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void MarkVisibleBrushSides(mapbrush_t *brush)
{
	int n, i, planenum;
	
} //end of the function MarkVisibleBrushSides

#endif //ME

/*
=================
Q2_ParseBrush
=================
*/
void Q2_ParseBrush (script_t *script, entity_t *mapent)
{
	
}

/*
================
Q2_MoveBrushesToWorld

Takes all of the brushes from the current entity and
adds them to the world's brush list.

Used by func_group and func_areaportal
================
*/
void Q2_MoveBrushesToWorld (entity_t *mapent)
{
	int			newbrushes;
	int			worldbrushes;
	mapbrush_t	*temp;
	int			i;

	// this is pretty gross, because the brushes are expected to be
	// in linear order for each entity

	newbrushes = mapent->numbrushes;
	worldbrushes = entities[0].numbrushes;

	temp = GetMemory(newbrushes*sizeof(mapbrush_t));
	memcpy (temp, mapbrushes + mapent->firstbrush, newbrushes*sizeof(mapbrush_t));

#if	0		// let them keep their original brush numbers
	for (i=0 ; i<newbrushes ; i++)
		temp[i].entitynum = 0;
#endif

	// make space to move the brushes (overlapped copy)
	memmove (mapbrushes + worldbrushes + newbrushes,
		mapbrushes + worldbrushes,
		sizeof(mapbrush_t) * (nummapbrushes - worldbrushes - newbrushes) );

	// copy the new brushes down
	memcpy (mapbrushes + worldbrushes, temp, sizeof(mapbrush_t) * newbrushes);

	// fix up indexes
	entities[0].numbrushes += newbrushes;
	for (i=1 ; i<num_entities ; i++)
		entities[i].firstbrush += newbrushes;
	FreeMemory(temp);

	mapent->numbrushes = 0;
}

/*
================
Q2_ParseMapEntity
================
*/
qboolean	Q2_ParseMapEntity(script_t *script)
{
	entity_t	*mapent;
	epair_t *e;
	side_t *s;
	int i, j;
	int startbrush, startsides;
	vec_t newdist;
	mapbrush_t *b;
	token_t token;

	if (!PS_ReadToken(script, &token)) return false;

	if (strcmp(token.string, "{") )
		Error ("ParseEntity: { not found");
	
	if (num_entities == MAX_MAP_ENTITIES)
		Error ("num_entities == MAX_MAP_ENTITIES");

	startbrush = nummapbrushes;
	startsides = nummapbrushsides;

	mapent = &entities[num_entities];
	num_entities++;
	memset (mapent, 0, sizeof(*mapent));
	mapent->firstbrush = nummapbrushes;
	mapent->numbrushes = 0;
//	mapent->portalareas[0] = -1;
//	mapent->portalareas[1] = -1;

	do
	{
		if (!PS_ReadToken(script, &token))
		{
			Error("ParseEntity: EOF without closing brace");
		} //end if
		if (!strcmp(token.string, "}")) break;
		if (!strcmp(token.string, "{"))
		{
			Q2_ParseBrush(script, mapent);
		} //end if
		else
		{
			PS_UnreadLastToken(script);
			e = ParseEpair(script);
			e->next = mapent->epairs;
			mapent->epairs = e;
		} //end else
	} while(1);

	GetVectorForKey(mapent, "origin", mapent->origin);

	//
	// if there was an origin brush, offset all of the planes and texinfo
	//
	if (mapent->origin[0] || mapent->origin[1] || mapent->origin[2])
	{
		for (i=0 ; i<mapent->numbrushes ; i++)
		{
			b = &mapbrushes[mapent->firstbrush + i];
			for (j=0 ; j<b->numsides ; j++)
			{
				s = &b->original_sides[j];
				newdist = mapplanes[s->planenum].dist -
					DotProduct (mapplanes[s->planenum].normal, mapent->origin);
				s->planenum = FindFloatPlane (mapplanes[s->planenum].normal, newdist);
				s->texinfo = TexinfoForBrushTexture (&mapplanes[s->planenum],
					&side_brushtextures[s-brushsides], mapent->origin);
			}
			MakeBrushWindings (b);
		}
	}

	// group entities are just for editor convenience
	// toss all brushes into the world entity
	if (!strcmp ("func_group", ValueForKey (mapent, "classname")))
	{
		Q2_MoveBrushesToWorld (mapent);
		mapent->numbrushes = 0;
		return true;
	}

	// areaportal entities move their brushes, but don't eliminate
	// the entity
	if (!strcmp ("func_areaportal", ValueForKey (mapent, "classname")))
	{
		char	str[128];

		if (mapent->numbrushes != 1)
			Error ("Entity %i: func_areaportal can only be a single brush", num_entities-1);

		b = &mapbrushes[nummapbrushes-1];
		b->contents = CONTENTS_AREAPORTAL;
		c_areaportals++;
		mapent->areaportalnum = c_areaportals;
		// set the portal number as "style"
		sprintf (str, "%i", c_areaportals);
		SetKeyValue (mapent, "style", str);
		Q2_MoveBrushesToWorld (mapent);
		return true;
	}

	return true;
}

//===================================================================

/*
================
LoadMapFile
================
*/
void Q2_LoadMapFile(char *filename)
{		
	int i;
	script_t *script;

	Log_Print("-- Q2_LoadMapFile --\n");
#ifdef ME
	//loaded map type
	loadedmaptype = MAPTYPE_QUAKE2;
	//reset the map loading
	ResetMapLoading();
#endif //ME

	script = LoadScriptFile(filename);
	if (!script)
	{
		Log_Print("couldn't open %s\n", filename);
		return;
	} //end if
	//white spaces and escape characters inside a string are not allowed
	SetScriptFlags(script, SCFL_NOSTRINGWHITESPACES |
									SCFL_NOSTRINGESCAPECHARS |
									SCFL_PRIMITIVE);

	nummapbrushsides = 0;
	num_entities = 0;
	
	while (Q2_ParseMapEntity(script))
	{
	}

	ClearBounds (map_mins, map_maxs);
	for (i=0 ; i<entities[0].numbrushes ; i++)
	{
		if (mapbrushes[i].mins[0] > 4096)
			continue;	// no valid points
		AddPointToBounds (mapbrushes[i].mins, map_mins, map_maxs);
		AddPointToBounds (mapbrushes[i].maxs, map_mins, map_maxs);
	} //end for

	PrintMapInfo();

	//free the script
	FreeScript(script);
//	TestExpandBrushes ();
	//
	Q2_CreateMapTexinfo();
} //end of the function Q2_LoadMapFile

#ifdef ME		//Begin MAP loading from BSP file
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Q2_SetLeafBrushesModelNumbers(int leafnum, int modelnum)
{

} //end of the function Q2_SetLeafBrushesModelNumbers
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Q2_InitNodeStack(void)
{
	nodestackptr = nodestack;
	nodestacksize = 0;
} //end of the function Q2_InitNodeStack
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Q2_PushNodeStack(int num)
{
	*nodestackptr = num;
	nodestackptr++;
	nodestacksize++;
	//
	if (nodestackptr >= &nodestack[NODESTACKSIZE])
	{
		Error("Q2_PushNodeStack: stack overflow\n");
	} //end if
} //end of the function Q2_PushNodeStack
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
int Q2_PopNodeStack(void)
{
	//if the stack is empty
	if (nodestackptr <= nodestack) return -1;
	//decrease stack pointer
	nodestackptr--;
	nodestacksize--;
	//return the top value from the stack
	return *nodestackptr;
} //end of the function Q2_PopNodeStack
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Q2_SetBrushModelNumbers(entity_t *mapent)
{
	
} //end of the function Q2_SetBrushModelNumbers
//===========================================================================
//===========================================================================
void Q2_ParseBSPBrushes(entity_t *mapent)
{
	
} //end of the function Q2_ParseBSPBrushes
//===========================================================================
//===========================================================================
qboolean Q2_ParseBSPEntity(int entnum)
{
	
	return true;
} //end of the function Q2_ParseBSPEntity
//===========================================================================
//
// Parameter:				-
// Returns:					-
// Changes Globals:		-
//===========================================================================
void Q2_LoadMapFromBSP(char *filename, int offset, int length)
{

} //end of the function Q2_LoadMapFromBSP

void Q2_ResetMapLoading(void)
{

} //end of the function Q2_ResetMapLoading

//End MAP loading from BSP file
#endif //ME

//====================================================================

/*
================
TestExpandBrushes

Expands all the brush planes and saves a new map out
================
*/
void TestExpandBrushes (void)
{

} //end of the function TestExpandBrushes

