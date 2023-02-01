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

// scripttimer.cpp: Scripted Timer & Fader
//

#include "scripttimer.h"
#include "level.h"

#if defined(GAME_DLL)
#include "archive.h"
#endif

Event EV_ScriptTimer_Think
(
	"scripttimer_think",
	EV_CODEONLY,
	NULL,
	NULL,
	"Internal event",
	EV_NORMAL
);

Container< ScriptTimer * > m_scriptTimers;

/*void ScriptTimer::HandleTimer( float frametime )
{
	ScriptTimer *timer;

	for( int i = 0; i < m_scriptTimers.NumObjects(); i++ )
	{
		timer = m_scriptTimers[ i ];

		if( timer->isEnabled() && timer->GetTime() > 0.0f ) {
			timer->Think( frametime );
		}
	}
}*/

ScriptTimer::ScriptTimer( timertype_e type )
{
	Reset();
	bEnabled = false;

	setType( type );

	m_scriptTimers.AddObject( this );
}

ScriptTimer::~ScriptTimer()
{
	Disable();

	if( m_scriptTimers.IndexOfObject( this ) )
	{
		m_scriptTimers.RemoveObject( this );
	}
}

void ScriptTimer::Archive( Archiver &arc )
{
	Listener::Archive( arc );

#ifdef CGAME_DLL
	arc.ArchiveFloat( &targetTime );
	arc.ArchiveFloat( &currentTime );
	arc.ArchiveFloat( &realTime );

	arc.ArchiveFloat( &glideRatio );

	arc.ArchiveInteger( ( int * )&timerType );

	arc.ArchiveBool( &bEnabled );

	if( arc.Loading() && bEnabled ) {
		Enable();
	}
#endif
}

void ScriptTimer::GlideRefresh()
{
	if( timerType != TIMER_GLIDE ) {
		return;
	}

	if( targetTime <= 0.0f ){
		return;
	}

	float r = realTime / ( targetTime * 0.5f );

	if( r > 1.0f )
	{
		glideRatio = 2.0f - 2.0f * ( r - 1.0f );

		if( glideRatio < 0.0f ) {
			glideRatio = 0.0f;
		}
	}
	else
	{
		glideRatio = 2.0f * r;

		if( glideRatio > 2.0f ) {
			glideRatio = 2.0f;
		}
	}
}

void ScriptTimer::Think( Event *ev )
{
	if( !bEnabled ) {
		return;
	}

#ifdef CGAME_DLL
	float frametime = ( float )cg->frametime;
#else
	float frametime = level.frametime;
#endif

	GlideRefresh();

	if( currentTime < targetTime )
	{
		realTime += frametime;

		if( timerType == TIMER_GLIDE ) {
			currentTime += frametime * glideRatio;
		} else {
			currentTime += frametime;
		}
	}
	else
	{
		currentTime = targetTime;
		realTime = targetTime;

		bEnabled = false;
		CancelEventsOfType( &EV_ScriptTimer_Think );

		return;
	}

	CancelEventsOfType( &EV_ScriptTimer_Think );
	PostEvent( EV_ScriptTimer_Think, level.frametime );
}

void ScriptTimer::Disable()
{
	bEnabled = false;

	CancelEventsOfType( &EV_ScriptTimer_Think );
}

void ScriptTimer::Enable()
{
	bEnabled = true;

	CancelEventsOfType( &EV_ScriptTimer_Think );

	Event *ev = new Event( &EV_ScriptTimer_Think );
	ProcessEvent( ev );
}

qboolean ScriptTimer::Done()
{
	return ( currentTime >= targetTime );
}

float ScriptTimer::GetCurrentTime()
{
	return realTime;
}

float ScriptTimer::GetRatio()
{
	float ratio;

	if( targetTime <= 0.0f ) {
		return 1.0f;
	}

	ratio = ( currentTime / targetTime );

	// ratio must not go below 0.0 and above 1.0

	return ( ratio < 0.0f ? 0.0f : ratio > 1.0f ? 1.0f : ratio );
}

float ScriptTimer::GetTime()
{
	return targetTime;
}

float ScriptTimer::LerpValue( float start, float end )
{
	return start + ( end - start ) * GetRatio();
}

Vector ScriptTimer::LerpValue( Vector start, Vector end )
{
	return start + ( end - start ) * GetRatio();
}

void ScriptTimer::Reset()
{
	currentTime = 0.f;
	realTime = 0.f;

	glideRatio = 0.f;
}

void ScriptTimer::SetCurrentTime( float time )
{
	realTime = time * 1000.0f;

	if( timerType == TIMER_GLIDE )
	{
		GlideRefresh();
		currentTime = time * 1000.0f * glideRatio;
	}
	else
	{
		currentTime = time * 1000.0f;
	}
}

void ScriptTimer::SetPhase( float phase )
{
	float t = targetTime * ( phase / 2.0f ) / 1000.0f;

	SetCurrentTime( t );
}

void ScriptTimer::SetTime( float time )
{
	Reset();

	targetTime = time * 1000.0f;
}

bool ScriptTimer::isEnabled()
{
	return bEnabled;
}

void ScriptTimer::setType( timertype_e type )
{
	timerType = type;
}

CLASS_DECLARATION( Listener, ScriptTimer, NULL )
{
	{ &EV_ScriptTimer_Think,	&ScriptTimer::Think },
	{ NULL, NULL }
};
