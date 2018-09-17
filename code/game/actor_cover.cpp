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

// actor_cover.cpp

#include "actor.h"

void Actor::InitCover
	(
	GlobalFuncs_t *func
	)

{
	func->ThinkState					= &Actor::Think_Cover;
	func->BeginState					= &Actor::Begin_Cover;
	func->EndState						= &Actor::End_Cover;
	func->SuspendState					= &Actor::Suspend_Cover;
	func->FinishedAnimation				= &Actor::FinishedAnimation_Cover;
	func->PassesTransitionConditions	= &Actor::PassesTransitionConditions_Attack;
	func->IsState						= &Actor::IsAttackState;
	func->PathnodeClaimRevoked			= &Actor::PathnodeClaimRevoked_Cover;
}

void Actor::Begin_Cover
	(
	void
	)

{
	DoForceActivate();
	m_csIdleMood = STRING_NERVOUS;
	m_csMood = STRING_ALERT;

	if( m_pCoverNode )
	{
		m_State = 302;
		m_iStateTime = level.inttime;
		return;
	}

	m_State = 300;
	m_iStateTime = level.inttime;

	if( level.inttime < m_iEnemyChangeTime + 200 )
	{
		SetLeashHome( origin );

		if( AttackEntryAnimation() )
		{
			m_bLockThinkState = true;
			m_State = 312;
			m_iStateTime = level.inttime;
		}
	}
}

void Actor::End_Cover
	(
	void
	)

{
	m_pszDebugState = "";

	if( m_pCoverNode )
	{
		m_pCoverNode->Relinquish();
		m_pCoverNode = NULL;
	}

	m_State = -1;
	m_iStateTime = level.inttime;
}

void Actor::Suspend_Cover
	(
	void
	)

{
	if( m_pCoverNode )
	{
		m_pCoverNode->Relinquish();
		m_pCoverNode->MarkTemporarilyBad();
		m_pCoverNode = NULL;
	}

	m_State = 301;
	m_iStateTime = level.inttime;
}

void Actor::State_Cover_NewEnemy
	(
	void
	)

{
	m_bHasDesiredLookAngles = true;

	Cover_FindCover( true );

	if( m_pCoverNode && PathExists() && PathComplete() )
	{
		Anim_RunToCover( 3 );
		m_State = 302;
	}
	else
	{
		Anim_Aim();
		AimAtTargetPos();
		m_State = 307;
	}

	m_iStateTime = level.inttime;
}

void Actor::State_Cover_FindCover
	(
	void
	)

{
	Anim_Aim();
	AimAtTargetPos();
	Cover_FindCover( false );

	if( m_pCoverNode )
	{
		if( PathExists() && !PathComplete() )
		{
			Anim_RunToCover( 3 );
			m_State = 302;
		}
		else
		{
			m_State = 307;
		}

		m_iStateTime = level.inttime;
	}
	else
	{
		if( !m_iPotentialCoverCount )
		{
			SetThink( THINKSTATE_ATTACK, THINK_TURRET );
		}
	}
}

void Actor::State_Cover_TakeCover
	(
	void
	)

{
	if( PathExists() && !PathComplete() )
	{
		FaceEnemyOrMotion( level.inttime - m_iStateTime );
		Anim_RunToCover( 3 );
		m_csPathGoalEndAnimScript = m_bInReload ? STRING_ANIM_RUNTO_COVER_SCR : STRING_ANIM_IDLE_SCR;
	}
	else
	{
		ClearPath();
		m_eAnimMode = 1;
		m_State = 303;
		m_iStateTime = level.inttime;
		State_Cover_FinishReloading();
	}
}

void Actor::State_Cover_FinishReloading
	(
	void
	)

{
	if( m_bInReload )
	{
		ContinueAnimation();
		AimAtTargetPos();
		return;
	}

	Weapon *pWeap = GetWeapon( 0 );

	if( !pWeap || pWeap->GetFireType( FIRE_PRIMARY ) == FT_PROJECTILE || pWeap->GetFireType( FIRE_PRIMARY ) == FT_SPECIAL_PROJECTILE )
	{
		if( m_pCoverNode->nodeflags & AI_DUCK )
		{
			Anim_Crouch();
		}
		else
		{
			Anim_Stand();
		}

		m_sCurrentPathNodeIndex = -1;

		if( m_Team == TEAM_AMERICAN )
		{
			m_iStateTime = level.inttime + ( rand() % 2001 + 2000 );
		}
		else
		{
			m_iStateTime = level.inttime + ( rand() % 11001 + 4000 );
		}

		Anim_Aim();
		AimAtTargetPos();
		return;
	}

	SetDesiredYaw(angles[1]);
	
	SafeSetOrigin( origin );
	m_eNextAnimMode = 1;
	m_bNextForceStart = false;
	m_csNextAnimString = m_csSpecialAttack;
	m_State = 304;
	m_iStateTime = level.inttime;
}

