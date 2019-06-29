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

// simpleactor.cpp: Base class for character AI

#include "actor.h"
#include "bg_local.h"

CLASS_DECLARATION( Sentient, SimpleActor, NULL )
{
	{ NULL, NULL }
};

#define OVERLOADED_ERROR() assert( !"overloaded version should always get called" )

SimpleActor::SimpleActor()
{
	if( LoadingSavegame )
		return;

	m_AnimMotionHigh			= true;
	m_DesiredGunDir				= vec_zero;
	m_eEmotionMode				= EMOTION_NEUTRAL;
	m_eNextAnimMode				= -1;
	m_fPathGoalTime				= 0.0f;
	m_csPathGoalEndAnimScript	= STRING_EMPTY;

	m_AnimActionHigh			= true;
	m_AnimDialogHigh			= true;
	m_fAimLimit_up				= 60.0f;
	m_fAimLimit_down			= -60.0f;

	m_csNextAnimString			= STRING_NULL;
	m_bNextForceStart			= false;
	m_fCrossblendTime			= 0.5f;
	m_csCurrentPosition			= STRING_STAND;
	m_bStartPathGoalEndAnim		= false;

	for( int i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		m_weightType[ i ] = 0.0f;
	}

	m_bMotionAnimSet			= false;
	m_ChangeActionAnimIndex		= -1;
	m_ChangeActionAnimIndex		= -1;
	m_ChangeSayAnimIndex		= -1;
	m_iMotionSlot				= -1;
	m_iActionSlot				= -1;
	m_bActionAnimSet			= false;
	m_bSayAnimSet				= false;
	m_iVoiceTime				= 0;
	m_bAimAnimSet				= false;
	m_iSaySlot					= -1;
	m_bLevelMotionAnim			= false;
	m_bLevelActionAnim			= false;
	m_bLevelSayAnim				= 0;
	m_bNextLevelSayAnim			= 0;
	m_DesiredYaw				= 0.0f;
	m_YawAchieved				= true;
	m_bPathErrorTime			= -10000000.0f;

	m_PainHandler.TrySetScript( STRING_ANIM_PAIN_SCR );
	m_DeathHandler.TrySetScript( STRING_ANIM_KILLED_SCR );
	m_AttackHandler.TrySetScript( STRING_ANIM_ATTACK_SCR );
	m_SniperHandler.TrySetScript( STRING_ANIM_SNIPER_SCR );

	m_bHasDesiredLookDest	= false;
	m_bUpdateAnimDoneFlags	= false;

	m_NearestNode			= NULL;
	m_fCrouchWeight			= 0.0f;

	m_csMood				= STRING_BORED;
	m_csIdleMood			= STRING_BORED;

	m_groundPlane			= qfalse;
	m_walking				= qfalse;
	m_groundPlaneNormal		= vec_zero;
	m_maxspeed				= 1000000.0f;
}

void SimpleActor::Archive
	(
	Archiver& arc
	)

{
	Sentient::Archive( arc );

	arc.ArchiveInteger( &m_eAnimMode );
	m_Anim.Archive( arc );

	arc.ArchiveBool( &m_bHasDesiredLookDest );
	arc.ArchiveBool( &m_bHasDesiredLookAngles );
	arc.ArchiveVector( &m_vDesiredLookDest );
	arc.ArchiveVec3( m_DesiredLookAngles );
	arc.ArchiveVec3( m_DesiredGunDir );

	m_Path.Archive( arc );
	arc.ArchiveVec3( m_Dest );
	arc.ArchiveVec3( m_NoClipDest );

	arc.ArchiveFloat( &path_failed_time );
	arc.ArchiveFloat( &m_fPathGoalTime );
	arc.ArchiveBool( &m_bStartPathGoalEndAnim );
	Director.ArchiveString( arc, m_csPathGoalEndAnimScript );

	arc.ArchiveInteger( &m_eNextAnimMode );
	Director.ArchiveString( arc, m_csNextAnimString );
	m_NextAnimLabel.Archive( arc );
	arc.ArchiveBool( &m_bNextForceStart );

	arc.ArchiveBoolean( &m_walking );
	arc.ArchiveBoolean( &m_groundPlane );
	arc.ArchiveVec3( m_groundPlaneNormal );

	arc.ArchiveVector( &watch_offset );
	arc.ArchiveBool( &m_bThink );
	arc.ArchiveInteger( &m_PainTime );

	arc.ArchiveBool( &m_bAimAnimSet );
	arc.ArchiveBool( &m_bActionAnimSet );

	Director.ArchiveString( arc, m_csMood );
	Director.ArchiveString( arc, m_csIdleMood );

	ArchiveEnum(m_eEmotionMode, eEmotionMode);

	arc.ArchiveFloat( &m_fAimLimit_up );
	arc.ArchiveFloat( &m_fAimLimit_down );

	for( int i = MAX_FRAMEINFOS - 1; i >= 0; i-- )
	{
		arc.ArchiveUnsigned( &m_weightType[ i ] );
	}

	for( int i = MAX_FRAMEINFOS - 1; i >= 0; i-- )
	{
		arc.ArchiveFloat( &m_weightBase[ i ] );
	}

	for( int i = MAX_FRAMEINFOS - 1; i >= 0; i-- )
	{
		arc.ArchiveFloat( &m_weightCrossBlend[ i ] );
	}

	arc.ArchiveBool( &m_AnimMotionHigh );
	arc.ArchiveBool( &m_AnimActionHigh );
	arc.ArchiveBool( &m_AnimDialogHigh );

	arc.ArchiveVec2( obstacle_vel );

	Director.ArchiveString( arc, m_csCurrentPosition );

	arc.ArchiveBool( &m_bMotionAnimSet );
	arc.ArchiveBool( &m_bDoAI );

	arc.ArchiveFloat( &m_fCrossblendTime );

	arc.ArchiveSafePointer( &m_pAnimThread );

	arc.ArchiveBool( &m_YawAchieved );
	arc.ArchiveFloat( &m_DesiredYaw );

	arc.ArchiveInteger( &m_iVoiceTime );
	arc.ArchiveBool( &m_bSayAnimSet );

	arc.ArchiveInteger( &hit_obstacle_time );

	Director.ArchiveString( arc, m_csAnimName );

	arc.ArchiveInteger( &m_bPathErrorTime );
	arc.ArchiveInteger( &m_iMotionSlot );
	arc.ArchiveInteger( &m_iActionSlot );
	arc.ArchiveInteger( &m_iSaySlot );

	arc.ArchiveBool( &m_bLevelMotionAnim );
	arc.ArchiveBool( &m_bLevelActionAnim );
	arc.ArchiveByte( &m_bLevelSayAnim );
	arc.ArchiveByte( &m_bNextLevelSayAnim );

	Director.ArchiveString( arc, m_csSayAnim );
	Director.ArchiveString( arc, m_csUpperAnim );

	m_PainHandler.Archive( arc );
	m_DeathHandler.Archive( arc );
	m_AttackHandler.Archive( arc );
	m_SniperHandler.Archive( arc );

	arc.ArchiveObjectPointer( ( Class ** )&m_NearestNode );
	arc.ArchiveVector( &m_vNearestNodePos );

	arc.ArchiveFloat( &m_fCrouchWeight );
	arc.ArchiveFloat( &m_maxspeed );
}

