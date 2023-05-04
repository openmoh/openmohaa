/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

// DESCRIPTION:
// Special Effects code

#include "cg_local.h"
#include "cg_commands.h"
#include "surfaceflags.h"
#include "cg_specialfx.h"

extern refEntity_t *current_entity;
extern dtiki_t* current_tiki;

static vec3_t g_vFootstepMins;
static vec3_t g_vFootstepMaxs;
static vec3_t g_vLadderstepMins;
static vec3_t g_vLadderstepMaxs;

ClientSpecialEffectsManager sfxManager;

Event EV_SFX_EffectDelay
(
	"effectdelay",
	EV_DEFAULT,
	"iivvvvv",
	"iEffect iCurrEmitter vPos vAngles vAxisA vAxisB vAxisC",
	"Resumes the execution of iEffect effect from its iCurrEmitter emitter."
);

CLASS_DECLARATION(Listener, ClientSpecialEffectsManager, NULL)
{
	{ &EV_SFX_EffectDelay, &ClientSpecialEffectsManager::ContinueEffectExecution },
	{ NULL, NULL }
};

specialeffectcommand_t::specialeffectcommand_t()
{
	emitter = NULL;
	fCommandTime = 0.0f;
	endfcn = NULL;
}

specialeffect_t::specialeffect_t()
{
	m_iCommandCount = 0;
}

ClientSpecialEffectsManager::ClientSpecialEffectsManager()
{
    m_bEffectsLoaded = 0;
    m_iNumPendingEvents = 0;
}

void ClientSpecialEffectsManager::LoadEffects()
{
    // FIXME: unimplemented
}

void CG_InitializeSpecialEffectsManager()
{
	sfxManager.LoadEffects();
}

void CG_AddPendingEffects()
{
	if (sfxManager.EffectsPending()) {
		sfxManager.ProcessPendingEvents();
	}
}

void ClientSpecialEffectsManager::ContinueEffectExecution(Event* ev)
{
	Vector norm;
	float axis[3][3];

	norm = ev->GetVector(5);
    VectorCopy(norm, axis[0]);

	norm = ev->GetVector(6);
    VectorCopy(norm, axis[1]);

	norm = ev->GetVector(7);
    VectorCopy(norm, axis[2]);

	ExecuteEffect(
		ev->GetInteger(1),
		ev->GetInteger(2),
		ev->GetVector(3),
		ev->GetVector(4),
		axis
	);
}

void ClientSpecialEffectsManager::ExecuteEffect(int iEffect, int iStartCommand, Vector vPos, Vector vAngles, float axis[3][3])
{
    // FIXME: unimplemented
}

void ClientSpecialEffectsManager::MakeEffect_Normal(int iEffect, Vector vPos, Vector vNormal)
{
    // FIXME: unimplemented
}

void ClientSpecialEffectsManager::MakeEffect_Angles(int iEffect, Vector vPos, Vector vAngles)
{
    float axis[3][3];

    AnglesToAxis((const float*)vAngles, axis);
    ClientSpecialEffectsManager::ExecuteEffect(iEffect, 0, vPos, vAngles, axis);
}

void ClientSpecialEffectsManager::MakeEffect_Axis(int iEffect, Vector vPos, float axis[3][3])
{
    Vector vAngles;

    MatrixToEulerAngles(axis, (float*)vAngles);
    ClientSpecialEffectsManager::ExecuteEffect(iEffect, 0, vPos, vAngles, axis);
}

/*
==============================================================

FOOTSTEP CODE

==============================================================
*/

static void CG_FootstepMain(trace_t* trace, int iRunning, int iEquipment)
{
	// FIXME: unimplemented
}

