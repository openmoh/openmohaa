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

// barrels.cpp : Barrels

#include "barrels.h"
#include "weaputils.h"

/*****************************************************************************
/*QUAKED func_barrel (0 0.25 0.5) ? INDESTRUCTABLE
Brush model barrel object
Bashing only makes thunk or gong sounds (depending on wether it's full or not)
Bullets and explosions effects are dependant apon the type of barrel.
"barreltype" sets the type of barrel that it is. Valid settings are:
- "water" makes the barrel be filled with water.
Always non-volitile. Only destroyed by explosion.
- "oil" makes the barrel be filled with oil.
Leakes oil when shot (not destroyed), shower of flames when exploded.
- "gas" makes the barrel be filled with gas.
A few bullet hits will make it explode.
Explosions will also make it explode.
- "empty" makes the barrel completely empty.
Does nothing special at all. It's just a poor empty barrel. aww :(
******************************************************************************/

Event EV_Barrel_Think
	(
	"_barrel_think",
	EV_DEFAULT,
	NULL,
	NULL,
	"think function for a barrel."
	);

Event EV_Barrel_Setup
	(
	"_barrel_setup",
	EV_DEFAULT,
	NULL,
	NULL,
	"Does the post spawn setup of the barrel"
	);

Event EV_Barrel_SetType
	(
	"barreltype",
	EV_DEFAULT,
	"s",
	"type",
	"Sets the barrel's type"
	);

CLASS_DECLARATION( Entity, BarrelObject, "func_barrel" )
{
	{ &EV_Barrel_Setup,					&BarrelObject::BarrelSetup },
	{ &EV_Barrel_Think,					&BarrelObject::BarrelThink },
	{ &EV_Barrel_SetType,				&BarrelObject::BarrelSetType },
	{ &EV_Damage,						&BarrelObject::BarrelDamaged },
	{ &EV_Killed,						&BarrelObject::BarrelKilled },
	{ NULL, NULL }
};

BarrelObject::BarrelObject()
{
	if( LoadingSavegame )
	{
		return;
	}

	edict->s.eType = ET_GENERAL;

	m_iBarrelType = 0;
	m_fFluidAmount = 0;
	m_fHeightFluid = 0;

	for( int i = 0; i < MAX_BARREL_LEAKS; i++ )
	{
		m_bLeaksActive[ i ] = 0;
	}

	m_fDamageSoundTime = 0;
	mass = 500;
	max_health = 75;
	health = 75;
	deadflag = DEAD_NO;
	takedamage = DAMAGE_YES;
	m_vJitterAngles = vec_zero;

	PostEvent( EV_Barrel_Setup, EV_POSTSPAWN );
}

int BarrelObject::PickBarrelLeak
	(
	void
	)

{
	int iHighest;
	float fHighestHeight;

	for( int i = 0; i < MAX_BARREL_LEAKS; i++ )
	{
		if( !m_bLeaksActive[ i ] ) {
			return i;
		}
	}

	fHighestHeight = m_vLeaks[ 0 ][ 2 ];
	iHighest = 0;

	for( int i = 1; i < MAX_BARREL_LEAKS; i++ )
	{
		if( m_vLeaks[ 0 ][ 2 ] > fHighestHeight )
		{
			fHighestHeight = m_vLeaks[ 0 ][ 2 ];
			iHighest = i;
		}
	}

	return iHighest;
}

void BarrelObject::BarrelSetup
	(
	Event *ev
	)

{
	setMoveType( MOVETYPE_PUSH );
	setSolidType( SOLID_BSP );

	m_vStartAngles = angles;
	m_fJitterScale = 64.0f / size[ 2 ];

	m_fFluidAmount = size[ 0 ] * size[ 1 ] * size[ 2 ] / 150.0f;
	m_fHeightFluid = m_fFluidAmount / size[ 2 ];

	// Position the barrel correctly
	CheckGround();
}

void BarrelObject::BarrelSetType
	(
	Event *ev
	)