void SimpleActor::SetMoveInfo
	(
	mmove_t *
	)

{
	OVERLOADED_ERROR();
}

void SimpleActor::GetMoveInfo
(
mmove_t *
)

{
	OVERLOADED_ERROR();
}

void SimpleActor::StopAnimating
	(
	int slot
	)

{
	int index = 0;

	m_weightType[ slot ] = 0;
	DoExitCommands( slot );

	if( edict->s.frameInfo[ slot ].index || gi.TIKI_NumAnims( edict->tiki ) <= 1 )
	{
		edict->s.frameInfo[ slot ].index = 0;
	}
	else
	{
		edict->s.frameInfo[ slot ].index = 1;
	}

	animFlags[ slot ] = ANIM_LOOP | ANIM_NODELTA | ANIM_NOEXIT | ANIM_PAUSED;
	edict->s.frameInfo[ slot ].weight = 0;
	animtimes[ slot ] = 0;
	animFlags[ slot ] = ( animFlags[ slot ] | ANIM_NODELTA ) & ~ANIM_FINISHED;
}

void SimpleActor::AnimFinished
	(
	int slot
	)

{
	assert( !DumpCallTrace("") );
}

bool SimpleActor::CanTarget
	(
	void
	)

{
	OVERLOADED_ERROR();
	return false;
}

bool SimpleActor::IsImmortal
	(
	void
	)

{
	OVERLOADED_ERROR();
	return false;
}

bool SimpleActor::DoesTheoreticPathExist
	(
	Vector vDestPos,
	float fMaxPath
	)

{
	return m_Path.DoesTheoreticPathExist( origin, vDestPos, this, fMaxPath, NULL, 0 );
}

void SimpleActor::SetPath
	(
	Vector vDestPos,
	const char *description,
	int iMaxDirtyTime,
	float *vLeashHome,
	float fLeashDistSquared
	)

{
	if (!PathExists()
		|| ( ( level.inttime >= m_Path.Time() + iMaxDirtyTime
			 || m_Path.Complete(origin) )
				&& m_Path.LastNode()->point != vDestPos)
		)
	{
		m_Path.FindPath(origin,
			vDestPos,
			this,
			0.0,
			vLeashHome,
			fLeashDistSquared);

		if (!m_Path.LastNode())
		{
			if (g_patherror->integer)
			{
				if (description)
				{
					int thinkState = ((Actor *)this)->m_ThinkState;
					if (g_patherror->integer == 1 || g_patherror->integer == 2 && (thinkState == THINKSTATE_IDLE || thinkState == THINKSTATE_CURIOUS))
					{
						if (m_bPathErrorTime + 5000 < level.inttime)
						{
							m_bPathErrorTime = level.inttime;
							Com_Printf(
								"^~^~^ Path not found in '%s' for (entnum %d, radnum %d, targetname '%s') from (%f %f %f) to (%f %f %f)\n",
								description,
								entnum,
								radnum,
								targetname.c_str(),
								origin.x,
								origin.y,
								origin.z,
								vDestPos.x,
								vDestPos.y,
								vDestPos.z
								);
							Com_Printf("Reason: %s\n", PathSearch::last_error);
						}
					}
				}
			}
		}
	}
}

void SimpleActor::SetPath
	(
	SimpleEntity *pDestNode,
	char *description,
	int iMaxDirtyTime
	)

{
	if (pDestNode)
	{
		SetPath(
			pDestNode->origin,
			description,
			iMaxDirtyTime,
			NULL,
			0.0);
	}
	else
	{
		if (m_bPathErrorTime + 5000 < level.inttime)
		{
			m_bPathErrorTime = level.inttime;
			Com_Printf(
				"^~^~^ No destination node specified for '%s' at (%f %f %f)\n",
				targetname.c_str(),
				origin.x,
				origin.y,
				origin.z);
		}
		ClearPath();
	}
}

void SimpleActor::SetPathWithinDistance
	(
	Vector vDestPos,
	char *description,
	float fMaxPath,
	int iMaxDirtyTime
	)

