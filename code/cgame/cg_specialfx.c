/*
===========================================================================
Copyright (C) 2011 su44

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
// cg_specialfx.c - MoHAA special effects

#include "cg_local.h"

// MoHAA footstep sounds (from ubersound.scr) :
// snd_step_paper, snd_step_glass, snd_step_wood, snd_step_metal, 
// snd_step_grill, snd_step_stone, snd_step_dirt, snd_step_grass,
// snd_step_mud, snd_step_puddle, snd_step_gravel, snd_step_sand,
// snd_step_foliage, snd_step_snow, snd_step_carpet
// BODY MOVEMENT
// snd_step_equipment - (equipment; for walking)
// snd_step_wade - (wade; for when wading through deeper water)

static void CG_FootstepMain(trace_t *trace, int iRunning, int iEquipment) {
    char sSoundName[MAX_QPATH] = "snd_step_";

    int contents;
	int surfaceFlags;
	vec3_t pos;

	ubersound_t *snd = 0;

	if ( !iRunning )
		return;

	// su44: it seems that trace_t::surfaceFlags
	// are not set if trace_t::allsolid == qtrue
	if( trace->allsolid == qtrue )
		return;

	if ( iEquipment ) {
		snd = CG_GetUbersound( "snd_step_equipment" );
		if (snd)
			cgi.S_StartSound( trace->endpos, trace->entityNum, CHAN_ITEM, snd->sfxHandle );
	}

	surfaceFlags = trace->surfaceFlags;
	contents = CG_PointContents(trace->endpos,-1);
	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) ) {
		// wombat: we test if the water is deep or shallow
		VectorCopy( trace->endpos, pos );
		pos[2] += 16.0f;
		contents = CG_PointContents(pos,-1);
		if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
			snd = CG_GetUbersound( "snd_step_wade" );
		else
			snd = CG_GetUbersound( "snd_step_puddle" );
	} else {
		if ( surfaceFlags & SURF_PAPER ) {
			snd = CG_GetUbersound( "snd_step_paper" );
		}
		else if ( surfaceFlags & SURF_WOOD ) {
			snd = CG_GetUbersound( "snd_step_wood" );
		}
		else if ( surfaceFlags & SURF_METAL ) {
			snd = CG_GetUbersound( "snd_step_metal" );
		}
		else if ( surfaceFlags & SURF_ROCK ) {
			snd = CG_GetUbersound( "snd_step_stone" );
		}
		else if ( surfaceFlags & SURF_DIRT ) {
			snd = CG_GetUbersound( "snd_step_dirt" );
		}
		else if ( surfaceFlags & SURF_GRILL ) {
			snd = CG_GetUbersound( "snd_step_grill" );
		}
		else if ( surfaceFlags & SURF_GRASS ) {
			snd = CG_GetUbersound( "snd_step_grass" );
		}
		else if ( surfaceFlags & SURF_MUD ) {
			snd = CG_GetUbersound( "snd_step_mud" );
		}
		else if ( surfaceFlags & SURF_PUDDLE ) {
			snd = CG_GetUbersound( "snd_step_puddle" );
		}
		else if ( surfaceFlags & SURF_GLASS ) {
			snd = CG_GetUbersound( "snd_step_glass" );
		}
		else if ( surfaceFlags & SURF_GRAVEL ) {
			snd = CG_GetUbersound( "snd_step_gravel" );
		}
		else if ( surfaceFlags & SURF_SAND ) {
			snd = CG_GetUbersound( "snd_step_sand" );
		}
		else if ( surfaceFlags & SURF_FOLIAGE ) {
			snd = CG_GetUbersound( "snd_step_foliage" );
		}
		else if ( surfaceFlags & SURF_SNOW ) {
			snd = CG_GetUbersound( "snd_step_snow" );
		}
		else if ( surfaceFlags & SURF_CARPET ) {
			snd = CG_GetUbersound( "snd_step_carpet" );
		}
	}
	if ( !snd ) {
		if(trace->surfaceFlags) {
			CG_Printf( "CG_FootstepMain: could not load sound for surface %i\n", trace->surfaceFlags );
		}
		return;
	}
	cgi.S_StartSound( trace->endpos, trace->entityNum, CHAN_BODY, snd->sfxHandle );
}

void CG_MeleeImpact(float *vStart, float *vEnd) {
	trace_t trace;
	float vMins[3] = { -4.0, -4.0, 0 };
	float vMaxs[3] = { 4.0, 4.0, 2.0 };
	CG_Trace(&trace,vStart,vMins,vMaxs,vEnd,1023,MASK_SHOT);
	if(trace.fraction != 1.f) {
		CG_FootstepMain(&trace,1,0);
	}
}


void CG_Footstep(char *szTagName, centity_t *ent, /*refEntity_t *pREnt,*/
	int iRunning, int iEquipment)
{
	int mask;
	int cylinder;
	int cliptoentities;
	trace_t trace;
	vec3_t mins, maxs;
	vec3_t vStart, vEnd, forward;


//	CG_Printf("CG_Footstep: %s\n",szTagName);

	vStart[0] = ent->lerpOrigin[0];
	vStart[1] = ent->lerpOrigin[1];
	vStart[2] = ent->lerpOrigin[2] + 8.0;
	if ( szTagName ) {
		tiki_t *tiki;
		int boneName;
		int tagIndex;
		vec3_t a;

		tiki = cgs.gameTIKIs[ent->currentState.modelindex];

		if(tiki == 0) {
			CG_Printf("CG_Footstep: tiki is NULL\n");
			return;
		}	

		// find tag in tiki
		boneName = cgi.TIKI_GetBoneNameIndex(szTagName); 
		for(tagIndex = 0; tagIndex < tiki->numBones; tagIndex++) {
			if(tiki->boneNames[tagIndex] == boneName) {
				break;
			}
		}
		if(tagIndex == tiki->numBones) {
			CG_Printf("CG_Footstep: Cant find bone %s in tiki %s f\n",szTagName,tiki->name);
			return;
		}
		if(ent->bones == 0) {
			CG_Printf("CG_Footstep: entity has null bones ptr\n");
			return;
		}	

		CG_CentBoneIndexLocal2World(tagIndex, ent, vStart, a);
		vStart[2] += 8;
	}
	if ( iRunning == -1 ) {
		AngleVectors(ent->lerpAngles, forward, 0, 0);
		vStart[0] = forward[0] * -16.0 + vStart[0];
		vStart[1] = forward[1] * -16.0 + vStart[1];
		vStart[2] = forward[2] * -16.0 + vStart[2];
		vEnd[0] = forward[0] * 64.0 + vStart[0];
		vEnd[1] = forward[1] * 64.0 + vStart[1];
		vEnd[2] = forward[2] * 64.0 + vStart[2];
		mins[0] = -2.0;
		mins[1] = -2.0;
		mins[2] = -8.0;
		maxs[0] = 2.0;
		maxs[1] = 2.0;
		maxs[2] = 8.0;
	} else {
		vEnd[0] = vStart[0];
		vEnd[1] = vStart[1];

		vStart[2] = vStart[2] + 16.0;
		vEnd[2] = vStart[2] - 64.0;

		mins[0] = -4.0;
		mins[1] = -4.0;
		mins[2] = 0.0;

		maxs[0] = 4.0;
		maxs[1] = 4.0;
		maxs[2] = 2.0;
	}
	if ( ent->currentState.eType == ET_PLAYER )	{
		cliptoentities = 1;
		cylinder = 1;
		mask = MASK_TREADMARK;
	} else {
		cliptoentities = 0;
		cylinder = 0;
		mask = MASK_FOOTSTEP;
	}
	CG_Trace(&trace, vStart, mins, maxs, vEnd,
		ent->currentState.number, mask/*, cylinder, cliptoentities*/);
	if ( trace.fraction != 1.0 ) {
		int equip;
		if (rand() > RAND_MAX*0.70)
			equip = qtrue; // play only in 30% of cases
		else equip = qfalse;
		CG_FootstepMain(&trace, iRunning, equip);
	} else {
		//if ( cg_debugFootsteps.integer )
		//	CG_Printf("Footstep: missed floor\n");
	}
}