{
	str sType = ev->GetString( 1 );

	if( !sType.icmp( "oil" ) )
	{
		health = 75;
		m_iBarrelType = BARREL_OIL;

		CacheResource( "models/fx/barrel_empty_destroyed.tik" );
		CacheResource( "models/fx/barrel_oil_destroyed.tik" );
	}
	else if( !sType.icmp( "water" ) )
	{
		health = 75;
		m_iBarrelType = BARREL_WATER;

		CacheResource( "models/fx/barrel_empty_destroyed.tik" );
		CacheResource( "models/fx/barrel_water_destroyed.tik" );
	}
	else if( !sType.icmp( "gas" ) )
	{
		spawnflags &= ~BARREL_INDESTRUCTABLE;
		health = 75;
		m_iBarrelType = BARREL_GAS;

		CacheResource( "models/fx/barrel_gas_destroyed.tik" );
	}
	else
	{
		health = 75;
		m_iBarrelType = BARREL_EMPTY;

		CacheResource( "models/fx/barrel_empty_destroyed.tik" );
	}

	max_health = health;
}

void BarrelObject::BarrelThink
	(
	Event *ev
	)

{
	int i;
	int iBiggestLeak;
	float fFluidTop;

	iBiggestLeak = 0;

	fFluidTop = m_fFluidAmount / m_fHeightFluid + ( mins[ 2 ] + origin[ 2 ] );
	for( i = 0; i < MAX_BARREL_LEAKS; i++ )
	{
		if( !m_bLeaksActive[ i ] ) {
			continue;
		}

		// Send infos to clients
		if( m_vLeaks[ i ][ 2 ] <= fFluidTop )
		{
			gi.SetBroadcastVisible( m_vLeaks[ i ], m_vLeaks[ i ] );

			if( m_vLeaks[ i ][ 2 ] <= fFluidTop - 1.3f )
			{
				if( m_vLeaks[ i ][ 2 ] <= fFluidTop - 3.0f )
				{
					// big leak

					if( m_iBarrelType == BARREL_OIL )
					{
						gi.MSG_StartCGM( 15 );
					}
					else
					{
						gi.MSG_StartCGM( 19 );
					}

					m_fFluidAmount -= 1.0f;
					iBiggestLeak |= 4;
				}
				else
				{
					// medium leak

					if( m_iBarrelType == BARREL_OIL )
					{
						gi.MSG_StartCGM( 16 );
					}
					else
					{
						gi.MSG_StartCGM( 20 );
					}

					m_fFluidAmount -= 0.75f;
					iBiggestLeak |= 2;
				}
			}
			else
			{
				// small leak

				if( m_iBarrelType == BARREL_OIL )
				{
					gi.MSG_StartCGM( 17 );
				}
				else
				{
					gi.MSG_StartCGM( 21 );
				}

				m_fFluidAmount -= 0.5f;
				iBiggestLeak |= 1;
			}

			gi.MSG_WriteCoord( m_vLeaks[ i ][ 0 ] );
			gi.MSG_WriteCoord( m_vLeaks[ i ][ 1 ] );
			gi.MSG_WriteCoord( m_vLeaks[ i ][ 2 ] );
			gi.MSG_WriteDir( m_vLeakNorms[ i ] );
			gi.MSG_EndCGM();
		}
		else
		{
			gi.SetBroadcastVisible( m_vLeaks[ i ], m_vLeaks[ i ] );

			if( m_iBarrelType == BARREL_OIL )
			{
				gi.MSG_StartCGM( 17 );
			}
			else
			{
				gi.MSG_StartCGM( 21 );
			}

			gi.MSG_WriteCoord( m_vLeaks[ i ][ 0 ] );
			gi.MSG_WriteCoord( m_vLeaks[ i ][ 1 ] );
			gi.MSG_WriteCoord( m_vLeaks[ i ][ 2 ] );
			gi.MSG_WriteDir( m_vLeakNorms[ i ] );
			gi.MSG_EndCGM();

			m_bLeaksActive[ i ] = qfalse;
		}
	}

	if( m_vJitterAngles[ 0 ] == 0.0f && m_vJitterAngles[ 2 ] == 0.0f )
	{
		if( !VectorCompare( angles, m_vStartAngles ) )
		{
			setAngles( m_vStartAngles );
		}
	}

	setAngles( Vector( m_vStartAngles[ 0 ] + m_vJitterAngles[ 0 ], m_vStartAngles[ 1 ], m_vStartAngles[ 2 ] + m_vJitterAngles[ 2 ] ) );

	if( m_vJitterAngles[ 0 ] > 0.0f )
	{
		m_vJitterAngles[ 0 ] -= 1.f / 3.f * m_fJitterScale;

		if( m_vJitterAngles[ 0 ] > 0.0f )
		{
			m_vJitterAngles[ 0 ] = 0.0f;
		}
	}
	else if( m_vJitterAngles[ 0 ] < 0.0f )
	{
		m_vJitterAngles[ 0 ] += 1.f / 3.f * m_fJitterScale;

		if( m_vJitterAngles[ 0 ] < 0.0f )
		{
			m_vJitterAngles[ 0 ] = 0.0f;
		}
	}

	m_vJitterAngles[ 0 ] = -m_vJitterAngles[ 0 ];

	if( m_vJitterAngles[ 2 ] > 0.0f )
	{
		m_vJitterAngles[ 2 ] -= 1.f / 3.f * m_fJitterScale;

		if( m_vJitterAngles[ 2 ] > 0.0f )
		{
			m_vJitterAngles[ 2 ] = 0.0f;
		}
	}
	else if( m_vJitterAngles[ 2 ] < 0.0f )
	{
		m_vJitterAngles[ 2 ] += 1.f / 3.f * m_fJitterScale;

		if( m_vJitterAngles[ 2 ] < 0.0f )
		{
			m_vJitterAngles[ 2 ] = 0.0f;
		}
	}

	m_vJitterAngles[ 2 ] = -m_vJitterAngles[ 2 ];

	// Check for at least one active leak to play a sound
	for( i = 0; i < MAX_BARREL_LEAKS; i++ )
	{
		if( m_bLeaksActive[ i ] ) {
			break;
		}
	}

	// Play a leak sound
	if( i != MAX_BARREL_LEAKS && iBiggestLeak )
	{
		if( !( iBiggestLeak & 4 ) )
		{
			if( iBiggestLeak & 2 )
			{
				// medium leak
				LoopSound( "liquid_leak", 0.60f, -1.0f, -1.0f, 0.90f );
			}
			else
			{
				// small leak
				LoopSound( "liquid_leak", 0.30f, -1.0f, -1.0f, 0.80f );
			}
		}
		else
		{
			// big leak
			LoopSound( "liquid_leak", 1.0f, -1.0f, -1.0f, 1.0f );
		}
	}

	if( i == MAX_BARREL_LEAKS ) {
		StopLoopSound();
	}

	if( m_vJitterAngles[ 0 ] || m_vJitterAngles[ 2 ] || i < MAX_BARREL_LEAKS )
	{
		m_fLastEffectTime += 0.075f;

		if( level.time >= m_fLastEffectTime )
		{
			m_fLastEffectTime = level.time + 0.075f;
		}

		PostEvent( EV_Barrel_Think, m_fLastEffectTime - level.time );
	}
}