{
	if (!PathExists()
		|| ((level.inttime >= m_Path.Time() + iMaxDirtyTime
			|| m_Path.Complete(origin)) && m_Path.LastNode()->point != vDestPos)
		)
	{
		m_Path.FindPath(origin,
			vDestPos,
			this,
			fMaxPath,
			NULL,
			0);

		if (!m_Path.LastNode())
		{
			if (g_patherror->integer)
			{
				if (description)
				{
					int thinkState = ((Actor *)this)->m_ThinkState;
					if (g_patherror->integer == 1 || g_patherror->integer == 2 && (thinkState == THINKSTATE_IDLE || thinkState == THINKSTATE_CURIOUS))
					{
						if (m_bPathErrorTime + 5000 < level.inttime)
						{
							m_bPathErrorTime = level.inttime;
							Com_Printf(
								"^~^~^ Path not found in '%s' for '%s' from (%f %f %f) to (%f %f %f)\n",
								description,
								targetname.c_str(),
								origin.x,
								origin.y,
								origin.z,
								vDestPos.x,
								vDestPos.y,
								vDestPos.z);
							Com_Printf("Reason: %s\n", PathSearch::last_error);
						}
					}
				}
			}
		}
	}
}

void SimpleActor::FindPathAway
	(
	vec_t *vAwayFrom,
	vec_t *vDirPreferred,
	float fMinSafeDist
	)

{
	m_Path.FindPathAway(origin, vAwayFrom, vDirPreferred, this, fMinSafeDist, NULL, 0);
	
	ShortenPathToAvoidSquadMates();
}

void SimpleActor::ClearPath
	(
	void
	)

{
	m_Path.Clear();
}

bool SimpleActor::PathComplete
	(
	void
	) const

{
	if( level.time >= m_fPathGoalTime )
	{
		if( m_Path.Complete( origin ) )
			return true;
	}

	return false;
}

bool SimpleActor::PathExists
	(
	void
	) const

{
	return m_Path.CurrentNode() != NULL;
}

bool SimpleActor::PathIsValid
	(
	void
	) const

{
	return true;
}

bool SimpleActor::PathAvoidsSquadMates
	(
	void
	) const

{

	Entity* player;
	float fDelta;
	float fDistSoFar;
	float fDistCap;
	float vDelta2[2];
	float vMins[3];
	float vMaxs[3];
	float vPos[3];
	Sentient *pOther;
	Sentient *pBuddy[256];
	int iNumBuddies;
	int i;
	float fRatio;

	if (ai_pathchecktime->value <= 0.0)
		return true;
	player = G_GetEntity(0);
	if (!player)
	{
		return true;
	}

	player = G_GetEntity(0);
	player = G_GetEntity(0);
	
	fDelta = (player->origin - origin).lengthXYSquared();
	if (fDelta > Square(ai_pathcheckdist->value))
	{
		return true;
	}

	fDistSoFar = 0;
	for (auto pNode = CurrentPathNode(); pNode >= LastPathNode(); pNode--)
	{
		if (fDistSoFar <= 0)
		{
			break;
		}
		//fDistCap = (ai_pathchecktime->value * 250.0) + 0.001 - fDistSoFar;

		//if (pNode->point[2] > fDistCap)
		{
		}
	}
	// FIXME: stub
	STUB();
	return false;
}

void SimpleActor::ShortenPathToAvoidSquadMates
	(
	void
	)

{
	if (PathExists() && !PathComplete())
	{
		Vector vBuddyPos;
		Vector vDelta;
		do 
		{
			Actor *pSquadMate = (Actor *)m_pNextSquadMate.Pointer();
			if (pSquadMate == this)
			{
				break;
			}

			while (true)
			{
				vBuddyPos = pSquadMate->origin;
				if (pSquadMate->IsSubclassOfActor() && pSquadMate->PathExists())
				{
					vBuddyPos = pSquadMate->LastPathNode()->point;
				}
				vDelta.x = LastPathNode()->point[0] - vBuddyPos.x;
				if (vDelta.x >= -15.0 && vDelta.x <= 15.0)
				{
					vDelta.y = LastPathNode()->point[1] - vBuddyPos.y;
					vDelta.z = LastPathNode()->point[2] - vBuddyPos.z;
					
					if (vDelta.y >= -15.0 && vDelta.y <= 15.0 && vDelta.z >= 0.0 && vDelta.z <= 94.0)
						break;
				}
				pSquadMate = (Actor *)pSquadMate->m_pNextSquadMate.Pointer();
				if (pSquadMate == this)
				{
					return;
				}
			}
			m_Path.Shorten(45.0);

		} while (PathExists());
	}
}

PathInfo *SimpleActor::CurrentPathNode
	(
	void
	) const

{
	return m_Path.CurrentNode();
}

PathInfo *SimpleActor::LastPathNode
	(
	void
	) const

{
	return m_Path.LastNode();
}

float SimpleActor::PathDist
	(
	void
	) const

{
	return m_Path.TotalDist();
}

bool SimpleActor::PathHasCompleteLookahead
	(
	void
	) const

{
	return m_Path.HasCompleteLookahead();
}

Vector SimpleActor::PathGoal
	(
	void
	) const

{
	return m_Path.CurrentPathGoal();
}

float *SimpleActor::PathDelta
	(
	void
	) const

{
	return m_Path.CurrentDelta();
}

bool SimpleActor::PathGoalSlowdownStarted
	(
	void
	) const

{
	return m_fPathGoalTime >= level.time;
}

void SimpleActor::SetDest
	(
	vec3_t dest
	)

{
	VectorCopy(m_Dest, dest);
}

void SimpleActor::StopTurning
	(
	void
	)

{
	//fixme: this is an inline function.
	m_YawAchieved = true;
}

void SimpleActor::SetDesiredYaw
	(
	float yaw
	)