void Actor::State_Cover_Target
	(
	void
	)

{
	DontFaceWall();

	if( m_eDontFaceWallMode <= 8 )
	{
		m_State = 305;
		m_iStateTime = level.inttime;
		State_Cover_FindEnemy();
		return;
	}

	Anim_Aim();
	AimAtTargetPos();

	if( level.inttime > m_iStateTime + 300 && fabs( m_DesiredYaw - angles[ 1 ] ) < 0.001f )
	{
		Vector end = m_vLastEnemyPos + velocity;
		if( DecideToThrowGrenade( end, &m_vGrenadeVel, &m_eGrenadeMode ) )
		{
			SetDesiredYawDir(m_vGrenadeVel);
			m_eNextAnimMode = 1;
			m_bNextForceStart = false;
			m_State = 310;
			m_csNextAnimString = m_eGrenadeMode == AI_GREN_TOSS_ROLL ? STRING_ANIM_GRENADETOSS_SCR : STRING_ANIM_GRENADETHROW_SCR;
			m_iStateTime = level.inttime;
		}
		else if( CanSeeEnemy( 500 ) && CanShootEnemy( 500 ) )
		{
			m_State = 309;
			m_iStateTime = level.inttime;
		}
		else
		{
			m_State = 308;

			if (m_Team == TEAM_AMERICAN)
			{
				m_iStateTime = level.inttime + (rand() % 2001 + 2000);
			}
			else
			{
				m_iStateTime = level.inttime + (rand() % 11001 + 4000);
			}

		}
	}
}

void Actor::State_Cover_Hide
	(
	void
	)

{
	if( m_Enemy )
	{
		SetEnemyPos( origin );
	}

	if( !m_pCoverNode )
	{
		m_State = 301;
		m_iStateTime = level.inttime;
		State_Cover_FindCover();
		return;
	}

	m_csSpecialAttack = m_pCoverNode->GetSpecialAttack( this );
	MPrintf( "special: %d", m_csSpecialAttack );

	if( m_csSpecialAttack )
	{
		SetDesiredYaw(angles[1]);
		SafeSetOrigin( origin );
		m_eNextAnimMode = 1;
		m_bNextForceStart = false;
		m_State = 304;
		m_csNextAnimString = m_csSpecialAttack;
		m_iStateTime = level.inttime;
		return;
	}

	bool bCanShoot = CanShootEnemy( 500 );
	if( CanSeeEnemy( 500 ) || bCanShoot )
	{
		if( CanShootEnemy( 500 ) )
		{
			Vector vDelta = m_Enemy->origin - origin;

			if( VectorLength2DSquared( vDelta ) * 0.75f <= ( orientation[ 0 ][ 0 ] + orientation[ 0 ][ 1 ] ) * ( orientation[ 0 ][ 0 ] + orientation[ 1 ][ 1 ] ) )
			{
				m_State = 307;
			}
			else
			{
				m_State = 309;
			}

			m_iStateTime = level.inttime;
			State_Cover_Shoot();
			return;
		}

		m_pCoverNode->Relinquish();
		m_pCoverNode->MarkTemporarilyBad();
		m_State = 305;
		m_iStateTime = level.inttime;
		State_Cover_FindEnemy();
		return;
	}

	if( m_Team == TEAM_AMERICAN )
	{
		if( level.inttime >= m_iLastFaceDecideTime + 1000 )
		{
			m_iLastFaceDecideTime = level.inttime;
			PathNode *node = PathManager.FindCornerNodeForExactPath( this, m_Enemy, m_fLeash + m_fMaxDistance );

			if( !node )
			{
				m_bHasDesiredLookAngles = false;
				Anim_Stand();
				m_PotentialEnemies.FlagBadEnemy( m_Enemy );
				UpdateEnemy( -1 );
				return;
			}

			Vector vDelta = node->m_PathPos - origin;

			if( vDelta[ 0 ] || vDelta[ 1 ] )
			{
				SetDesiredYawDir(vDelta);
			}
		}

		Anim_Aim();
	}
	else
	{
		if( level.inttime >= m_iLastFaceDecideTime + 1000 )
		{
			m_iLastFaceDecideTime = level.inttime;
			Vector eyepos = EyePosition();
			Vector end = m_vLastEnemyPos + eyepos;
			trace_t trace = G_Trace(
				eyepos,
				vec_zero,
				vec_zero,
				end,
				this,
				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA),
				true,
				"State_Cover" );

			PathNode *node = PathManager.FindCornerNodeForWall( origin, m_vLastEnemyPos, this, 0.0f, trace.plane.normal );

			if( node )
			{
				Vector vDelta = node->m_PathPos - origin;

				if( vDelta[ 0 ] == 0.0f || vDelta[ 1 ] == 0.0f )
				{
					SetDesiredYawDir(vDelta);
				}

				m_eDontFaceWallMode = 6;
			}
			else
			{
				AimAtTargetPos();
				DontFaceWall();
			}
		}

		if( m_eDontFaceWallMode <= 8 )
		{
			Anim_Stand();
		}
		else
		{
			Anim_Aim();
		}
	}

	if( level.inttime <= m_iStateTime )
		return;

	PathNode *pNode = ( PathNode * )G_FindRandomSimpleTarget( m_pCoverNode->target );

	m_pCoverNode->Relinquish();
	m_pCoverNode = NULL;

	if( !pNode )
	{
		Anim_Stand();
		m_State = 305;
		m_iStateTime = level.inttime + ( rand() & 0x7FF );
		return;
	}

	assert( pNode->IsSubclassOfPathNode() );
	if( !pNode->IsSubclassOfPathNode() )
	{
		return;
	}

	if( !pNode->IsClaimedByOther( this ) )
	{
		SetPath( pNode, "Actor::State_Cover_Target", 0 );

		if( PathExists() )
		{
			m_pCoverNode = pNode;
			pNode->Claim( this );
			Anim_RunToDanger( 3 );
			m_State = 306;
			m_iStateTime = level.inttime;
		}
	}
}