void BarrelObject::BarrelDamaged
	(
	Event *ev
	)

{
	Vector vDir;
	Vector vForward;
	Vector vRight;
	int iDamage;
	int iMeansOfDeath;
	Vector vHitPos;
	Vector vHitDirection;
	Vector vHitNormal;

	if( !takedamage )
	{
		return;
	}

	iDamage = ev->GetInteger( 2 );
	iMeansOfDeath = ev->GetInteger( 9 );
	vHitPos = ev->GetVector( 4 );
	vHitDirection = ev->GetVector( 5 );
	vHitNormal = ev->GetVector( 6 );

	vDir = ( vHitDirection - vHitNormal ) * 0.5f;
	AngleVectors( angles, vForward, vRight, NULL );

	m_vJitterAngles[ 0 ] += DotProduct( vDir, vForward ) * m_fJitterScale * 0.0275f * ( float )iDamage;
	m_vJitterAngles[ 2 ] += DotProduct( vDir, vRight ) * m_fJitterScale * 0.0275f * ( float )iDamage;

	if( m_vJitterAngles[ 0 ] > m_fJitterScale * 1.5f )
	{
		m_vJitterAngles[ 0 ] = m_fJitterScale * 1.5f;
	}
	else if( m_vJitterAngles[ 0 ]  < -( m_fJitterScale * 1.5f ) )
	{
		m_vJitterAngles[ 0 ] = -( m_fJitterScale * 1.5f );
	}

	if( m_vJitterAngles[ 2 ] > m_fJitterScale * 1.5f )
	{
		m_vJitterAngles[ 2 ] = m_fJitterScale * 1.5f;
	}
	else if( m_vJitterAngles[ 2 ] < -( m_fJitterScale * 1.5f ) )
	{
		m_vJitterAngles[ 2 ] = -( m_fJitterScale * 1.5f );
	}

	if( !( spawnflags & BARREL_INDESTRUCTABLE ) )
	{
		if( ( iMeansOfDeath == MOD_VEHICLE || iMeansOfDeath == MOD_ROCKET ||
			iMeansOfDeath == MOD_GRENADE || iMeansOfDeath == MOD_EXPLODEWALL ||
			iMeansOfDeath == MOD_EXPLOSION || m_iBarrelType == BARREL_GAS ) &&
			iMeansOfDeath != MOD_BASH
			)
		{
			if( iDamage >= health )
			{
				PostEvent( EV_Killed, 0.01f );
				takedamage = DAMAGE_NO;
				return;
			}

			if( m_iBarrelType == BARREL_GAS )
			{
				iDamage /= 2;

				health -= iDamage;
				if( health < 1.0f )
					health = 1.0f;
			}
		}
	}

	if( !EventPending( EV_Barrel_Think ) )
	{
		m_fLastEffectTime = level.time - 0.075f;
		ProcessEvent( EV_Barrel_Think );
	}

	if( m_iBarrelType <= BARREL_WATER )
	{
		int index = PickBarrelLeak();

		if( vHitPos[ 2 ] <= m_fFluidAmount / m_fHeightFluid + origin[ 2 ] + mins[ 2 ] )
		{
			m_bLeaksActive[ index ] = qtrue;
			m_vLeaks[ index ] = vHitPos;
			m_vLeakNorms[ index ] = vHitNormal;

			gi.SetBroadcastVisible( vHitPos, vHitPos );

			if( m_iBarrelType == BARREL_OIL )
				gi.MSG_StartCGM( 18 );
			else
				gi.MSG_StartCGM( 22 );
		}
		else
		{
			gi.SetBroadcastVisible( vHitPos, vHitPos );

			if( m_iBarrelType == BARREL_OIL )
				gi.MSG_StartCGM( 17 );
			else
				gi.MSG_StartCGM( 21 );
		}

		gi.MSG_WriteCoord( vHitPos[ 0 ] );
		gi.MSG_WriteCoord( vHitPos[ 1 ] );
		gi.MSG_WriteCoord( vHitPos[ 2 ] );
		gi.MSG_WriteDir( vHitNormal );
		gi.MSG_EndCGM();
	}
}