{
	m_YawAchieved = false;
	m_DesiredYaw = yaw;
}

void SimpleActor::SetDesiredYawDir
	(
	vec3_t vec
	)

{
	m_YawAchieved = false;
	m_DesiredYaw = vectoyaw(vec);
}

void SimpleActor::SetDesiredYawDest
	(
	vec3_t vec
	)

{
	m_bHasDesiredLookDest = true;
	VectorCopy(vec, m_vDesiredLookDest);
}

void SimpleActor::UpdateEmotion
	(
	void
	)

{
	int anim;
	if (IsDead())
	{
		Anim_Emotion(EMOTION_DEAD);
	}

	anim = GetEmotionAnim();

	if (anim == -1)
	{
		Com_Printf(
			"Failed to set emotion for (entnum %d, radnum %d, targetname '%s'\n",
			entnum,
			radnum,
			targetname.c_str());
	}
	else
	{
		m_bSayAnimSet = true;
		StartSayAnimSlot(anim);
	}
}

int SimpleActor::GetEmotionAnim
	(
	void
	)

{
	char *emotionanim = NULL;
	int anim;

	if (m_eEmotionMode)
	{
		switch (m_eEmotionMode)
		{
		case EMOTION_NEUTRAL:
		case EMOTION_AIMING:
			emotionanim = "facial_idle_neutral";
			break;
		case EMOTION_WORRY:
			emotionanim = "facial_idle_worry";
			break;
		case EMOTION_PANIC:
			emotionanim = "facial_idle_panic";
			break;
		case EMOTION_FEAR:
			emotionanim = "facial_idle_fear";
			break;
		case EMOTION_DISGUST:
			emotionanim = "facial_idle_disgust";
			break;
		case EMOTION_ANGER:
			emotionanim = "facial_idle_anger";
			break;
		case EMOTION_DETERMINED:
		case EMOTION_CURIOUS:
			emotionanim = "facial_idle_determined";
			break;
		case EMOTION_DEAD:
			emotionanim = "facial_idle_dead";
			break;
		default:

			char assertStr[16317] = { 0 };
			strcpy(assertStr, "\"Unknown value for m_EmotionMode in SimpleActor::GetEmotionAnim\"\n\tMessage: ");
			Q_strcat(assertStr, sizeof(assertStr), DumpCallTrace(""));
			assert(!assertStr);
			return -1;
			break;
		}
	}
	else
	{
		if (m_csMood == STRING_NERVOUS)
		{
			emotionanim = "facial_idle_determined";
		}
		else if (m_csMood <= STRING_NERVOUS)
		{
			if (m_csMood != STRING_BORED)
			{
				assert(!"Unknown value for m_csMood");
				return -1;
			}
			else
			{
				emotionanim = "facial_idle_neutral";
			}

		}
		else if (m_csMood == STRING_CURIOUS)
		{
			emotionanim = "facial_idle_determined";
		}
		else if (m_csMood != STRING_ALERT)
		{
			assert(!"Unknown value for m_csMood");
			return -1;
		}
		else
		{

		}
	}

	if (emotionanim == NULL)
	{
		emotionanim = "facial_idle_anger";
		//assert(!"Unexpected behaviour in SimpleActor::GetEmotionAnim");
	}

	anim = gi.Anim_NumForName(edict->tiki, emotionanim);
	if (anim == -1)
		Com_Printf(
			"^~^~^ SimpleActor::GetEmotionAnim: unknown animation '%s' in '%s'\n",
			emotionanim,
			edict->tiki->a->name);
	return anim;
}

int SimpleActor::GetMotionSlot
	(
	int slot
	)

{
	if( m_AnimMotionHigh )
		return slot + 3;
	else
		return slot;
}

int SimpleActor::GetActionSlot
	(
	int slot
	)

{
	if( m_AnimActionHigh )
		return slot + 9;
	else
		return slot + 6;
}

int SimpleActor::GetSaySlot
	(
	void
	)

{
	return m_AnimDialogHigh ? 13 : 12;
}

void SimpleActor::StartCrossBlendAnimSlot
	(
	int slot
	)

{
	if (m_weightType[slot] == 1)
	{
		m_weightType[slot] = 4;
	}
	else
	{
		if (m_weightType[slot] < 1)
			return;
		if (m_weightType[slot] == 6)
			m_weightType[slot] = 5;
		else
			m_weightType[slot] = 3;
	}
	m_weightCrossBlend[slot] = 1.0;
	m_weightBase[slot] = edict->s.frameInfo[slot].weight;
}

void SimpleActor::StartMotionAnimSlot
	(
	int slot,
	int anim,
	float weight
	)

{
	int iSlot = GetMotionSlot(slot);

	m_weightCrossBlend[iSlot] = 0.0;
	m_weightType[iSlot] = 1;
	m_weightBase[iSlot] = weight;

	NewAnim(anim, iSlot, 1.0);
	animFlags[iSlot] |= ANIM_NOACTION;

	SetTime(iSlot, 0.0);

	UpdateNormalAnimSlot(iSlot);
}

void SimpleActor::StartAimMotionAnimSlot
	(
	int slot,
	int anim
	)

{
	int iSlot = GetMotionSlot(slot);

	m_weightCrossBlend[iSlot] = 0.0;
	m_weightType[iSlot] = 1;

	NewAnim(anim, iSlot, 1.0);
	animFlags[iSlot] |= ANIM_NOACTION;

	SetTime(iSlot, 0.0);

	UpdateNormalAnimSlot(iSlot);
}

void SimpleActor::StartActionAnimSlot
	(
	int anim
	)