void Actor::State_Cover_Shoot
	(
	void
	)

{
	if( m_bNeedReload )
	{
		Cover_FindCover( true );

		if( m_pCoverNode )
		{
			Anim_RunToCover( 3 );
			FaceEnemyOrMotion( 0 );
			m_State = 302;
			m_iStateTime = level.inttime;
			return;
		}
	}
	sizeof(SimpleEntity);
	Anim_Shoot();
	AimAtTargetPos();

	if( level.inttime > m_iStateTime + 10000 )
	{
		gi.Cvar_Set( "g_monitornum", va( "%i", entnum ) );
		assert( !"anim/shoot.scr took over 10 seconds" );
		Com_Error( ERR_DROP, "anim/shoot.scr took over 10 seconds, entnum = %i, targetname = %s", entnum, targetname.c_str() );
	}
}

void Actor::State_Cover_Grenade
	(
	void
	)

{
	GenericGrenadeTossThink();
}

void Actor::State_Cover_SpecialAttack
	(
	void
	)

{
	m_bHasDesiredLookAngles = false;

	assert( m_pCoverNode );
	if( !m_pCoverNode )
	{
		m_State = 305;
		m_iStateTime = level.inttime;
		State_Cover_FindEnemy();
		return;
	}

	if( mbBreakSpecialAttack )
	{
		if( m_pCoverNode )
		{
			m_pCoverNode->Relinquish();
			m_pCoverNode = NULL;
		}

		m_State = 305;
		m_iStateTime = level.inttime;
		State_Cover_FindEnemy();
		return;
	}

	if( level.inttime >= m_iLastEnemyPosChangeTime + level.intframetime || !m_csSpecialAttack )
	{
		m_csSpecialAttack = m_pCoverNode->GetSpecialAttack( this );

		if( !m_csSpecialAttack )
		{
			m_State = 305;
			m_iStateTime = level.inttime;
			State_Cover_FindEnemy();
			return;
		}
	}

	SetDesiredYaw(angles[1]);

	m_eNextAnimMode = 1;
	m_csNextAnimString = m_csSpecialAttack;
	m_bNextForceStart = false;
}

void Actor::State_Cover_FindEnemy
	(
	void
	)

