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

// earthquake.cpp: Earthquake trigger causes a localized earthquake when triggered.
// The earthquake effect is visible to the user as the shaking of his screen.
// 
#include "earthquake.h"
#include "weapon.h"
#include "sentient.h"
#include "level.h"

/*****************************************************************************/
/*QUAKED func_viewjitter (0 0.25 0.5) (-8 -8 -8) (8 8 8)
Causes a radius view jitter
"radius" sets the max effective range if the view jitter. Default is 128
"edgeeffect" set the fraction of the total jitter to apply at the max range. Default is 0.2
"jitteramount" sets the amount to jitter the view. Set in angles as "pitch yaw roll". Default is "2 2 3"
"duration" set how long the view jitter should last. Default is 0 (instantanious)
"timedecay" sets how the jitter angles will fall off over the jitter's duraction in degrees per second. Default is "2 2 3"
"donedeath" makes the view jitter remove itself after being fired once. Any non-zero value will work.
******************************************************************************/

Event EV_ViewJitter_Think
	(
	"_viewjitter_think",
	EV_DEFAULT,
	0,
	0,
	"think function for the viewjitter."
	);

Event EV_ViewJitter_Radius
	(
	"radius",
	EV_DEFAULT,
	"f",
	"radius",
	"Sets the max radius of the view jitter. 0 affects all"
	);

Event EV_ViewJitter_EdgeEffect
	(
	"edgeeffect",
	EV_DEFAULT,
	"f",
	"fraction",
	"Sets the fraction of the jitter to apply at the max radius"
	);

Event EV_ViewJitter_Amount
	(
	"jitteramount",
	EV_DEFAULT,
	"v",
	"jitterangles",
	"Sets the jitter angles to apply to the player"
	);

Event EV_ViewJitter_Duration
	(
	"duration",
	EV_DEFAULT,
	"f",
	"time",
	"Sets the length of time it should last. 0 will be instantanious"
	);

Event EV_ViewJitter_TimeDecay
	(
	"timedecay",
	-1,
	"v",
	"decayrate",
	"Sets jitter decay per second"
	);

Event EV_ViewJitter_DoneDeath
	(
	"donedeath",
	EV_DEFAULT,
	0,
	0,
	"Makes the view jitter only happen once"
	);

CLASS_DECLARATION( Trigger, ViewJitter, "func_viewjitter" )
{
	{ &EV_Touch,						NULL },
	{ &EV_Trigger_Effect, 				&ViewJitter::EventActivateJitter },
	{ &EV_ViewJitter_Radius, 			&ViewJitter::EventSetRadius },
	{ &EV_ViewJitter_EdgeEffect, 		&ViewJitter::EventSetEdgeEffect },
	{ &EV_ViewJitter_Amount, 			&ViewJitter::EventSetAmount },
	{ &EV_ViewJitter_Duration, 			&ViewJitter::EventSetDuration },
	{ &EV_ViewJitter_TimeDecay, 		&ViewJitter::EventSetTimeDecay },
	{ &EV_ViewJitter_DoneDeath, 		&ViewJitter::EventSetDoneDeath },
	{ &EV_ViewJitter_Think, 			&ViewJitter::EventJitterThink },
	{ NULL, NULL }
};

ViewJitter::ViewJitter()
{
	if( LoadingSavegame )
	{
		return;
	}

	edict->r.svFlags |= SVF_NOCLIENT;

	m_fRadius = 16384.0f;
	m_fEdgeEffect = 0.2f;

	m_vJitterStrength = Vector( 2, 2, 3 );
	m_vTimeDecay = Vector( 2, 2, 3 );

	m_fDuration = 0;

	m_bDoneDeath = qfalse;
}