#define GROUND_DISTANCE 8
#define WATER_NO_SPLASH_HEIGHT 16
void CG_Footstep(char* szTagName, centity_t* ent, refEntity_t* pREnt, int iRunning, int iEquipment)
{
	int i;
	int iTagNum;
	vec3_t vStart, vEnd;
	vec3_t midlegs;
	vec3_t vMins, vMaxs;
	str sSoundName;
	trace_t trace;
	orientation_t oTag;

	// send a trace down from the player to the ground
	VectorCopy(ent->lerpOrigin, vStart);
	vStart[2] += GROUND_DISTANCE;

	if (szTagName)
	{
		iTagNum = cgi.Tag_NumForName(pREnt->tiki, szTagName);;
		if (iTagNum != -1)
		{
			oTag = cgi.TIKI_Orientation(pREnt, iTagNum);

			for (i = 0; i < 2; i++)
			{
                vStart[0] += pREnt->axis[i][0] * oTag.origin[0];
                vStart[0] += pREnt->axis[i][1] * oTag.origin[1];
                vStart[0] += pREnt->axis[i][2] * oTag.origin[2];
			}
		}
	}

	if (iRunning == -1)
    {
        AngleVectors(ent->lerpAngles, midlegs, NULL, NULL);

        VectorSet(vMins, -2, -2, -8);
        VectorSet(vMaxs, 2, 2, 8);

        vStart[0] += midlegs[0] * -16.0;
        vStart[1] += midlegs[1] * -16.0;
        vStart[2] += midlegs[2] * -16.0;

        vEnd[0] = vStart[0] + midlegs[0] * 64.0;
        vEnd[1] = vStart[1] + midlegs[1] * 64.0;
        vEnd[2] = vStart[2] + midlegs[2] * 64.0;
	}
	else
    {
        VectorSet(vMins, -4, -4, 0);
        VectorSet(vMaxs, 4, 4, 2);

		// add 16 units above feets
        vStart[2] += 16.0;
		VectorCopy(vStart, vEnd);
		vEnd[2] -= 64.0;
	}

	if (ent->currentState.eType == ET_PLAYER)
	{
		CG_Trace(
			&trace,
			vStart,
			vMins,
			vMaxs,
			vEnd,
			ent->currentState.number,
			MASK_PLAYERSOLID,
			qtrue,
			qtrue,
			"Player Footsteps"
		);
	}
	else
	{
		CG_Trace(
			&trace,
			vStart,
			vMins,
			vMaxs,
			vEnd,
			ent->currentState.number,
			MASK_MONSTERSOLID,
			qfalse,
			qfalse,
			"Monster Footsteps"
		);
	}

	if (trace.fraction == 1.0f)
    {
        if (cg_debugFootsteps->integer) {
            cgi.DPrintf("Footstep: missed floor\n");
        }

		return;
	}

	CG_FootstepMain(&trace, iRunning, iEquipment);
}

void CG_MeleeImpact(vec3_t vStart, vec3_t vEnd) {
	// FIXME: unimplemented
}

#if 0
void CG_Footstep(char* szTagName, centity_t* ent, refEntity_t* pREnt, int iRunning, int iEquipment)
{
	// FIXME: unimplemented

	vec3_t   end, midlegs;
	trace_t	trace;
	int      contents, surftype;
	spawnthing_t   effect;
	refEntity_t* old_entity;
	dtiki_t* old_tiki;
	refEntity_t    new_entity;
	dtiki_t* new_tiki;
	qhandle_t      hModel;

	// send a trace down from the player to the ground
	VectorCopy(ent->lerpOrigin, end);
	end[2] -= GROUND_DISTANCE;

	if (ent->currentState.eType == ET_PLAYER)
	{
		CG_Trace(&trace, ent->lerpOrigin, NULL, NULL, end, ent->currentState.number, MASK_PLAYERSOLID, qtrue, qtrue, "Player Footsteps");
	}
	else
	{
		CG_Trace(&trace, ent->lerpOrigin, NULL, NULL, end, ent->currentState.number, MASK_MONSTERSOLID, qfalse, qfalse, "Monster Footsteps");
	}

	if (trace.fraction == 1.0f)
	{
		return;
	}

	contents = CG_PointContents(trace.endpos, -1);
	if (contents & MASK_WATER)
	{
		// take our ground position and trace upwards 
		VectorCopy(trace.endpos, midlegs);
		midlegs[2] += WATER_NO_SPLASH_HEIGHT;
		contents = CG_PointContents(midlegs, -1);
		if (contents & MASK_WATER)
		{
			commandManager.PlaySound("footstep_wade", NULL, CHAN_AUTO, volume);
			if (cg_debugFootsteps->integer)
			{
				cgi.DPrintf("Footstep: wade    volume: %.2f\n", volume);
			}
		}
		else
		{
			commandManager.PlaySound("footstep_splash", NULL, CHAN_AUTO, volume);
			if (cg_debugFootsteps->integer)
			{
				cgi.DPrintf("Footstep: splash  volume: %.2f\n", volume);
			}
		}
	}
	else
	{
		surftype = trace.surfaceFlags & MASK_SURF_TYPE;
		switch (surftype)
		{
		case SURF_WOOD:
			commandManager.PlaySound("footstep_wood", NULL, CHAN_AUTO, volume);
			if (cg_debugFootsteps->integer)
			{
				cgi.DPrintf("Footstep: wood    volume: %.2f\n", volume);
			}
			break;
		case SURF_METAL:
			commandManager.PlaySound("footstep_metal", NULL, CHAN_AUTO, volume);
			if (cg_debugFootsteps->integer)
			{
				cgi.DPrintf("Footstep: metal   volume: %.2f\n", volume);
			}
			break;
		case SURF_ROCK:
			commandManager.PlaySound("footstep_rock", NULL, CHAN_AUTO, volume);
			if (cg_debugFootsteps->integer)
			{
				cgi.DPrintf("Footstep: rock    volume: %.2f\n", volume);
			}
			break;
		case SURF_DIRT:
			memset(&new_entity, 0, sizeof(refEntity_t));

			commandManager.PlaySound("footstep_dirt", NULL, CHAN_AUTO, volume);

			if (cg_debugFootsteps->integer)
			{
				cgi.DPrintf("Footstep: dirt    volume: %.2f\n", volume);
			}

			// Save the old stuff

			old_entity = current_entity;
			old_tiki = current_tiki;

			// Setup the new entity
			memset(&new_entity.shaderRGBA, 0xff, sizeof(new_entity.shaderRGBA));
			new_entity.origin[0] = trace.endpos[0];
			new_entity.origin[1] = trace.endpos[1];
			new_entity.origin[2] = trace.endpos[2] + 5;
			new_entity.scale = 1;

			current_entity = &new_entity;

			// Setup the new tiki

			hModel = cgi.R_RegisterModel("models/fx_dirtstep.tik");
			new_tiki = cgi.R_Model_GetHandle(hModel);
			current_tiki = new_tiki;

			// Process new entity

			CG_ProcessInitCommands(current_tiki, current_entity);

			// Put the old stuff back

			current_entity = old_entity;
			current_tiki = old_tiki;

			/* commandManager.InitializeSpawnthing( &effect );

			effect.SetModel( "models/fx_dirtstep.tik" );

			effect.cgd.origin[0]	= trace.endpos[0];
			effect.cgd.origin[1]	= trace.endpos[1];
			effect.cgd.origin[2]	= trace.endpos[2] + 5;

			effect.cgd.scale = 1;

			//effect.cgd.scaleRate = 1;

			effect.cgd.life = 2000;

			effect.cgd.flags |= T_FADE;

			effect.cgd.flags |= T_ANGLES;

			effect.randangles[ 0 ] = NOT_RANDOM;
			effect.cgd.angles[ 0 ] = 0;
			effect.randangles[ 1 ] = NOT_RANDOM;
			effect.cgd.angles[ 1 ] = 0;
			effect.randangles[ 2 ] = NOT_RANDOM;
			effect.cgd.angles[ 2 ] = 0;

			commandManager.SpawnTempModel( 1, &effect ); */

			break;
		case SURF_GRILL:
			commandManager.PlaySound("footstep_grill", NULL, CHAN_AUTO, volume);
			if (cg_debugFootsteps->integer)
			{
				cgi.DPrintf("Footstep: grill   volume: %.2f\n", volume);
			}
			break;
		}
	}
}
#endif