{
	int iSlot = GetActionSlot(0);

	m_weightCrossBlend[iSlot] = 0.0;
	m_weightType[iSlot] = 2;
	m_weightBase[iSlot] = 1.0;

	NewAnim(anim, iSlot, 1.0);

	SetTime(iSlot, 0.0);

	UpdateNormalAnimSlot(iSlot);
}

void SimpleActor::StartSayAnimSlot
	(
	int anim
	)

{
	int iSlot = GetSaySlot();

	m_weightCrossBlend[iSlot] = 0.0;
	m_weightType[iSlot] = 6;
	m_weightBase[iSlot] = 1.0;

	NewAnim(anim, iSlot, 1.0);
	animFlags[iSlot] |= ANIM_NOACTION;

	SetTime(iSlot, 0.0);
	UpdateSayAnimSlot();
}

void SimpleActor::StartAimAnimSlot
	(
	int slot,
	int anim
	)

{
	int iSlot = GetActionSlot(slot);

	m_weightCrossBlend[iSlot] = 0.0;
	m_weightType[iSlot] = 7;

	NewAnim(anim, iSlot, 1.0);

	SetTime(iSlot, 0.0);

	UpdateNormalAnimSlot(iSlot);
}

void SimpleActor::SetBlendedWeight
	(
	int slot
	)

{
	m_bUpdateAnimDoneFlags |= 1 << slot;
	if (m_weightCrossBlend[slot] < 1.0)
	{
		edict->s.frameInfo[slot].weight = (3.0 - m_weightCrossBlend[slot] - m_weightCrossBlend[slot])
			* Square(m_weightCrossBlend[slot])
			* m_weightBase[slot];
	}
	else
	{
		m_weightCrossBlend[slot] = 1.0;
		edict->s.frameInfo[slot].weight = m_weightBase[slot];
	}
}

void SimpleActor::EventSetAnimLength
	(
	Event *ev
	)

{
	int slot;
	float length;
	
	if (ev->NumArgs() != 1)
	{
		ScriptError("bad number of arguments");
	}

	length = ev->GetFloat(1);

	if (length <= 0)
	{
		ScriptError("Positive lengths only allowed");
	}

	if (m_bMotionAnimSet)
	{
		ScriptError("Must set anim before length");
	}

	slot = GetMotionSlot(0);

	if (animFlags[slot] & ANIM_LOOP)
	{
		gi.Anim_Frametime(edict->tiki, edict->s.frameInfo[slot].index);

		animFlags[slot] = (animFlags[slot] | ANIM_NODELTA) & ~ANIM_FINISHED;

		animtimes[slot] = Square(gi.Anim_NumFrames(edict->tiki, edict->s.frameInfo[slot].index) - 1);

		SetOnceType(slot);
	}

	SetSyncTime(0);

	if (length > animtimes[slot])
	{
		ScriptError("cannot lengthen animation which has length %f", animtimes[slot]);
	}
	
	animtimes[slot] = length;
	animFlags[slot] = (animFlags[slot] | ANIM_NODELTA) & ~ANIM_FINISHED;
}

void SimpleActor::UpdateNormalAnimSlot
	(
	int slot
	)

{
	m_weightCrossBlend[slot] += m_fCrossblendTime == 0.0 ? 1.0 : level.frametime / m_fCrossblendTime;

	SetBlendedWeight(slot);
}

void SimpleActor::UpdateCrossBlendAnimSlot
	(
	int slot
	)

{
	m_weightCrossBlend[slot] -= m_fCrossblendTime == 0.0 ? 1.0 : level.frametime / m_fCrossblendTime;

	if (m_weightCrossBlend[slot] > 0.0)
	{
		SetBlendedWeight(slot);
	}
	else
	{
		m_weightType[slot] = 8;
		edict->s.frameInfo[slot].weight = 0.0;
	}
}

void SimpleActor::UpdateCrossBlendDialogAnimSlot
	(
	int slot
	)

{
	m_weightCrossBlend[slot] -= m_iSaySlot < 0 ? level.frametime + level.frametime : level.frametime / 0.1;

	if (m_weightCrossBlend[slot] > 0.0)
	{
		SetBlendedWeight(slot);
	}
	else
	{
		m_weightType[slot] = 8;
		edict->s.frameInfo[slot].weight = 0.0;
	}
}

void SimpleActor::UpdateSayAnimSlot
	(
	int slot
	)

{
	m_weightCrossBlend[slot] += m_iSaySlot < 0 ? level.frametime + level.frametime : level.frametime / 0.1;

	SetBlendedWeight(slot);
}

void SimpleActor::UpdateLastFrameSlot
	(
	int slot
	)

{
	m_weightType[slot] = 0;
	DoExitCommands(slot);

	if (edict->s.frameInfo[slot].index || gi.TIKI_NumAnims(edict->tiki) <= 1)
		edict->s.frameInfo[slot].index = 0;
	else
		edict->s.frameInfo[slot].index = 1;
	
	animFlags[slot] = ANIM_PAUSED | ANIM_NOEXIT | ANIM_NODELTA | ANIM_LOOP;

	edict->s.frameInfo[slot].weight = 0.0;

	animFlags[slot] = (animFlags[slot] | ANIM_NODELTA) & ~ANIM_FINISHED;
}

void SimpleActor::UpdateAnimSlot
	(
	int slot
	)

{
	int weightType = m_weightType[slot];
	switch (weightType)
	{
	case 0:
		break;
	case 1:
	case 2:
	case 7:
		UpdateNormalAnimSlot(slot);
		break;
	case 3:
	case 4:
		UpdateCrossBlendAnimSlot(slot);
		break;
	case 5:
		UpdateCrossBlendDialogAnimSlot(slot);
		break;
	case 6:
		UpdateSayAnimSlot(slot);
		break;
	case 8:
		UpdateLastFrameSlot(slot);
		break;
	default:
		assert(weightType && !"impleActor::UpdateAnimSlot: Bad weight type.");
		break;
	}
}