{
	if( m_Team == TEAM_AMERICAN )
	{
		m_bHasDesiredLookAngles = false;

__setpath:
		Anim_Stand();
		SetPathWithLeash( m_vLastEnemyPos, NULL, 0 );
		ShortenPathToAvoidSquadMates();
		if( PathExists() && !PathComplete() && PathAvoidsSquadMates() )
		{
			m_State = 311;
			m_iStateTime = level.inttime;
		}
		else
		{
			m_bTurretNoInitialCover = true;
			SetThink( THINKSTATE_ATTACK, THINK_TURRET );
		}

		return;
	}

	AimAtTargetPos();
	Anim_Aim();
	DontFaceWall();

	if( CanSeeEnemy( 200 ) )
	{
		if( m_eDontFaceWallMode > 8 )
		{
			m_State = 307;
			m_iStateTime = level.inttime;
		}

		goto __setpath;
	}

	if( m_eDontFaceWallMode <= 8 || level.inttime > m_iStateTime + 500 )
	{
		goto __setpath;
	}
}

void Actor::State_Cover_SearchNode
	(
	void
	)

{
	m_bHasDesiredLookAngles = false;

	if( CanSeeEnemy( 200 ) )
	{
		Anim_Aim();
		AimAtTargetPos();
		m_iStateTime = level.inttime;
		return;
	}

	if( PathExists() && !PathComplete() )
	{
		FaceEnemyOrMotion( level.inttime - m_iStateTime );
		Anim_RunToDanger( 3 );
	}
	else
	{
		Anim_Aim();
		AimAtTargetPos();

		if( level.inttime > m_iStateTime + 3000 )
		{
			m_State = 301;
			m_iStateTime = level.inttime;
		}
	}
}

void Actor::State_Cover_HuntEnemy
	(
	void
	)

{
	FaceEnemyOrMotion( level.inttime - m_iStateTime );
	MovePathWithLeash();

	if( PathExists() && !PathComplete() )
	{
		if( CanSeeEnemy( 300 ) )
		{
			m_State = 307;
			m_iStateTime = level.inttime;
		}
	}
	else
	{
		m_State = 305;
		m_iStateTime = level.inttime + ( rand() & 0x7FF );

		if( m_pCoverNode )
		{
			m_pCoverNode->Relinquish();
			m_pCoverNode = NULL;
		}
	}
}

void Actor::State_Cover_FakeEnemy
	(
	void
	)

{
	AimAtTargetPos();
	Anim_Aim();

	if( level.inttime >= m_iStateTime )
	{
		SetThinkState( THINKSTATE_IDLE, THINKLEVEL_NORMAL);
	}
}

void Actor::Think_Cover
	(
	void
	)

{
	if( !RequireThink() )
	{
		return;
	}

	UpdateEyeOrigin();
	NoPoint();
	UpdateEnemy( 500 );

	if( m_State == 312 )
	{
		ContinueAnimation();
	}
	else
	{
		m_bLockThinkState = false;
		if( m_Enemy )
		{
			if( m_State == 313 )
			{
				m_State = 300;
				m_iStateTime = level.inttime;
			}
		}
		else
		{
			if( m_State != 313 )
			{
				m_State = 313;
				m_iStateTime = level.inttime + ( rand() & 0x7FF ) + 1000;
			}
		}

		switch( m_State )
		{
		case 300:
			m_pszDebugState = "NewEnemy";
			State_Cover_NewEnemy();
			break;
		case 301:
			m_pszDebugState = "FindCover";
			State_Cover_FindCover();
			break;
		case 302:
			m_pszDebugState = "TakeCover";
			State_Cover_TakeCover();
			break;
		case 303:
			m_pszDebugState = "FinishReloading";
			State_Cover_FinishReloading();
			break;
		case 304:
			m_pszDebugState = "SpecialAttack";
			State_Cover_SpecialAttack();
			break;
		case 305:
			m_pszDebugState = "FindEnemy";
			State_Cover_FindEnemy();
			break;
		case 306:
			m_pszDebugState = "SearchNode";
			State_Cover_SearchNode();
			break;
		case 307:
			m_pszDebugState = "Target";
			State_Cover_Target();
			break;
		case 308:
			m_pszDebugState = "Hide";
			State_Cover_Hide();
			break;
		case 309:
			m_pszDebugState = "Shoot";
			State_Cover_Shoot();
			break;
		case 310:
			m_pszDebugState = "Grenade";
			State_Cover_Grenade();
			break;
		case 311:
			m_pszDebugState = "HuntEnemy";
			State_Cover_HuntEnemy();
			break;
		case 313:
			m_pszDebugState = "FakeEnemy";
			State_Cover_FakeEnemy();
			break;
		default:
			Com_Printf( "Actor::Think_Cover: invalid think state %i\n", m_State );
			assert( 0 );
			break;
		}

		CheckForTransition( THINKSTATE_GRENADE, THINKLEVEL_NORMAL );
	}

	if( m_State != 305 && m_State != 307 && m_State != 308 && m_State != 309 )
	{
		PostThink( true );
	}
	else
	{
		PostThink( false );
	}
}