void BarrelObject::BarrelKilled
	(
	Event *ev
	)

{
	float fFluidTop;
	Vector vPos;
	str sModel;

	setSolidType( SOLID_NOT );
	PostEvent( EV_Remove, 0.05f );

	vPos[ 2 ] = ( mins[ 2 ] + origin[ 2 ] );
	fFluidTop = m_fFluidAmount / m_fHeightFluid + vPos[ 2 ];

	if( m_iBarrelType == BARREL_GAS )
	{
		RadiusDamage(
			centroid,
			this,
			this,
			200,
			this,
			MOD_EXPLOSION,
			350,
			24 );

		sModel = "models/fx/barrel_gas_destroyed.tik";
	}
	else if( m_iBarrelType == BARREL_WATER )
	{
		if( vPos[ 2 ] + 0.25f * maxs[ 2 ] <= fFluidTop )
		{
			sModel = "models/fx/barrel_water_destroyed.tik";
		}
		else
		{
			sModel = "models/fx/barrel_empty_destroyed.tik";
		}
	}
	else if( m_iBarrelType == BARREL_OIL )
	{
		if( vPos[ 2 ] + 0.25f * maxs[ 2 ] > fFluidTop )
		{
			sModel = "models/fx/barrel_empty_destroyed.tik";
		}
		else
		{
			RadiusDamage(
				centroid,
				this,
				this,
				200,
				this,
				MOD_EXPLOSION,
				350,
				24 );

			sModel = "models/fx/barrel_oil_destroyed.tik";
		}
	}
	else
	{
		sModel = "models/fx/barrel_empty_destroyed.tik";
	}

	Animate *exp = new Animate;

	// Spawn an explosion effect
	exp->edict->s.renderfx |= RF_DONTDRAW;
	exp->setModel( sModel );

	vPos = origin + ( maxs + mins ) * 0.5f;

	exp->setAngles( Vector( -90, 0, 0 ) );
	exp->setOrigin( vPos );
	exp->NewAnim( "idle" );

	exp->PostEvent( EV_Remove, 0.1f );
	exp->Unregister( STRING_DEATH );
}