void SimpleActor::StopAllAnimating
	(
	void
	)

{
	SetSyncTime(0);

	for (int slot = 0; slot < MAX_FRAMEINFOS; slot++)
	{
		UpdateLastFrameSlot(slot);
	}
}

void SimpleActor::ChangeMotionAnim
	(
	void
	)

{
	//int lastMotionSlot;
	//int firstMotionSlot;
	int iSlot;
	int i;

	m_bMotionAnimSet = false;
	m_iMotionSlot = -1;
	m_bLevelMotionAnim = false;

	if (m_ChangeMotionAnimIndex != level.frame_skel_index)
	{
		m_ChangeMotionAnimIndex = level.frame_skel_index;

		MPrintf("Swapping motion channels....\n");
		for (iSlot = GetMotionSlot(0), i = 0; i < 3; i++, iSlot++)
		{
			StartCrossBlendAnimSlot(iSlot);
		}
		m_AnimDialogHigh = !m_AnimDialogHigh;

	}
	
	for (iSlot = GetMotionSlot(0), i = 0; i < 3; i++, iSlot++)
	{
		StopAnimating(iSlot);
	}
}

void SimpleActor::ChangeActionAnim
	(
	void
	)

{
	int iSlot;
	int i;

	m_bAimAnimSet = false;
	m_bActionAnimSet = false;
	m_iActionSlot = -1;
	m_bLevelActionAnim = false;


	if (m_ChangeActionAnimIndex != level.frame_skel_index)
	{
		m_ChangeActionAnimIndex = level.frame_skel_index;

		MPrintf("Swapping action channels....\n");
		for (iSlot = GetActionSlot(0), i = 0; i < 3; i++, iSlot++)
		{
			animFlags[iSlot] |= ANIM_NOACTION;
			StartCrossBlendAnimSlot(iSlot);
		}
		m_AnimDialogHigh ^= 1; // toggle
	}

	for (iSlot = GetActionSlot(0), i = 0; i < 3; i++, iSlot++)
	{
		StopAnimating(iSlot);
	}
}

void SimpleActor::ChangeSayAnim
	(
	void
	)

{
	int iSlot; 

	m_bSayAnimSet = false;
	m_bLevelSayAnim = 0;
	m_iVoiceTime = level.inttime;
	m_iSaySlot = -1;

	if (m_ChangeSayAnimIndex != level.frame_skel_index)
	{
		m_ChangeSayAnimIndex = level.frame_skel_index;
		
		MPrintf("Swapping dialog channel....\n");
		
		iSlot = GetSaySlot();
		StartCrossBlendAnimSlot(iSlot);

		m_AnimDialogHigh ^= 1; // toggle
	}

	iSlot = GetSaySlot();
	StopAnimating(iSlot);
}

void SimpleActor::UpdateAim
	(
	void
	)

{
	float dir;

	int aimForwardSlot;
	int aimUpSlot;
	int aimDownSlot;

	if (m_bAimAnimSet)
	{
		dir = -m_DesiredGunDir[0];

		aimForwardSlot = GetActionSlot(0);
		aimUpSlot = aimForwardSlot + 1;
		aimDownSlot = aimForwardSlot + 2;

		if (dir <= 180)
		{
			if (dir < -180)
			{
				dir += 360;
			}
		}
		else
		{
			dir -= 360;
		}

		float factor;
		if (dir < 0)
		{
			if (dir < m_fAimLimit_down)
				dir = m_fAimLimit_down;

			factor = dir / m_fAimLimit_down;

			m_weightBase[aimForwardSlot] = 0;
			m_weightBase[aimUpSlot] = 1 - factor;
			m_weightBase[aimDownSlot] = factor;
		}
		else
		{
			if (dir > m_fAimLimit_up)
				dir = m_fAimLimit_up;

			factor = dir / m_fAimLimit_up;

			m_weightBase[aimForwardSlot] = factor;
			m_weightBase[aimUpSlot] = 1 - factor;
			m_weightBase[aimDownSlot] = 0;

		}
		SetControllerAngles(TORSO_TAG, vec_origin);
	}
}

void SimpleActor::UpdateAimMotion
	(
	void
	)

{
	int slot = GetMotionSlot(0);

	if (m_fCrouchWeight < 0.0)
	{
		m_weightBase[slot] = 0.0;
		m_weightBase[slot + 1] = m_fCrouchWeight + 1.0;
		m_weightBase[slot + 2] = -m_fCrouchWeight;
	}
	else
	{
		m_weightBase[slot] = m_fCrouchWeight;
		m_weightBase[slot + 1] = 1.0 - m_fCrouchWeight;
		m_weightBase[slot + 2] = 0.0;
	}
}

void SimpleActor::EventAIOn
	(
	Event *ev
	)

{
	m_bDoAI = true;
}

void SimpleActor::EventAIOff
	(
	Event *ev
	)

{
	m_bDoAI = false;
}

void SimpleActor::EventGetWeaponGroup
	(
	Event *ev
	)

{
	Weapon *weapon = GetActiveWeapon(WEAPON_MAIN);
	const_str csWeaponGroup = weapon->GetWeaponGroup();
	if (csWeaponGroup == STRING_NONE)
	{
		csWeaponGroup = STRING_UNARMED;
	}
	ev->AddConstString(csWeaponGroup);
}

void SimpleActor::EventGetWeaponType
	(
	Event *ev
	)

