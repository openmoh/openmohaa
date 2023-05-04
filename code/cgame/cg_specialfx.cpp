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

#define GROUND_DISTANCE 8
#define WATER_NO_SPLASH_HEIGHT 16

static void CG_FootstepMain(trace_t* trace, int iRunning, int iEquipment)
{
	int contents;
	int surftype;
	int iEffectNum;
	float fVolume;
	vec3_t vPos;
	vec3_t midlegs;
	str sSoundName;

	iEffectNum = -1;

	VectorCopy(trace->endpos, vPos);
	sSoundName = "snd_step_";

	contents = CG_PointContents(trace->endpos, -1);
	if (contents & MASK_WATER)
	{
		// take our ground position and trace upwards 
		VectorCopy(trace->endpos, midlegs);
		midlegs[2] += WATER_NO_SPLASH_HEIGHT;
		contents = CG_PointContents(midlegs, -1);
		if (contents & MASK_WATER)
		{
			sSoundName += "wade";
		}
		else
		{
			sSoundName += "puddle";
			iEffectNum = 95;
		}
	}
	else
	{
		surftype = trace->surfaceFlags & MASK_SURF_TYPE;
		switch (surftype)
		{
		case SURF_FOLIAGE:
			sSoundName += "foliage";
			iEffectNum = 93;
			break;
		case SURF_SNOW:
			sSoundName += "snow";
			iEffectNum = 97;
			break;
		case SURF_CARPET:
			sSoundName += "carpet";
			iEffectNum = 90;
			break;
		case SURF_SAND:
			sSoundName += "sand";
			iEffectNum = 96;
			break;
		case SURF_PUDDLE:
			sSoundName += "puddle";
			iEffectNum = 95;
			break;
		case SURF_GLASS:
            sSoundName += "glass";
            iEffectNum = 90;
			break;
		case SURF_GRAVEL:
            sSoundName += "gravel";
            iEffectNum = 91;
			break;
		case SURF_MUD:
            sSoundName += "mud";
            iEffectNum = 94;
			break;
		case SURF_DIRT:
            sSoundName += "dirt";
            iEffectNum = 92;
			break;
		case SURF_GRILL:
            sSoundName += "grill";
            iEffectNum = 90;
			break;
		case SURF_GRASS:
            sSoundName += "grass";
            iEffectNum = 93;
			break;
		case SURF_ROCK:
            sSoundName += "stone";
            iEffectNum = 91;
			break;
		case SURF_PAPER:
            sSoundName += "paper";
            iEffectNum = 90;
			break;
		case SURF_WOOD:
            sSoundName += "wood";
            iEffectNum = 90;
			break;
		case SURF_METAL:
            sSoundName += "metal";
            iEffectNum = 90;
			break;
		default:
			sSoundName += "stone";
			iEffectNum = 91;
			break;
		}
	}

	if (cg_debugFootsteps->integer) {
		cgi.DPrintf("Footstep: %s  running = %i  effect = %i\n", sSoundName.c_str(), iRunning, surftype);
	}

	if (iRunning)
	{
		if (iRunning == -1) {
			fVolume = 0.5;
		}
		else {
			fVolume = 1.0;
		}
	}
	else {
		fVolume = 0.25;
	}

	if (!iRunning && cgs.gametype == GT_SINGLE_PLAYER) {
		return;
	}

	commandManager.PlaySound(
		sSoundName,
		vPos,
		-1,
		fVolume,
		-1,
		-1,
		1
	);

	if (iEquipment && random() < 0.3)
	{
        // also play equipment sound

        commandManager.PlaySound(
            "snd_step_equipment",
            vPos,
            -1,
            fVolume,
            -1,
            -1,
            1
        );
	}
}

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
		iTagNum = cgi.Tag_NumForName(pREnt->tiki, szTagName);
		if (iTagNum != -1)
		{
			oTag = cgi.TIKI_Orientation(pREnt, iTagNum);

			for (i = 0; i < 2; i++) {
                VectorMA(vStart, oTag.origin[i], pREnt->axis[i], vStart);
			}
		}
	}

	if (iRunning == -1)
    {
        AngleVectors(ent->lerpAngles, midlegs, NULL, NULL);
        VectorMA(vStart, -16, midlegs, vStart);
        VectorMA(vStart, 64, midlegs, vEnd);

        VectorSet(vMins, -2, -2, -8);
        VectorSet(vMaxs, 2, 2, 8);
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
	vec3_t vMins, vMaxs;
	trace_t trace;

	VectorSet(vMins, -4, -4, 0);
	VectorSet(vMaxs, 4, 4, 2);
	CG_Trace(&trace, vStart, vMins, vMaxs, vEnd, ENTITYNUM_NONE, MASK_PLAYERSOLID, qtrue, qtrue, "CG_MeleeImpact");

	if (trace.fraction != 1.0) {
		CG_FootstepMain(&trace, qtrue, qfalse);
	}
}

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