ViewJitter::ViewJitter( Vector vOrigin, float fRadius, float fEdgeEffect, Vector vStrength, float fDuration, Vector vTimeDecay, float fStartDecay )
{
	if( LoadingSavegame )
	{
		return;
	}

	setOrigin( vOrigin );
	m_fEdgeEffect = fEdgeEffect;

	// square the raidus
	m_fRadius = fRadius * fRadius;

	m_vJitterStrength = vStrength;
	m_vTimeDecay = vTimeDecay;

	m_fDuration = fDuration;

	m_bDoneDeath = false;
	m_fTimeRunning = 0;

	PostEvent( EV_ViewJitter_Think, fStartDecay );
}

void ViewJitter::EventActivateJitter
	(
	Event *ev
	)
{
	m_fTimeRunning = 0;
	PostEvent( EV_ViewJitter_Think, 0 );
}

void ViewJitter::EventJitterThink
	(
	Event *ev
	)
{
	int i;
	int iNumSents;
	float fDist;
	float fRadiusDecay;
	Vector vCurrJitter;
	Vector vApplyJitter;
	Vector vDelta;
	Sentient *pSent;

	m_fTimeRunning += level.frametime;

	vDelta = m_vJitterStrength - m_vTimeDecay * m_fTimeRunning;

	iNumSents = SentientList.NumObjects();

	for( i = 1; i <= iNumSents; i++ )
	{
		pSent = SentientList.ObjectAt( i );

		if( pSent->deadflag ) {
			continue;
		}

		fRadiusDecay = ( pSent->origin - origin ).lengthSquared();
		if( fRadiusDecay <= m_fRadius )
		{
			Vector vVariation;

			if( m_fEdgeEffect == 1.0f )
			{
				vApplyJitter = vDelta;
			}
			else
			{
				fDist = sqrt( fRadiusDecay );
				vApplyJitter = Vector( ( 1.0f - fDist * ( m_fEdgeEffect / m_fRadius ) ) * vDelta[ 0 ], 0, 0 );
			}

			if( pSent->m_vViewVariation[ 0 ] <= vApplyJitter[ 0 ] )
				vVariation[ 0 ] = vApplyJitter[ 0 ];
			else
				vVariation[ 0 ] = pSent->m_vViewVariation[ 0 ];

			if( pSent->m_vViewVariation[ 1 ] <= vApplyJitter[ 1 ] )
				vVariation[ 1 ] = vApplyJitter[ 1 ];
			else
				vVariation[ 1 ] = pSent->m_vViewVariation[ 1 ];

			if( pSent->m_vViewVariation[ 2 ] <= vApplyJitter[ 2 ] )
				vVariation[ 2 ] = vApplyJitter[ 2 ];
			else
				vVariation[ 2 ] = pSent->m_vViewVariation[ 2 ];

			pSent->m_vViewVariation = vVariation;
		}
	}

	if( m_fDuration > 0.0f )
	{
		if( m_fTimeRunning <= m_fDuration || !m_bDoneDeath )
		{
			PostEvent( EV_ViewJitter_Think, level.frametime );
		}
		else
		{
			ProcessEvent( EV_Remove );
		}
	}
	else if( m_bDoneDeath )
	{
		ProcessEvent( EV_Remove );
	}
}

void ViewJitter::EventSetRadius
	(
	Event *ev
	)
{
	float rad = ev->GetFloat( 1 );

	m_fRadius = rad * rad;
	if( m_fRadius < 1.0f )
	{
		m_fRadius = 16384.0f;
	}
}

void ViewJitter::EventSetEdgeEffect
	(
	Event *ev
	)
{
	m_fEdgeEffect = ev->GetFloat( 1 );
}

void ViewJitter::EventSetAmount
	(
	Event *ev
	)
{
	m_vJitterStrength = ev->GetVector( 1 );
}

void ViewJitter::EventSetDuration
	(
	Event *ev
	)
{
	m_fDuration = ev->GetFloat( 1 );
}

void ViewJitter::EventSetTimeDecay
	(
	Event *ev
	)
{
	m_vTimeDecay = ev->GetVector( 1 );
}

void ViewJitter::EventSetDoneDeath
	(
	Event *ev
	)
{
	m_bDoneDeath = qtrue;
}