{
	Weapon *weapon;
	const_str csWeaponType;

	if (!m_pTurret)
	{
		weapon = GetActiveWeapon(WEAPON_MAIN);
	}

	if (!weapon)
	{
		csWeaponType = STRING_RIFLE;
	}
	else
	{
		int iWeaponClass = weapon->GetWeaponClass();

		switch (iWeaponClass)
		{
		case WEAPON_CLASS_PISTOL:
			csWeaponType = STRING_PISTOL;
			break;
		case WEAPON_CLASS_RIFLE:
			csWeaponType = STRING_RIFLE;
			break;
		case WEAPON_CLASS_SMG:
			csWeaponType = STRING_SMG;
			break;
		case WEAPON_CLASS_MG:
			csWeaponType = STRING_MG;
			break;
		case WEAPON_CLASS_GRENADE:
			csWeaponType = STRING_GRENADE;
			break;
		case WEAPON_CLASS_HEAVY:
			csWeaponType = STRING_HEAVY;
			break;
		case WEAPON_CLASS_CANNON:
			csWeaponType = STRING_CANNON;
			break;
		case WEAPON_CLASS_ITEM:
			csWeaponType = STRING_ITEM;
			break;
		case WEAPON_CLASS_ITEM2:
			csWeaponType = STRING_ITEM2;
			break;
		case WEAPON_CLASS_ITEM3:
			csWeaponType = STRING_ITEM3;
			break;
		case WEAPON_CLASS_ITEM4:
			csWeaponType = STRING_ITEM4;
			break;
		default:
			csWeaponType = STRING_EMPTY;
			break;
		}

	}

	ev->AddConstString(csWeaponType);
}

void SimpleActor::EventGetPainHandler
	(
	Event *ev
	)

{
	ScriptVariable var;
	
	m_PainHandler.GetScriptValue(&var);

	ev->AddValue(var);
}

void SimpleActor::EventSetPainHandler
	(
	Event *ev
	)

{
	if (ev->IsFromScript())
	{
		ScriptVariable var = ev->GetValue(1);
		
		m_PainHandler.SetScript(var);
	}
	else
	{
		str varString = ev->GetString(1);
		m_PainHandler.SetScript(varString);
	}
}

void SimpleActor::EventGetDeathHandler
	(
	Event *ev
	)

{
	ScriptVariable var;

	m_DeathHandler.GetScriptValue(&var);

	ev->AddValue(var);
}

void SimpleActor::EventSetDeathHandler
	(
	Event *ev
	)

{
	if (ev->IsFromScript())
	{
		ScriptVariable var = ev->GetValue(1);

		m_DeathHandler.SetScript(var);
	}
	else
	{
		str varString = ev->GetString(1);
		m_DeathHandler.SetScript(varString);
	}
}

void SimpleActor::EventGetAttackHandler
	(
	Event *ev
	)

{
	ScriptVariable var;

	m_AttackHandler.GetScriptValue(&var);

	ev->AddValue(var);
}

void SimpleActor::EventSetAttackHandler
	(
	Event *ev
	)

{
	if (ev->IsFromScript())
	{
		ScriptVariable var = ev->GetValue(1);

		m_AttackHandler.SetScript(var);
	}
	else
	{
		str varString = ev->GetString(1);
		m_AttackHandler.SetScript(varString);
	}
}

void SimpleActor::EventGetSniperHandler
	(
	Event *ev
	)

{
	ScriptVariable var;

	m_SniperHandler.GetScriptValue(&var);

	ev->AddValue(var);
}

void SimpleActor::EventSetSniperHandler
	(
	Event *ev
	)

{
	if (ev->IsFromScript())
	{
		ScriptVariable var = ev->GetValue(1);

		m_SniperHandler.SetScript(var);
	}
	else
	{
		str varString = ev->GetString(1);
		m_SniperHandler.SetScript(varString);
	}
}

void SimpleActor::EventSetCrossblendTime
	(
	Event *ev
	)

{
	m_fCrossblendTime = ev->GetFloat(1);
}

void SimpleActor::EventGetCrossblendTime
	(
	Event *ev
	)

{
	ev->AddFloat(m_fCrossblendTime);
}

void SimpleActor::EventSetEmotion
	(
	Event *ev
	)

{
	switch (ev->GetConstString(1))
	{
	case STRING_EMOTION_NONE:
		Anim_Emotion(EMOTION_NONE);
		break;
	case STRING_EMOTION_NEUTRAL:
		Anim_Emotion(EMOTION_NEUTRAL);
		break;
	case STRING_EMOTION_WORRY:
		Anim_Emotion(EMOTION_WORRY);
		break;
	case STRING_EMOTION_PANIC:
		Anim_Emotion(EMOTION_PANIC);
		break;
	case STRING_EMOTION_FEAR:
		Anim_Emotion(EMOTION_FEAR);
		break;
	case STRING_EMOTION_DISGUST:
		Anim_Emotion(EMOTION_DISGUST);
		break;
	case STRING_EMOTION_ANGER:
		Anim_Emotion(EMOTION_ANGER);
		break;
	case STRING_EMOTION_AIMING:
		Anim_Emotion(EMOTION_AIMING);
		break;
	case STRING_EMOTION_DETERMINED:
		Anim_Emotion(EMOTION_DETERMINED);
		break;
	case STRING_EMOTION_DEAD:
		Anim_Emotion(EMOTION_DEAD);
		break;
	case STRING_EMOTION_CURIOUS:
		Anim_Emotion(EMOTION_CURIOUS);
		break;
	default:
		assert(!"Unknown emotion mode specified in script.");
		break;
	}
}

void SimpleActor::EventGetPosition
	(
	Event *ev
	)

{
	ev->AddConstString(m_csCurrentPosition);
}

void SimpleActor::EventSetPosition
	(
	Event *ev
	)

{
	m_csCurrentPosition = ev->GetConstString(1);
}

void SimpleActor::EventGetAnimMode
	(
	Event *ev
	)