void Actor::FinishedAnimation_Cover
	(
	void
	)

{
	if( m_State == 309 )
	{
		if( m_Enemy && !m_Enemy->IsDead() && CanSeeEnemy( 500 ) && CanShootEnemy( 500 ) )
		{
			m_iStateTime = level.inttime;
		}
		else
		{
			m_State = 301;
			m_iStateTime = level.inttime;
		}
	}
	else if( m_State == 310 || m_State == 312 )
	{
		m_State = 301;
		m_iStateTime = level.inttime;
	}
}

void Actor::PathnodeClaimRevoked_Cover
	(
	void
	)

{
	m_State = 301;
	m_iStateTime = level.inttime;
}

bool Actor::Cover_IsValid
	(
	PathNode *node
	)

{
	if( !node->IsClaimedByOther( this ) )
	{
		if( node->nodeflags & AI_CONCEALMENT )
		{
			return true;
		}
		else if( CanSeeFrom( origin + eyeposition, m_Enemy ) )
		{
			if( !( node->nodeflags & AI_DUCK ) )
			{
				return false;
			}
			else if( CanSeeFrom( origin + eyeposition - Vector( 0, 0, 32 ), m_Enemy ) )
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}
	else
	{
		return false;
	}
}
bool Actor::Cover_SetPath
	(
	PathNode *node
	)

{
	SetPathWithLeash( node, NULL, 0 );

	if( !PathExists() )
	{
		return false;
	}

	float origin_ratio;
	Vector enemy_offset;
	PathInfo *current_node;
	Vector enemy_origin;
	Vector vDelta;
	float fMinDistSquared;
	float fPathDist;

	fPathDist = PathDist();
	fMinDistSquared = fPathDist * fPathDist;
	vDelta = node->origin - origin;

	if( fMinDistSquared >= vDelta.lengthSquared() * 4.0f )
	{
		if( fPathDist > 128.0f )
		{
			return false;
		}
	}

	if( !PathComplete() )
	{
		enemy_origin = m_Enemy->origin;
		vDelta = enemy_origin - origin;

		if( VectorLength2DSquared( vDelta ) * 0.64f > 192 * 192 )
		{
			origin_ratio = 192 * 192;
		}

		for( current_node = CurrentPathNode() - 1; current_node >= LastPathNode(); current_node-- )
		{
			vDelta[ 0 ] = origin[ 0 ] - current_node->point[ 0 ];
			vDelta[ 1 ] = origin[ 1 ] - current_node->point[ 1 ];

			if( origin_ratio >= VectorLength2DSquared( vDelta ) )
			{
				return false;
			}

			float fDot = DotProduct2D( vDelta, current_node->dir );
			if( fDot < 0.0f && -current_node->dist <= fDot )
			{
				if( ( vDelta[ 0 ] * current_node->dir[ 0 ] - vDelta[ 1 ] * current_node->dir[ 1 ] ) * ( vDelta[ 0 ] * current_node->dir[ 0 ] - vDelta[ 1 ] * current_node->dir[ 1 ] ) )
				{
					return false;
				}
			}
		}
	}

	return true;
}

void Actor::Cover_FindCover
	(
	bool bCheckAll
	)

{

	if( m_pCoverNode )
	{
		if( Cover_IsValid( m_pCoverNode ) && Cover_SetPath( m_pCoverNode ) )
		{
			return;
		}

		m_pCoverNode->Relinquish();
		m_pCoverNode = NULL;
	}

	if( !m_iPotentialCoverCount )
	{
		m_iPotentialCoverCount = PathManager.FindPotentialCover( this, origin, m_Enemy, m_pPotentialCoverNode, 16 );
	}

	if( m_iPotentialCoverCount )
	{
		PathNode *pNode = NULL;

		while( m_iPotentialCoverCount )
		{
			m_iPotentialCoverCount--;
			pNode = m_pPotentialCoverNode[ m_iPotentialCoverCount ];
			m_pPotentialCoverNode[ m_iPotentialCoverCount ] = NULL;

			if( Cover_IsValid( pNode ) && Cover_SetPath( pNode ) )
			{
				break;
			}

			if( !bCheckAll )
			{
				return;
			}
		}

		m_pCoverNode = pNode;
		m_pCoverNode->Claim( this );
		memset( m_pPotentialCoverNode, 0, sizeof( m_pPotentialCoverNode ) );
		m_iPotentialCoverCount = 0;
	}
}