void CG_LandingSound(centity_t* ent, refEntity_t* pREnt, float volume, int iEquipment)
{
	// FIXME: unimplemented
}
/*
===============
CG_Splash

Draw a mark at the water surface
===============
*/
void CG_Splash( centity_t *cent ) {
	vec3_t		start, end, diff;
	trace_t		trace;
	int			contents;
	float			dist;
	float			time_required;

	spawnthing_t   m_ripple;

	if ( !cg_shadows->integer ) {
		return;
	}

   VectorSubtract( cent->currentState.origin, cent->nextState.origin, diff );
   diff[ 2 ] = 0;
	dist = VectorLength( diff );

	// See if enough time has passed to add another ripple

	if ( dist >= 1 )
		time_required = 100 - dist;
	else
		time_required = 200;

	if ( time_required < 10 )
		time_required = 10;

	if ( cent->splash_last_spawn_time + time_required > cg.time )
		return;

	// Save the current time

	cent->splash_last_spawn_time = cg.time;

	// Make sure the entity is moving
	if ( dist < 1 )
		{
		if ( cent->splash_still_count >= 0 )
			{
			cent->splash_still_count++;

			if ( cent->splash_still_count > 2 )
				cent->splash_still_count = 0;
			else
				return;
			}
		else
			return;
		}


	VectorCopy( cent->lerpOrigin, end );

	// if the feet aren't in liquid, don't make a mark
	// this won't handle moving water brushes, but they wouldn't draw right anyway...
	contents = cgi.CM_PointContents( end, 0 );
	if ( !( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, start );
	start[2] += 88;

	// if the head isn't out of liquid, don't make a mark
	contents = cgi.CM_PointContents( start, 0 );
	if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}

	// trace down to find the surface
	cgi.CM_BoxTrace( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ), qfalse );

	if ( trace.fraction == 1.0 ) {
		return;
	}

	// FIXME
	// TODO: Make effect
}

qboolean ClientSpecialEffectsManager::EffectsPending()
{
	return sfxManager.m_iNumPendingEvents > 0;
}

specialeffect_t* ClientSpecialEffectsManager::GetTestEffectPointer()
{
    return &m_effects[SPECIAL_EFFECT_TEST];
}