{
	// not found in ida
	STUB();
}

void SimpleActor::EventSetAnimMode
	(
	Event *ev
	)

{
	// not found in ida
	STUB();
}

void SimpleActor::EventSetAnimFinal
	(
	Event *ev
	)

{
	ScriptError("animfinal is obsolete");
}

void SimpleActor::DesiredAnimation
	(
	int eAnimMode,
	const_str csAnimString
	)

{
	//fixme: this is an inline function.
	m_eNextAnimMode = eAnimMode;
	m_csNextAnimString = csAnimString;
	m_bNextForceStart = false;
}

void SimpleActor::StartAnimation
	(
	int eAnimMode,
	const_str csAnimString
	)

{
	//fixme: this is an inline function.
	m_eNextAnimMode = eAnimMode;
	m_csNextAnimString = csAnimString;
	m_bNextForceStart = true;
}

void SimpleActor::DesiredAnimation
	(
	int eAnimMode,
	ScriptThreadLabel AnimLabel
	)

{
	//fixme: this is an inline function.
	m_eNextAnimMode = eAnimMode;
	m_csNextAnimString = STRING_NULL;
	m_NextAnimLabel = AnimLabel;
	m_bNextForceStart = false;
}

void SimpleActor::StartAnimation
	(
	int eAnimMode,
	ScriptThreadLabel AnimLabel
	)

{
	//fixme: this is an inline function.
	m_eNextAnimMode = eAnimMode;
	m_csNextAnimString = STRING_NULL;
	m_NextAnimLabel = AnimLabel;
	m_bNextForceStart = true;
}

void SimpleActor::ContinueAnimationAllowNoPath
	(
	void
	)

{
	if (m_eNextAnimMode < 0)
	{
		m_bNextForceStart = false;
		m_csNextAnimString = STRING_NULL;
		m_eNextAnimMode = m_eAnimMode;
		m_NextAnimLabel = m_Anim;
	}
}

void SimpleActor::ContinueAnimation
	(
	void
	)

{
	int eAnimMode = m_eNextAnimMode;
	if (eAnimMode < 0)
	{
		m_bNextForceStart = false;
		m_csNextAnimString = STRING_NULL;
		m_eNextAnimMode = m_eAnimMode;
		m_NextAnimLabel = m_Anim;
		eAnimMode = m_eAnimMode;
	}

	if (eAnimMode <= 3 && !PathExists())
	{
		//assert(!DumpCallTrace("ContinueAnimation() called on a pathed animation, but no path exists"));
		Anim_Stand();
	}
}

void SimpleActor::SetPathGoalEndAnim
	(
	const_str csEndAnim
	)

{
	m_csPathGoalEndAnimScript = csEndAnim;
}

bool SimpleActor::UpdateSelectedAnimation
	(
	void
	)

{
	if (m_csNextAnimString ==  STRING_NULL)
	{
		if (m_bNextForceStart)
		{
			m_Anim = m_NextAnimLabel;
			m_eAnimMode = m_eNextAnimMode;
			if (m_eNextAnimMode != 3)
				SetPathGoalEndAnim(STRING_EMPTY);
			m_bStartPathGoalEndAnim = false;
			m_eNextAnimMode = -1;
			return true;
		}

		if (m_pAnimThread)
		{
			if (m_eAnimMode == m_eNextAnimMode)
			{
				if (m_Anim == m_NextAnimLabel)
				{
					m_bStartPathGoalEndAnim = false;
					m_eNextAnimMode = -1;
					return false;
				}
			}
		}
		m_Anim = m_NextAnimLabel;
		m_eAnimMode = m_eNextAnimMode;
		if (m_eNextAnimMode != 3)
			SetPathGoalEndAnim(STRING_EMPTY);
		m_bStartPathGoalEndAnim = false;
		m_eNextAnimMode = -1;
		return true;
	}

	if (m_bNextForceStart)
	{

		Com_Printf("UpdateSelectedAnimation\n");
		m_Anim.TrySetScript(m_csNextAnimString);
		m_eAnimMode = m_eNextAnimMode;
		if (m_eNextAnimMode != 3)
			SetPathGoalEndAnim(STRING_EMPTY);
		m_bStartPathGoalEndAnim = false;
		m_eNextAnimMode = -1;
		return true;
	}

	if (!m_pAnimThread || m_eAnimMode != m_eNextAnimMode)
	{
		m_Anim.TrySetScript(m_csNextAnimString);
		m_eAnimMode = m_eNextAnimMode;
		if (m_eNextAnimMode != 3)
			SetPathGoalEndAnim(STRING_EMPTY);
		m_bStartPathGoalEndAnim = false;
		m_eNextAnimMode = -1;
		return true;
	}

	if (m_fPathGoalTime <= level.time)
	{
		if (!m_Anim.IsFile(m_csNextAnimString))
		{
			m_Anim.TrySetScript(m_csNextAnimString);
			m_eAnimMode = m_eNextAnimMode;
			if (m_eNextAnimMode != 3)
				SetPathGoalEndAnim(STRING_EMPTY);
			m_bStartPathGoalEndAnim = false;
			m_eNextAnimMode = -1;
			return true;
		}
	}

	m_eNextAnimMode = -1;
	if (m_bStartPathGoalEndAnim)
	{
		m_bStartPathGoalEndAnim = false;

		if (!m_Anim.IsFile(m_csPathGoalEndAnimScript))
		{
			m_Anim.TrySetScript(m_csPathGoalEndAnimScript);
			return true;
		}
	}
	return false;
}

const char *SimpleActor::DumpCallTrace
	(
	const char *pszFmt,
	...
	) const

{
	OVERLOADED_ERROR();
	return "overloaded version should always get called";
}
